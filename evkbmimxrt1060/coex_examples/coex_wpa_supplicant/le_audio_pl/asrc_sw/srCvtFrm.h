/*
 * Copyright 2023 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/********************************************************************
 *
 *  FILENAME: srCvtFrm.h
 *
 *  DESCRIPTION: Frame based SRC with polyphase filter. 
 *
 *******************************************************************/
#ifndef _SR_CVT_FRM_H_
#define _SR_CVT_FRM_H_

#if defined(LE_AUDIO_SINK_SYNC_ENABLE) && (LE_AUDIO_SINK_SYNC_ENABLE > 0)

#define MAX_FLT_TAPS		168
#define MAX_FRM_SIZE		523

/******************** data structure ******************************/
typedef struct {
	int fsIn;				//input sampling rate
	int sfOut;				//output sampling rate
	int phs;				//total phases	
	int fltTaps;			//filter taps
	int frmSizeIn;			//input frame size
	int frmSizeOut;			//output frame size
	int cc;					//channel count
} SrCvtFrmCfg_t;

typedef struct {
	//cfg					
	int fsIn;			  	//input sampling rate	
	int sfOut;			  	//output sampling rate	
	int phs;			  	//total phases		
	int fltTaps;		  	//filter taps	
	short *lpfCoef;		  	//filter coef	

	int shift;			  	//shift	
	int rnd;				//round
	int scaleup;			//scale up 1.5 for 24k and 48k, down sampler only
	int frmSizeIn;			//input frame size 
	int frmSizeOut;			//output frame size 

	int fltPhaseAcc;		//phase
	int fltPhaseInc;		//phase increment
	int fltMaxPhaseAcc;		//phase acc max value

	int smplsToRead;		//number of new samples needed
	int smplsInHistBuf;		//valid samples in the hist buffer
	short histBuf[MAX_FLT_TAPS + 1];	//the filter buffer
} SrCvtFrm_t;


/**************************** data  ******************************/
extern int srcSupportedFreqs[];

extern SrCvtFrmCfg_t srcFrmCfgs[];
extern short coef_downAllRate[];
extern short coef_up_32_32[];

/******************** function prototype **************************/
void srCvtUpdateFreqOffset(SrCvtFrm_t *srCvt, double freqOffset);
void srCvtSetFrcSmpl(SrCvtFrm_t *srCvt, double smplFrc);
double srCvtGetFrcSmpl(SrCvtFrm_t *srCvt);

void initUpCvtFrm(SrCvtFrm_t *sfCvt, SrCvtFrmCfg_t *cfg, double freqOffset);
void initDownCvtFrm(SrCvtFrm_t *sfCvt, SrCvtFrmCfg_t *cfg, double freqOffset);
int upCvtFrm(SrCvtFrm_t *sfCvt, short *smplsIn, short *smplsOut);
int downCvtFrm(SrCvtFrm_t *sfCvt, short *smplsIn, short *smplsOut);

#endif /*defined(LE_AUDIO_SINK_SYNC_ENABLE) && (LE_AUDIO_SINK_SYNC_ENABLE > 0)*/
#endif //_SR_CVT_FRM_H_
