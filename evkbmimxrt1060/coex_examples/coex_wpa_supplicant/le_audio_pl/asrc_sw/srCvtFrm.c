/*
 * Copyright 2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/********************************************************************
 *
 *  FILENAME: srCvtFrm.c
 *
 *  DESCRIPTION: Frame based SRC with polyphase filter. 
 *
 *******************************************************************/
#if defined(LE_AUDIO_SINK_SYNC_ENABLE) && (LE_AUDIO_SINK_SYNC_ENABLE > 0)
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "srCvtFrm.h"
#include "fsl_debug_console.h"
/********************************************************************
 ** filter coefficients
 *******************************************************************/
short coef_downAllRate[8*168] = {
#include "downSmplAllRates.dat"
};

short coef_up_32_32[32 * 32] = {
#include "upSmpl_32_32_rev.dat"
};

/********************************************************************
 ** srCvtUpdateFreqOffset
 *
 *  FILENAME: srCvtFrm.c
 *
 *  PARAMETERS:	SrCvtFrm_t *srCvt
 *				double freqOffset, ppm
 *
 *  DESCRIPTION: Set the new phase increment
 *
 *  RETURNS: None
 *
 *******************************************************************/
void srCvtUpdateFreqOffset (SrCvtFrm_t *srCvt, double freqOffset)
{
	if (srCvt->phs == 32) {	//up SRC
		srCvt->fltPhaseInc = 
			(int)((1.0 + freqOffset) * (double)(srCvt->fltMaxPhaseAcc) * srCvt->fsIn / srCvt->sfOut);
	}
	else {	//down SRC
		srCvt->fltPhaseInc = (int)((1.0 + freqOffset) * (double)(srCvt->fltMaxPhaseAcc));
	}
}

/********************************************************************
 ** srCvtSetFrcSmpl
 *
 *  FILENAME: srCvtFrm.c
 *
 *  PARAMETERS:	SrCvtFrm_t *srCvt
 *				double smplFrc, the fractional sample, 0.0 <= smplFrc < 1.0
 *
 *  DESCRIPTION: Set the new phase
 *
 *  RETURNS: None
 *
 *******************************************************************/
void srCvtSetFrcSmpl (SrCvtFrm_t *srCvt, double smplFrc)
{
	if (srCvt->phs == 32) {	//the up SRC
		srCvt->fltPhaseAcc = 
			(int)(smplFrc * (double)(srCvt->fltMaxPhaseAcc) * srCvt->fsIn / srCvt->sfOut);
	}
	else {	//down SRC
		srCvt->fltPhaseAcc = (int)(smplFrc * (double)(srCvt->fltMaxPhaseAcc));
	}
}

/********************************************************************
 ** setPhaseIncFrm
 *
 *  FILENAME: srCvtFrm.c
 *
 *  PARAMETERS:	SrCvtFrm_t *srCvt
 *
 *  DESCRIPTION: Set the new phase increment
 *
 *  RETURNS: the fractional sample
 *
 *******************************************************************/
double srCvtGetFrcSmpl (SrCvtFrm_t *srCvt)
{
	double smplFrc;
	if (srCvt->phs == 32) {	//the up SRC
		smplFrc = (double)srCvt->fltPhaseAcc * srCvt->sfOut / srCvt->fsIn;
		smplFrc = (double)(srCvt->fltMaxPhaseAcc - smplFrc) /(double)(srCvt->fltMaxPhaseAcc);
	}
	else {	//down SRC
		smplFrc = (double)(srCvt->fltMaxPhaseAcc - srCvt->fltPhaseAcc) /(double)(srCvt->fltMaxPhaseAcc);
	}

	return (smplFrc);
}

/********************************************************************
 ** initUpCvtFrm
 *
 *  FILENAME: srCvtFrm.c
 *
 *  PARAMETERS:	SrCvtFrm_t *srCvt
 *				SrCvtCfg_t *cfg
 *				double freqOffset, ppm
 *
 *  DESCRIPTION: Initialize the CVT
 *
 *  RETURNS: None
 *
 *******************************************************************/
