/*
 * @brief USB audio class module routines
 *
 * @note
 * Copyright  2013, NXP
 * All rights reserved.
 *
 * @par
 * Software that is described herein is for illustrative purposes only
 * which provides customers with programming information regarding the
 * LPC products.  This software is supplied "AS IS" without any warranties of
 * any kind, and NXP Semiconductors and its licensor disclaim any and
 * all warranties, express or implied, including all implied warranties of
 * merchantability, fitness for a particular purpose and non-infringement of
 * intellectual property rights.  NXP Semiconductors assumes no responsibility
 * or liability for the use of the software, conveys no license or rights under any
 * patent, copyright, mask work right, or any other intellectual property rights in
 * or to any products. NXP Semiconductors reserves the right to make changes
 * in the software without notification. NXP Semiconductors also makes no
 * representation or warranty that such application will be suitable for the
 * specified use without further testing or modification.
 *
 * @par
 * Permission to use, copy, modify, and distribute this software and its
 * documentation is hereby granted, under NXP Semiconductors' and its
 * licensor's relevant copyrights in the software, without fee, provided that it
 * is used in conjunction with NXP Semiconductors microcontrollers.  This
 * copyright, permission, and disclaimer notice must appear in all copies of
 * this code.
 */
#include <string.h>
#include "app_usbd_cfg.h"
#include "board.h"
#include "usbd_adc.h"
#include "audio_usbd.h"
#include "Power_Tasks.h"

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/

/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/
USB_ADC_CTRL_T g_AdcCtrl;
/* Interface to manage individual codecs */
typedef struct _volumeInterface_t
{
    uint16_t volumeCur;
    uint16_t volumeMin;
    uint16_t volumeMax;
    uint16_t volumeRes;
    uint16_t volumeDef;
} volumeInterface_t;

const volumeInterface_t g_volume = {
    0x0000, // volumeCur; -57 dB
    0x8000, // volumeMin; -57 dB
    0x7fff, // volumeMax; +6 dB
    0x0001, // volumeRes; 1 dB
    0x0039, // volumeDef; 0 dB
};
volatile uint32_t audioPosition = 0U;
extern unsigned short g_data_buffer[];

/*****************************************************************************
 * Private functions
 ****************************************************************************/

/* Audio Device Class Interface Get Request Callback
    Called automatically on ADC Interface Get Request */
static ErrorCode_t ADC_IF_GetRequest(USB_ADC_CTRL_T *pAdcCtrl, USB_CORE_CTRL_T *pCtrl)
{
    ErrorCode_t ret = ERR_USBD_INVALID_REQ;
    uint16_t *pBuf;

    /* Feature Unit: volume control terminal ID = 2 */
    if (pCtrl->SetupPacket.wIndex.WB.H == 0x02)
    {
        /* check if it is for master channel = 0 */
        if (pCtrl->SetupPacket.wValue.WB.L == 0)
        {
            /* Master Channel */
            switch (pCtrl->SetupPacket.wValue.WB.H)
            {
                case AUDIO_MUTE_CONTROL:
                    if (pCtrl->SetupPacket.bRequest == AUDIO_REQUEST_GET_CUR)
                    {
                        pCtrl->EP0Buf[0] = (pAdcCtrl->flags & ADC_PLAY_MUTE) ? 1 : 0;
                        ret              = LPC_OK;
                    }
                    break;

                case AUDIO_VOLUME_CONTROL:
                    pBuf = (uint16_t *)pCtrl->EP0Buf;
                    switch (pCtrl->SetupPacket.bRequest)
                    {
                        case AUDIO_REQUEST_GET_CUR:
                            *pBuf = g_volume.volumeCur;
                            ret   = LPC_OK;
                            break;

                        case AUDIO_REQUEST_GET_MIN:
                            *pBuf = g_volume.volumeMin;
                            ret   = LPC_OK;
                            break;

                        case AUDIO_REQUEST_GET_MAX:
                            *pBuf = g_volume.volumeMax;
                            ret   = LPC_OK;
                            break;

                        case AUDIO_REQUEST_GET_RES:
                            *pBuf = g_volume.volumeRes;
                            ret   = LPC_OK;
                            break;
                    }
                    break;
            }
        }
    }

    return ret;
}

