/*! *********************************************************************************
* Copyright (c) 2015, Freescale Semiconductor, Inc.
* Copyright 2016-2021 NXP
* All rights reserved.
*
* \file
*
* SPDX-License-Identifier: BSD-3-Clause
********************************************************************************** */

#include "connectivity_test_menus.h"

/************************************************************************************
*************************************************************************************
* Public memory declarations
*************************************************************************************
************************************************************************************/

char * const cu8Logo[]={  
  "\f\r\n",
  "\n\n\r\n",
  " ####         ######      ##### ##########\n\r",
  " ######      # ######    ##### #############\n\r", 
  " #######     ## ######  ##### ###############\n\r",  
  " ########    ### ########### ####       #####\n\r",  
  " #### #####  #### ######### #####       #####\n\r",   
  " ####  ##### #### ######### #################\n\r",  
  " ####   ######## ########### ###############\n\r", 
  " ####     ##### ######  ##### ############\n\r",
  " ####      ### ######    ##### ##\n\r",
  " ####       # ######      ##### #\n\r\n\r",
  "\r          Connectivity Test Demo\n\n"           ,
  "\r\n -Press enter to start",
  NULL
};

/*@CMA, Conn Test. New string*/
char * const cu8MainMenu[]={  
  "\f\r  Connectivity Test Interface short cuts\n",
  "\r------------------------------------------\n",
  "\r -Press [t] for Tx operation\n",
  "\r -Press [r] for Rx operation\n",
  "\r -Press [q] for channel up\n",
  "\r -Press [w] for channel down\n",
  "\r -Press [a] for Power up\n",
  "\r -Press [s] for Power down\n",
  "\r -Press [d] to increase the XTAL Trim value\n",
  "\r -Press [f] to decrease the XTAL Trim value\n",
  "\r -Press [n] to increase the Payload\n",
  "\r -Press [m] to decrease the Payload\n",
  "\r -Press [k] to increase CCA Threshold in Carrier Sense Test\n",
  "\r -Press [l] to decrease CCA Threshold in Carrier Sense Test\n",
  "\r -Press [z] to toggle Acknoledgement(None/Ack/EnhAck)\n",
  "\r -Press [x] to change the source address for the packets\n",
  "\r -Press [c] to change the destination address for the packets\n",
  "\r These keys can be used all over the application to change \n",
  "\r the test parameters\n",
  "\r  ________________________________\n",
  "\r |                                |\n",
  "\r |   Select the Test to perform   |\n",
  "\r |________________________________|\n",
  "\r -Press [1] Continuous tests\n",
  "\r -Press [2] Packet Error Rate test\n",
  "\r -Press [3] Range test\n",
  "\r -Press [4] Carrier Sense and Transmission Control menu\n",
  "\r -Press [!] Reset MCU\n\r\n",
  NULL
};

/*@CMA, Conn Test. New string*/
char * const cu8ShortCutsBar[]={ 
  "\f\r\n",
  "\r----------------------------------------------------------------------\n",
  "\r   [t] Tx   [q] Ch+  [a] Pw+  [n] Pyld+  [l] CCAThr-  [d] XtalTrim+\n",  
  "\r   [r] Rx   [w] Ch-  [s] Pw-  [m] Pyld-  [k] CCAThr+  [f] XtalTrim-\n",  
  "\r   [z] toggle Ack for Tx (None/Ack/EnhAck)\n",
  "\r   [x] change the source address\n",
  "\r   [c] change the destination address\n",
  "\r----------------------------------------------------------------------\n\r",
  NULL
};

char * const cu8ContinuousTestMenu[]={ 
  "\r __________________________ \n",
  "\r|                          |\n",
  "\r|   Continuous Test Menu   |\n",
  "\r|__________________________|\n\r\n",
  "\r-Press [1] Idle\n",
  "\r-Press [2] Burst PRBS Transmission using packet mode\n",
  "\r-Press [3] Continuous Modulated Transmission\n",
  "\r-Press [4] Continuous Unmodulated Transmission\n",
  "\r-Press [5] Continuous Reception\n",
  "\r-Press [6] Continuous Energy Detect\n",
  "\r-Press [7] Continuous Scan\n",
  "\r-Press [8] Continuous Cca\n"
  "\r-Press [p] Previous Menu\n\r\n",
  "\rNow Running: ",
  NULL
};

char * const cu8PerTxTestMenu[]={ 
  "\r  ____________________________ \n",
  "\r |                            |\n",
  "\r |      PER Tx Test Menu      |\n",
  "\r |____________________________|\n\r\n",
  "\r  Choose the amount of packets to send:\n",
  "\r [0] - 1     Packet     [1] - 25    Packets\n",
  "\r [2] - 100   Packets    [3] - 500   Packets\n",
  "\r [4] - 1000  Packets    [5] - 2000  Packets\n",
  "\r [6] - 5000  Packets    [7] - 10000 Packets\n",
  "\r [8] - 65535 Packets\n\r\n",
  "\r-Press [p] Previous Menu\n\r\n",
  NULL
};