void initUpCvtFrm(SrCvtFrm_t *srCvt, SrCvtFrmCfg_t *cfg, double freqOffset) 
{
	int i;
		
 	//copy cfg
	srCvt->fsIn		= cfg->fsIn;
	srCvt->sfOut	= cfg->sfOut;
	srCvt->phs		= cfg->phs;			
	srCvt->fltTaps 	= cfg->fltTaps;
	srCvt->lpfCoef	= coef_up_32_32;
	srCvt->shift 	= 15;
	srCvt->rnd 		= 1 << 14;
	
	srCvt->frmSizeIn = cfg->frmSizeIn;
	srCvt->frmSizeOut = cfg->frmSizeOut;
	srCvt->smplsInHistBuf = srCvt->fltTaps;

	srCvt->fltPhaseInc = (int)((1.0 + freqOffset) * (double)(srCvt->phs << 24) * srCvt->fsIn / srCvt->sfOut);
	srCvt->fltMaxPhaseAcc = 1 << (24 + 5);

	//the input samples	
	srCvt->smplsToRead = srCvt->frmSizeIn + 1;

	for (i = 0; i <= MAX_FLT_TAPS; i++) {
		srCvt->histBuf[i] = 0;
	}
	srCvt->fltPhaseAcc = 0;
}

/********************************************************************
 ** initDownCvtFrm
 *
 *  FILENAME: srCvtFrm.c
 *
 *  PARAMETERS:	SrCvtFrm_t *srCvt
 *				SrCvtCfg_t *cfg
 *				double freqOffset
 *
 *  DESCRIPTION: Initialize the CVT
 *
 *  RETURNS: None
 *
 *******************************************************************/
void initDownCvtFrm(SrCvtFrm_t *srCvt, SrCvtFrmCfg_t *cfg, double freqOffset) 
{
	int i;

	int phInc = (1 << 28);
	phInc += phInc >> 1;

	//copy cfg
	srCvt->fltMaxPhaseAcc = phInc;
	srCvt->fsIn		= cfg->fsIn;
	srCvt->sfOut	= cfg->sfOut;
	srCvt->phs		= cfg->phs;			
	srCvt->fltTaps 	= cfg->fltTaps;
	srCvt->lpfCoef	= coef_downAllRate;

	//different sampling rate has different scale
	srCvt->scaleup = 0;
	if (cfg->sfOut == 8000) {
		srCvt->shift 	= 15;
	}
	else if (cfg->sfOut == 16000) {
		srCvt->shift 	= 14;
	}
	else if (cfg->sfOut == 32000) {
		srCvt->shift 	= 13;
	}
	else if (cfg->sfOut == 24000) {
		srCvt->shift 	= 14;
		srCvt->scaleup = 1;
	}
	else if (cfg->sfOut == 48000) {
		srCvt->shift = 13;
		srCvt->scaleup = 1;
	}
	else {
		printf ("Invalid output sampling rate: %d\n", cfg->sfOut);
	}
	srCvt->shift -= 2;
	srCvt->rnd = 1 << (srCvt->shift - 1);

	srCvt->frmSizeIn = cfg->frmSizeIn;
	srCvt->frmSizeOut = cfg->frmSizeOut;
	srCvt->smplsInHistBuf = srCvt->fltTaps + 1;

	srCvt->fltTaps = srCvt->fltTaps;
	//cvt with interpolater
	//may need to port to fixed point
	srCvt->fltPhaseInc = (int)((1.0 + freqOffset) * (double)phInc);

	srCvt->smplsToRead = srCvt->frmSizeIn + 1;

	for (i = 0; i <= MAX_FLT_TAPS; i++) {
		srCvt->histBuf[i] = 0;
	}
	srCvt->fltPhaseAcc = 0x800000;
}

/********************************************************************
 ** upCvtFlt
 *
 *  FILENAME: srCvtFrm.c
 *
 *  PARAMETERS:	short *pSmpl, pointer to the sample array
 *				short *coef, pointer to the coef 
 *				int taps, the number of taps of the filter
 *
 *  DESCRIPTION: run the fir filter
 *
 *  RETURNS: filtering result
 *	NOTE: May use inline ASM to reduce the CPU load
 *
 *******************************************************************/
static int upCvtFlt (short *pSmpl, short *coef, int taps) {
	int acc = 0;
	int i;
	//taps = 32, fixed
	for (i = 0; i < taps >> 3; i++) {
		acc += *coef++ * *pSmpl++;
		acc += *coef++ * *pSmpl++;
		acc += *coef++ * *pSmpl++;
		acc += *coef++ * *pSmpl++;
		acc += *coef++ * *pSmpl++;
		acc += *coef++ * *pSmpl++;
		acc += *coef++ * *pSmpl++;
		acc += *coef++ * *pSmpl++;
	}
	return acc;
}