/* Audio Device Class Interface Set Request Callback
    Called automatically on ADC Interface Set Request */
static ErrorCode_t ADC_IF_SetRequest(USB_ADC_CTRL_T *pAdcCtrl, USB_CORE_CTRL_T *pCtrl)
{
    ErrorCode_t ret = ERR_USBD_INVALID_REQ;

    /* Feature Unit: volume control terminal ID = 2 */
    if (pCtrl->SetupPacket.wIndex.WB.H == 0x02)
    {
        /* Feature Unit: check if it is for master channel = 0 */
        if ((pCtrl->SetupPacket.wValue.WB.L == 0) && (pCtrl->SetupPacket.bRequest == AUDIO_REQUEST_SET_CUR))
        {
            /* Master Channel */
            switch (pCtrl->SetupPacket.wValue.WB.H)
            {
                case AUDIO_MUTE_CONTROL:
                    if (pCtrl->EP0Buf[0])
                    {
                        pAdcCtrl->flags |= ADC_PLAY_MUTE;
                    }
                    else
                    {
                        pAdcCtrl->flags &= ~ADC_PLAY_MUTE;
                    }

                    // Codec_Mute(pCtrl->EP0Buf[0]);
                    ret = (LPC_OK);
                    break;

                case AUDIO_VOLUME_CONTROL:
                    // Codec_SetVolume(volume);
                    ret = (LPC_OK);
                    break;
            }
        }
    }

    return ret;
}

/* Audio Device Class EndPoint Get Request Callback
    Called automatically on ADC EndPoint Get  */
static ErrorCode_t ADC_EP_GetRequest(USB_ADC_CTRL_T *pAdcCtrl, USB_CORE_CTRL_T *pCtrl)
{
    int dir         = SUBS_DMIC;
    ErrorCode_t ret = ERR_USBD_INVALID_REQ;

    switch (pCtrl->SetupPacket.wIndex.W)
    {
        case USB_ADC_IN_EP:
            dir = SUBS_DMIC;
            /* Feature Unit: Interface = 0, ID = 2 */
            if (pCtrl->SetupPacket.wValue.WB.L == 0)
            {
                /* Master Channel */
                if ((pCtrl->SetupPacket.wValue.WB.H == AUDIO_CONTROL_SAMPLING_FREQ) &&
                    (pCtrl->SetupPacket.bRequest == AUDIO_REQUEST_GET_CUR))
                {
                    pCtrl->EP0Buf[0] = (uint8_t)(pAdcCtrl->subs[dir].sample_rate & 0xFF);
                    pCtrl->EP0Buf[1] = (uint8_t)((pAdcCtrl->subs[dir].sample_rate >> 8) & 0xFF);
                    pCtrl->EP0Buf[2] = (uint8_t)((pAdcCtrl->subs[dir].sample_rate >> 16) & 0xFF);
                    ret              = (LPC_OK);
                }
            }
            break;

        default:
            break;
    }
    return ret;
}

/* Audio Device Class EndPoint Set Request Callback
    Called automatically on ADC EndPoint Set Request */