char * const cu8PerRxTestMenu[]={ 
  "\r  ______________________ \n",
  "\r |                      |\n",
  "\r |   PER Rx Test Menu   |\n",
  "\r |______________________|\n\r\n",
  "\r -Press [space bar] to start/stop Receiving Packets\n",
  "\r -Press [p] Previous Menu\n\r\n",
  NULL
};

#if gMpmMaxPANs_c == 2
char * const cu8MpmMenuPs[]={
  "\r  ______________________ \n",
  "\r |                      |\n",
  "\r |   Select Prescaler   |\n",
  "\r |______________________|\n\r\n",
  "\r [0] Time base 0.5 ms\n",
  "\r [1] Time base 2.5 ms\n",
  "\r [2] Time base 10  ms\n",
  "\r [3] Time base 50  ms\n\r\n",
  NULL
};
#endif

char * const cu8RangeTxTestMenu[]={ 
  "\r  ________________________ \n",
  "\r |                        |\n",
  "\r |   Range Tx Test Menu   |\n",
  "\r |________________________|\n\r\n",
  "\r -Press [space bar] to start/stop Transmiting Packets\n",
  "\r -Press [p] Previous Menu\n\r\n",
  NULL
};

char * const cu8RangeRxTestMenu[]={ 
  "\r  ________________________ \n",
  "\r |                        |\n",
  "\r |   Range Rx Test Menu   |\n",
  "\r |________________________|\n\r\n",
  "\r -Press [space bar] to start/stop Receiving Packets\n",
  "\r -Press [p] Previous Menu\n\r\n",
  NULL
};

/*@CMA, Conn Test. New menu*/
char * const cu8RadioRegistersEditMenu[]={ 
  "\r   ____________________________ \n",
  "\r  |                            |\n",
  "\r  | Radio Registers Edit Menu  |\n",
  "\r  |____________________________|\n\r\n",
  "\r  -Press [1] Write Direct Registers\n",
  "\r  -Press [2] Read  Direct Registers\n",
  "\r  -Press [3] Write Indirect Registers\n",
  "\r  -Press [4] Read  Indirect Registers\n",
  "\r  -Press [5] Dump  All Registers\n",
  "\r  -Press [p] Previous Menu\n\r\n",
  NULL
};

char * const cu8RadioCSTCSelectMenu[]={ 
  "\r   ___________________________________________________________ \n",
  "\r  |                                                           |\n",
  "\r  | Radio Carrier Sense and Transmission Control Select Menu  |\n",
  "\r  |___________________________________________________________|\n\r\n",
  "\r  -Press [1] Carrier Sense Test with un-modulation input signal\n",
  "\r  -Press [2] Transmission Control Test\n",
  "\r  -Press [p] Previous Menu\n\r\n",
  NULL
};

char * const cu8CsTcTestMenu[]={ 
  "\r  ____________________________ \n",
  "\r |                            |\n",
  "\r |     Tr Ctrl Test Menu      |\n",
  "\r |____________________________|\n\r\n",
  "\r  Choose the amount of packets to send:\n",
  "\r [0] - 1     Packet     [1] - 25    Packets\n",
  "\r [2] - 100   Packets    [3] - 500   Packets\n",
  "\r [4] - 1000  Packets    [5] - 2000  Packets\n",
  "\r [6] - 5000  Packets    [7] - 10000 Packets\n",
  "\r [8] - 65535 Packets\n\r\n",
  "\r-Press [p] Previous Menu\n\r\n",
  NULL
};

char * const cu8SelectTags[] ={
  " Channel select ",
  "  Power select  ",
  " Test Tx select ",
  " Test Rx select ",
  "Trim coarse tune",
  " Trim fine tune "
};

char * const cu8TxTestTags[] ={
  "     PER Tx     ",
  "    Range Tx    ",
  "   Cont. Idle   ",
  "  Cont. PRBS9   ",
  "Cont. Modulated ",
  "Cont. Unmodulatd"
};

char * const cu8RxTestTags[] ={
  "     PER Rx     ",
  "    Range Rx    ",
  "Cont. Reception ",
  "   Cont. Scan   ",
  "Cont.Energy Det."
};

char * const cu8TxModTestTags[] ={
  "0's\r\n",
  "1's\r\n",
  "\b\b\b\b\b PN9\r\n"
};

char * const cu8ContinuousTestTags[] ={
  "Idle mode",
  "Continuous Tx Modulated - All ",
  "Continuous Tx Unmodulated",
  "Continuous PRBS9"
};

/************************************************************************************
*************************************************************************************
* Private prototypes
*************************************************************************************
************************************************************************************/


/************************************************************************************
*************************************************************************************
* Private memory declarations
*************************************************************************************
************************************************************************************/


/************************************************************************************
*************************************************************************************
* Interface functions
*************************************************************************************
************************************************************************************/
/***********************************************************************************
*
* PrintMenu
*
************************************************************************************/
void PrintMenu(char * const pu8Menu[], serial_write_handle_t writeHandle)
{
  uint8_t u8Index = 0;
  while(pu8Menu[u8Index]){
    (void)SerialManager_WriteBlocking(writeHandle, (uint8_t *)pu8Menu[u8Index], strlen(pu8Menu[u8Index]));
    u8Index++;
  }
}

/************************************************************************************
*************************************************************************************
* private functions
*************************************************************************************
************************************************************************************/