/********************************************************************
 ** upCvtFrm
 *
 *  FILENAME: srCvtFrm.c
 *
 *  PARAMETERS:	SrCvtFrm_t *srCvt
 *				short *smplsIn
 *				short *smplsOut
 *
 *  DESCRIPTION: run the upsampler and the polyphase filter
 *
 *  RETURNS: the return the number of samples at the output sampling rate
 *
 *******************************************************************/
int upCvtFrm (SrCvtFrm_t *srCvt, short *smplsIn, short *smplsOut) {
	int fltPhase;
	short *pSmpl, *pSmpl2, *coef;
	int smplnm1, smpln;
	int interpPhase;
	int delta;
	short procBuffer[MAX_FLT_TAPS + MAX_FRM_SIZE + 2];
	int totalSmpls;
	int remainingSmpls;
	int smplsNeeded;
	int outputSmplCnt = 0;
	memcpy (procBuffer, srCvt->histBuf, srCvt->smplsInHistBuf << 1);
	memcpy (&procBuffer[srCvt->smplsInHistBuf], smplsIn, srCvt->frmSizeIn << 1);
	totalSmpls = srCvt->smplsInHistBuf + srCvt->frmSizeIn;
	 
	pSmpl = procBuffer;
	remainingSmpls = srCvt->frmSizeIn - ((srCvt->fltTaps + 1) - srCvt->smplsInHistBuf);

	while (remainingSmpls > 0) {
		//get filter phase
		fltPhase = (srCvt->fltPhaseAcc >> 24) & 0x1f;

		//run filter
		coef = &srCvt->lpfCoef[fltPhase * srCvt->fltTaps];
		smplnm1 = upCvtFlt (pSmpl, coef, srCvt->fltTaps); 

		//find next phase and the sample
		pSmpl2 = pSmpl;
		if (fltPhase + 1 >= srCvt->phs) {
			coef = srCvt->lpfCoef;
			pSmpl2++;
		}
		else {
			coef = &srCvt->lpfCoef[(fltPhase + 1) * srCvt->fltTaps];
		}
		smpln = upCvtFlt (pSmpl2, coef, srCvt->fltTaps); 

		//interpolator
		interpPhase = (int)(((srCvt->fltPhaseAcc & 0xffffff) + 512) >> 10);		//to use short for interpPhase without overflow, less than 0.01 dB loss
		delta = (int)(smpln - smplnm1);
		smplnm1 >>= 2;
		smplnm1 += (int)(((int64_t)interpPhase * delta) >> 16);					//use the ARM instruction SMLAWB
		smplnm1 = (smplnm1 + 0x1000) >> 13;										//shift = 15, fixed

		if (smplnm1 > 32767)
			smplnm1 = 32767;
		else if (smplnm1 < -32768)
			smplnm1 = -32768;
		//save to output buffer
		*smplsOut++ = (short)smplnm1;
		outputSmplCnt++;

		//phase update
		srCvt->fltPhaseAcc += srCvt->fltPhaseInc;

		smplsNeeded = 0;
		while (srCvt->fltPhaseAcc >= (srCvt->phs << 24)) {
			srCvt->fltPhaseAcc -= srCvt->phs << 24;
			smplsNeeded++;
		}
		remainingSmpls -= smplsNeeded;
		pSmpl += smplsNeeded;

	}
	srCvt->smplsToRead = pSmpl - procBuffer;
	srCvt->smplsInHistBuf = totalSmpls - srCvt->smplsToRead;
	memcpy (srCvt->histBuf, pSmpl, srCvt->smplsInHistBuf << 1);

	return (outputSmplCnt);
}

/********************************************************************
 ** downCvtFlt
 *
 *  FILENAME: srCvtFrm.c
 *
 *  PARAMETERS:	short *pSmpl, pointer to the sample array
 *				short *coef, pointer to the coef 
 *				int taps, the number of taps of the filter
 *				int stride, the update to coef pointer
 *
 *  DESCRIPTION: run the fir filter
 *
 *  RETURNS: filtering result
 *	NOTE: May use inline ASM to reduce the CPU load
 *
 *******************************************************************/