static ErrorCode_t ADC_EP_SetRequest(USB_ADC_CTRL_T *pAdcCtrl, USB_CORE_CTRL_T *pCtrl)
{
    int dir = SUBS_DMIC;
    uint32_t rate;
    ErrorCode_t ret = ERR_USBD_INVALID_REQ;

    switch (pCtrl->SetupPacket.wIndex.W)
    {
        case USB_ADC_IN_EP:
            dir = SUBS_DMIC;
            /* Feature Unit: Interface = 0, ID = 2 */
            if (pCtrl->SetupPacket.wValue.WB.L == 0)
            {
                /* Master Channel */
                if (pCtrl->SetupPacket.wValue.WB.H == AUDIO_CONTROL_SAMPLING_FREQ)
                {
                    rate = pCtrl->EP0Buf[0] | (pCtrl->EP0Buf[1] << 8) | (pCtrl->EP0Buf[2] << 16);
                    if (pCtrl->SetupPacket.bRequest == AUDIO_REQUEST_SET_CUR)
                    {
                        if (ret == LPC_OK)
                        {
                            pAdcCtrl->subs[dir].sample_rate = rate;
                            pAdcCtrl->subs[dir].packet_sz   = (rate / 1000) * AUDIO_BYTES_PER_SAMPLE;
                        }
                    }
                }
            }
            break;

        default:
            break;
    }
    return ret;
}

ErrorCode_t ADC_ep0_hdlr(USBD_HANDLE_T hUsb, void *data, uint32_t event)
{
    USB_CORE_CTRL_T *pCtrl   = (USB_CORE_CTRL_T *)hUsb;
    USB_ADC_CTRL_T *pAdcCtrl = (USB_ADC_CTRL_T *)data;
    ErrorCode_t ret          = ERR_USBD_UNHANDLED;

    if (pCtrl->SetupPacket.bmRequestType.BM.Type == REQUEST_CLASS)
    {
        switch (event)
        {
            case USB_EVT_SETUP:
                if ((pCtrl->SetupPacket.bmRequestType.BM.Recipient == REQUEST_TO_INTERFACE) &&
                    ((pCtrl->SetupPacket.wIndex.WB.L == USB_ADC_CTRL_IF) ||
                     (pCtrl->SetupPacket.wIndex.WB.L == USB_ADC_DMIC_IF)))
                {
                    switch (pCtrl->SetupPacket.bRequest)
                    {
                        case AUDIO_REQUEST_GET_CUR:
                        case AUDIO_REQUEST_GET_MIN:
                        case AUDIO_REQUEST_GET_MAX:
                        case AUDIO_REQUEST_GET_RES:

                            ret = ADC_IF_GetRequest(pAdcCtrl, pCtrl);
                            if (ret == LPC_OK)
                            {
                                pCtrl->EP0Data.pData = pCtrl->EP0Buf; /* point to data to be sent */
                                USBD_API->core->DataInStage(pCtrl);   /* send requested data */
                            }
                            break;

                        case AUDIO_REQUEST_SET_CUR:
                            pCtrl->EP0Data.pData = pCtrl->EP0Buf; /* data to be received */
                            ret                  = LPC_OK;
                            break;
                    }
                }
                else if (pCtrl->SetupPacket.bmRequestType.BM.Recipient == REQUEST_TO_ENDPOINT)
                {
                    switch (pCtrl->SetupPacket.bRequest)
                    {
                        case AUDIO_REQUEST_GET_CUR:
                        case AUDIO_REQUEST_GET_MIN:
                        case AUDIO_REQUEST_GET_MAX:
                        case AUDIO_REQUEST_GET_RES:
                            ret = ADC_EP_GetRequest(pAdcCtrl, pCtrl);
                            if (ret == LPC_OK)
                            {
                                pCtrl->EP0Data.pData = pCtrl->EP0Buf; /* point to data to be sent */
                                USBD_API->core->DataInStage(pCtrl);   /* send requested data */
                            }
                            break;

                        case AUDIO_REQUEST_SET_CUR:
                            pCtrl->EP0Data.pData = pCtrl->EP0Buf; /* data to be received */
                            ret                  = LPC_OK;
                            break;
                    }
                }
                break;

            case USB_EVT_OUT:
                if ((pCtrl->SetupPacket.bmRequestType.BM.Recipient == REQUEST_TO_INTERFACE) &&
                    ((pCtrl->SetupPacket.wIndex.WB.L == USB_ADC_CTRL_IF) || /* IF number correct? */
                     (pCtrl->SetupPacket.wIndex.WB.L == USB_ADC_DMIC_IF)))
                {
                    switch (pCtrl->SetupPacket.bRequest)
                    {
                        case AUDIO_REQUEST_SET_CUR:
                            ret = ADC_IF_SetRequest(pAdcCtrl, pCtrl);
                            if (ret == LPC_OK)
                            {
                                USBD_API->core->StatusInStage(pCtrl); /* send Acknowledge */
                            }
                            break;
                    }
                }
                else if (pCtrl->SetupPacket.bmRequestType.BM.Recipient == REQUEST_TO_ENDPOINT)
                {
                    switch (pCtrl->SetupPacket.bRequest)
                    {
                        case AUDIO_REQUEST_SET_CUR:
                            ret = ADC_EP_SetRequest(pAdcCtrl, pCtrl);
                            if (ret == LPC_OK)
                            {
                                USBD_API->core->StatusInStage(pCtrl); /* send Acknowledge */
                            }
                            break;
                    }
                }
                break;

            default:
                break;
        }
    }
    return ret;
}

static void USB_PrepareData()
{
    ADC_SUBSTREAM_T *pSubs = &g_AdcCtrl.subs[SUBS_DMIC];          //	get the line-in/mic handle
    uint8_t *idx           = (uint8_t *)&pSubs->wr_idx;           //	point to write index
    uint8_t *ptr           = (uint8_t *)pSubs->eTD[*idx].buf_ptr; //	point to buffer
    const uint32_t pkt_ct  = 32;                                  //	calculate packet count
    uint8_t k;

    /* copy audio wav data from flash to buffer */
    for (k = 0U; k < pkt_ct; k++)
    {
        if (audioPosition > (BUFFER_LENGTH - 1U))
        {
            audioPosition = 0U;
        }
        if ((k & 0x1U) == 1)
        {
            ptr[k] = g_data_buffer[audioPosition] >> 8U;
            audioPosition++;
        }
        else
        {
            ptr[k] = g_data_buffer[audioPosition];
        }
    }
    pSubs->eTD[*idx].buf_length = pkt_ct;
    (*idx)++;
    if (*idx >= NUM_DTDS)
    {
        *idx = 0; //	if we are past the packet count, then reset to the first packet again
    }
}

/** ADC_iso_in_hdlr: */
//__attribute__ ((section(".critical_text")))
ErrorCode_t ADC_iso_in_hdlr(USBD_HANDLE_T hUsb, void *data, uint32_t event)
{
    ADC_SUBSTREAM_T *pSubs   = (ADC_SUBSTREAM_T *)data;
    USB_ADC_CTRL_T *pAdcCtrl = &g_AdcCtrl;
    uint32_t *addr;
    EP_TD_T *eTD;

    if ((pAdcCtrl->flags & ADC_REC_DATAVALID) && (event == USB_EVT_IN))
    {
        do
        {
            /* Get the USB TD which needs to be re-programmed */
            addr = (uint32_t *)&pSubs->epQH[pSubs->usb_idx];
            pSubs->usb_idx ^= 1;
            /* update read index */
            INDEX_INC(pSubs->rd_idx);
            /* get next TD to be queued */
            eTD = &pSubs->eTD[pSubs->rd_idx];
            USB_PrepareData();
            /* make decision on how big the next packet should be to give
               host implicit feedback.
             */
            /* clear and re-queue the buffer */
            *addr &= ~0x3FFFFFF;
            /* re-queue the td at the end of list */
            *addr = (1ul << 31)   /* buffer active */
                    | (1ul << 26) /* ISOCH endpoint type */
                    | ((eTD->buf_length << 16) | (0xFFFF & (eTD->buf_ptr >> 6)));

            pSubs->rd_count += eTD->buf_length;

        } while ((pSubs->epQH[pSubs->usb_idx] & (1UL << 31)) == 0);
    }
    return LPC_OK;
}

/*****************************************************************************
 * Public functions
 ****************************************************************************/