static int downCvtFlt (short *pSmpl, short *coef, int taps, int stride) {
	int acc = 0;

	if (taps < 0 || taps > 168) {
		return (0);
	}
	while (taps > 0) {
		taps -= 14;
		acc += *coef * *pSmpl++;
		coef += stride; 
		acc += *coef * *pSmpl++;
		coef += stride; 
		acc += *coef * *pSmpl++;
		coef += stride; 
		acc += *coef * *pSmpl++;
		coef += stride; 
		acc += *coef * *pSmpl++;
		coef += stride; 
		acc += *coef * *pSmpl++;
		coef += stride; 
		acc += *coef * *pSmpl++;
		coef += stride; 
		acc += *coef * *pSmpl++;
		coef += stride; 
		acc += *coef * *pSmpl++;
		coef += stride; 
		acc += *coef * *pSmpl++;
		coef += stride; 
		acc += *coef * *pSmpl++;
		coef += stride; 
		acc += *coef * *pSmpl++;
		coef += stride; 
		acc += *coef * *pSmpl++;
		coef += stride; 
		acc += *coef * *pSmpl++;
		coef += stride; 
	}
	return acc;
}

/********************************************************************
 ** downCvt
 *
 *  FILENAME: srCvtFrm.c
 *
 *  PARAMETERS:	SrCvtFrm_t *srCvt
 *				short *smplsIn
 *				short *smplsOut
 *
 *  DESCRIPTION: run the down sampler and the polyphase filter
 *
 *  RETURNS: the number of input samples for the next frame = frameSize +/-1
 *  		The number of samples in the first call should be frameSize + 1
 *
 *******************************************************************/
int downCvtFrm(SrCvtFrm_t *srCvt, short *smplsIn, short *smplsOut) {
	int i;
	int fltPhase;
	short *pSmpl, *pSmpl2, *coef;
	int smplnm1, smpln;
	int interpPhase;
	int delta;
	short procBuffer[MAX_FLT_TAPS + MAX_FRM_SIZE + 2];
	int totalSmpls;

	memcpy (procBuffer, srCvt->histBuf, srCvt->smplsInHistBuf << 1);
	memcpy (&procBuffer[srCvt->smplsInHistBuf], smplsIn, srCvt->smplsToRead << 1);
	totalSmpls = srCvt->smplsInHistBuf + srCvt->smplsToRead;
	 
	pSmpl = procBuffer;

	for (i = 0; i < srCvt->frmSizeOut; i++) {
		//get filter phase
		fltPhase = srCvt->fltPhaseAcc >> 23;
		while (fltPhase >= srCvt->phs) {
			fltPhase -= srCvt->phs;
		}

		//run filter
		coef = &srCvt->lpfCoef[srCvt->phs - 1 - fltPhase];
		smplnm1 = downCvtFlt (pSmpl, coef, srCvt->fltTaps, srCvt->phs);

		//find next phase and the sample
		pSmpl2 = pSmpl;
		if (fltPhase + 1 >= srCvt->phs) {
			coef = &srCvt->lpfCoef[srCvt->phs - 1];
			pSmpl2++;
		}
		else {
			coef = &srCvt->lpfCoef[(srCvt->phs - 1) - (fltPhase + 1)];
		}

		smpln = downCvtFlt (pSmpl2, coef, srCvt->fltTaps, srCvt->phs);

		//interpolator
		interpPhase = ((srCvt->fltPhaseAcc & 0x7fffff) + 256) >> 9;	//scale down by one bit
		delta = smpln - smplnm1;
		smplnm1 >>= 2;
		smplnm1 += (int)(((int64_t)interpPhase * delta) >> 16);		//use the ARM instruction SMLAWB, snr reduced by 0.05dB

		if (srCvt->scaleup) {
			smplnm1 += smplnm1 >> 1;
		}
		smplnm1 = (smplnm1 + srCvt->rnd) >> srCvt->shift;
		if (smplnm1 > 32767)
			smplnm1 = 32767;
		else if (smplnm1 < -32768)
			smplnm1 = -32768;
		//save to output buffer
		*smplsOut++ = (short)smplnm1;

		//phase update
		srCvt->fltPhaseAcc += srCvt->fltPhaseInc;

		while (srCvt->fltPhaseAcc >= (srCvt->phs << 23)) {
			srCvt->fltPhaseAcc -= srCvt->phs << 23;
			pSmpl++;
		}
	}
	srCvt->smplsToRead = pSmpl - procBuffer;
	srCvt->smplsInHistBuf = totalSmpls - srCvt->smplsToRead;
	memcpy (srCvt->histBuf, pSmpl, srCvt->smplsInHistBuf << 1);

	return (srCvt->smplsToRead);
}

#endif /*defined(LE_AUDIO_SINK_SYNC_ENABLE) && (LE_AUDIO_SINK_SYNC_ENABLE > 0)*/