/** ADC_init: USB device audio class init routine */
ErrorCode_t ADC_init(ADC_INIT_PARAM_T *adc_param)
{
    ErrorCode_t ret              = LPC_OK;
    USBD_API_INIT_PARAM_T *param = adc_param->param;
    USBD_HANDLE_T hUsb           = adc_param->hUsb;
    USB_ADC_CTRL_T *pAdcCtrl     = &g_AdcCtrl;
    EP_Queue_T *epQueue          = (EP_Queue_T *)USB0->EPLISTSTART;
    ADC_SUBSTREAM_T *pSubs;
    uint32_t maxp = 0;
    uint32_t mem_required;
    int j, i;

    /* calculate memory required */
    /* memory for capture buffers */
    mem_required = (NUM_DTDS * ((adc_param->maxp[SUBS_DMIC] + 0x3F) & ~0x3F));

    /* check for memory alignment */
    if ((param->mem_base & 0x3) || (param->mem_size < mem_required))
    {
        return ERR_USBD_BAD_MEM_BUF;
    }

    /* Init control structures with passed params */
    adc_param->pAdcCtrl = pAdcCtrl;
    pAdcCtrl->hUsb      = adc_param->hUsb;

    /* align to 64 byte boundary before allocating TDs */
    while (param->mem_base & 0x3F)
    {
        (param->mem_base)++;
        (param->mem_size)--;
    }

    /* initialize sub-streams - speaker and line-in/mic */
    for (i = 0; i < MAX_SUBSTREAMS; i++)
    {
        if (adc_param->ep_num[i] != 0)
        {
            /* init  sub-stream */
            pSubs              = &pAdcCtrl->subs[i];
            pSubs->ep_index    = ((adc_param->ep_num[i] & 0xF) << 1) + i; /* endpoint control bit */
            pSubs->ep_num      = adc_param->ep_num[i];                    /* endpoint number */
            pSubs->epQH        = (uint32_t *)&epQueue[pSubs->ep_index];
            pSubs->sample_rate = DEF_SAMPLE_RATE; /* default sample rate */
            pSubs->packet_sz   = (DEF_SAMPLE_RATE / 1000) * AUDIO_BYTES_PER_SAMPLE;
            maxp = pSubs->maxp  = ((adc_param->maxp[i] + 0x3F) & ~0x3F); /* MAX packet size */
            pSubs->wr_valid_idx = DTD_INVALID_IDX;

            for (j = 0; j < NUM_DTDS; j++)
            {
                pSubs->eTD[j].buf_ptr    = param->mem_base;
                pSubs->eTD[j].buf_length = maxp;
                param->mem_base += maxp;
                param->mem_size -= maxp;
            }
        }
    }

    /* register ep0 handler */
    ret = USBD_API->core->RegisterClassHandler(hUsb, ADC_ep0_hdlr, pAdcCtrl);
    if (ret == LPC_OK)
    {
        /* register ISO IN endpoint interrupt handler */
        if (adc_param->ep_num[SUBS_DMIC] != 0)
        {
            pSubs = &pAdcCtrl->subs[SUBS_DMIC];
            ret   = USBD_API->core->RegisterEpHandler(hUsb, pSubs->ep_index, ADC_iso_in_hdlr, pSubs);
        }
    }

    return ret;
}

/** ADC_start_xfr: */
//__attribute__ ((section(".critical_text")))
void ADC_start_xfr(USB_ADC_CTRL_T *pAdcCtrl, ADC_SUBSTREAM_T *pSubs, uint32_t mode)
{
    uint32_t ep_bit = (1ul << pSubs->ep_index);
    EP_TD_T *eTD;

    /* flush EP by setting skip bit */
    USB0->EPSKIP = ep_bit;
    while (USB0->EPSKIP & ep_bit)
    { /* wait until skip is cleared */
    }
    pSubs->epQH[0] &= ~(1ul << 31); /* clear active bit */
    pSubs->epQH[1] &= ~(1ul << 31); /* clear active bit */
    USB0->EPINUSE &= ~ep_bit;       /* reset the in buffer index to 0*/

    if (mode == ADC_USB_WRITER)
    {
        /* reset indexes */
        pSubs->wr_idx = pSubs->rd_idx = pSubs->usb_idx = 0;
        pSubs->wr_valid_idx                            = DTD_INVALID_IDX;
        /* accept maxp packets */
        pSubs->eTD[0].buf_length = pSubs->maxp;
        pSubs->eTD[1].buf_length = pSubs->maxp;
    }
    else
    {
        /* reset indexes */
        pSubs->rd_idx = pSubs->usb_idx = 0;
    }

    /* Initialize the Queue head */
    USB0->EPINUSE &= ~ep_bit; /* reset the in buffer index to 0*/
    eTD            = &pSubs->eTD[0];
    pSubs->epQH[0] = (1ul << 31)                         /* buffer active */
                     | (1ul << 26)                       /* ISOCH endpoint type */
                     | ((eTD->buf_length & 0x3FF) << 16) /* length of the rx buffer */
                     | ((eTD->buf_ptr >> 6) & 0xFFFF);   /* offset of rx buffer */
    eTD            = &pSubs->eTD[1];
    pSubs->epQH[1] = (1ul << 31)                         /* buffer active */
                     | (1ul << 26)                       /* ISOCH endpoint type */
                     | ((eTD->buf_length & 0x3FF) << 16) /* length of the rx buffer */
                     | ((eTD->buf_ptr >> 6) & 0xFFFF);   /* offset of rx buffer */
}

/** ADC_stop_xfr: */
void ADC_stop_xfr(USB_ADC_CTRL_T *pAdcCtrl, ADC_SUBSTREAM_T *pSubs, uint32_t mode)
{
    uint32_t ep_bit = (1ul << pSubs->ep_index);
    /* flush EP by setting skip bit */
    USB0->EPSKIP = ep_bit;
    while (USB0->EPSKIP & ep_bit)
    { /* wait until skip is cleared */
    }
    pSubs->epQH[0] &= ~(1ul << 31); /* clear active bit */
    pSubs->epQH[1] &= ~(1ul << 31); /* clear active bit */
    USB0->EPINUSE &= ~ep_bit;       /* reset the in buffer index to 0*/
    /* reset indexes */
    pSubs->wr_idx = pSubs->rd_idx = pSubs->usb_idx = 0;
    pSubs->wr_valid_idx                            = DTD_INVALID_IDX;
}

static void ADC_Reset_Event_NoCODECReset(USBD_HANDLE_T hUsb)
{
    USB_ADC_CTRL_T *pAdcCtrl = &g_AdcCtrl;

    g_AdcCtrl.flags &= ~ADC_PLAY_DATAVALID;

    /* reset counters */
    pAdcCtrl->subs[SUBS_DMIC].rd_count = pAdcCtrl->subs[SUBS_DMIC].wr_count = 0;
    pAdcCtrl->subs[SUBS_DMIC].total_diff                                    = 0;
    /* reset indexes */
    pAdcCtrl->subs[SUBS_DMIC].wr_idx = pAdcCtrl->subs[SUBS_DMIC].rd_idx = 0;
    pAdcCtrl->subs[SUBS_DMIC].wr_valid_idx                              = DTD_INVALID_IDX;
}

void ADC_Flush_Audio(void)
{
    USB_ADC_CTRL_T *pAdcCtrl = &g_AdcCtrl;

    /* reset counters */
    pAdcCtrl->subs[SUBS_DMIC].rd_count = pAdcCtrl->subs[SUBS_DMIC].wr_count = 0;
    pAdcCtrl->subs[SUBS_DMIC].total_diff                                    = 0;
    /* reset indexes */
    pAdcCtrl->subs[SUBS_DMIC].wr_idx = pAdcCtrl->subs[SUBS_DMIC].rd_idx = 0;
    pAdcCtrl->subs[SUBS_DMIC].wr_valid_idx                              = DTD_INVALID_IDX;
}

/** ADC_Suspend_Event: Suspend event */
ErrorCode_t ADC_Suspend_Event(USBD_HANDLE_T hUsb)
{
    /* schedule suspend */
    g_AdcCtrl.flags |= ADC_USB_SUSPEND;
    ADC_Reset_Event_NoCODECReset(hUsb);
    // powerFlags |= POWERFLAGS_SUSPEND;
    return LPC_OK;
}

/** USB_Resume_Event: Resume event */
ErrorCode_t ADC_Resume_Event(USBD_HANDLE_T hUsb)
{
    g_AdcCtrl.flags &= ~ADC_USB_SUSPEND;
    // powerFlags &= ~POWERFLAGS_SUSPEND;
    // powerFlags |= POWERFLAGS_RESUME;
    return LPC_OK;
}

/** ADC_Reset_Event: */
ErrorCode_t ADC_Reset_Event(USBD_HANDLE_T hUsb)
{
    // powerFlags = 0;
    ADC_Reset_Event_NoCODECReset(hUsb);

    return LPC_OK;
}

/*   This callback routine is called when ADC recieves capture start
            or end message (full bandwidth to zero bandwidth interface switch).
 */
ErrorCode_t Codec_Record(void *pCtrl, uint32_t altId)
{
    ErrorCode_t ret          = LPC_OK;
    USB_ADC_CTRL_T *pAdcCtrl = (USB_ADC_CTRL_T *)pCtrl;
    ADC_SUBSTREAM_T *pSubs   = &pAdcCtrl->subs[SUBS_DMIC];
    if (altId != 0)
    {
        /*##################################################################
           ROM driver enables ep so disable it untill we have data to send.
         */
        ADC_stop_xfr(pAdcCtrl, pSubs, ADC_USB_READER);
        /*##################################################################*/
        /* set capture state flags */
        pAdcCtrl->flags |= ADC_REC_DATAVALID;
        pAdcCtrl->flags |= ADC_RECORDING;
        ADC_start_xfr(&g_AdcCtrl, pSubs, ADC_USB_READER);
    }
    else
    {
        pAdcCtrl->flags &= ~(ADC_REC_DATAVALID | ADC_RECORDING);
        /* stop USB transfers */
        ADC_stop_xfr(pAdcCtrl, pSubs, ADC_USB_READER);

        if ((pAdcCtrl->flags & ADC_PLAYING) == 0)
        {
        }
    }
    /* reset buffer pointers */
    pSubs->wr_idx = 0;

    return ret;
}

/** ADC_Interface_Event:  */
ErrorCode_t ADC_Interface_Event(USBD_HANDLE_T hUsb)
{
    ErrorCode_t ret          = LPC_OK;
    USB_CORE_CTRL_T *pCtrl   = (USB_CORE_CTRL_T *)hUsb;
    USB_ADC_CTRL_T *pAdcCtrl = &g_AdcCtrl;
    uint16_t wIndex          = pCtrl->SetupPacket.wIndex.W;
    uint16_t wValue          = pCtrl->SetupPacket.wValue.W;

    /* write code to enable/disable audo playback when interface
       ALT setting is changed */
    if (wIndex == USB_ADC_DMIC_IF)
    {
        ret = Codec_Record(pAdcCtrl, wValue);
    }

    if (pAdcCtrl->flags & (ADC_PLAYING | ADC_RECORDING))
    {
        /* enable SOF event to track synchronization. Enable SOF interrupt in
           all cases except when controller is in ADC_BRIDGE_MODE and has
           SYNC endpoints (i.e. ADC_ASYNC_MODE is not set). */
    }
    else
    {
        pAdcCtrl->subs[SUBS_DMIC].rd_count = pAdcCtrl->subs[SUBS_DMIC].wr_count = 0;
        pAdcCtrl->subs[SUBS_DMIC].total_diff                                    = 0;
    }

    return ret;
}

//__attribute__ ((section("RamCodeSections")))
ErrorCode_t ADC_SOF_Event(USBD_HANDLE_T hUsb)
{
    /* Handle SOF event */
    return LPC_OK;
}
