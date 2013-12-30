/**************************************************************************************************
    Filename:
    Revised:        $Date: 2006-10-31 17:48:41 -0800 (Tue, 31 Oct 2006) $
    Revision:       $Revision: 12464 $

    Description:

    Describe the purpose and contents of the file.

    Copyright (c) 2006 by Texas Instruments, Inc.
    All Rights Reserved.  Permission to use, reproduce, copy, prepare
    derivative works, modify, distribute, perform, display or sell this
    software and/or its documentation for any purpose is prohibited
    without the express written consent of Texas Instruments, Inc.
**************************************************************************************************/

#ifndef MAC_MEM_H
#define MAC_MEM_H


/* ------------------------------------------------------------------------------------------------
 *                                          Includes
 * ------------------------------------------------------------------------------------------------
 */
#include "hal_types.h"


/* ------------------------------------------------------------------------------------------------
 *                                          Typedefs
 * ------------------------------------------------------------------------------------------------
 */
typedef volatile unsigned char XDATA macRam_t;


/* ------------------------------------------------------------------------------------------------
 *                                         Prototypes
 * ------------------------------------------------------------------------------------------------
 */
uint8 macMemReadRamByte(macRam_t * pRam);
void macMemWriteRam(macRam_t * pRam, uint8 * pData, uint8 len);
void macMemReadRam(macRam_t * pRam, uint8 * pData, uint8 len);
void macMemWriteTxFifo(uint8 * pData, uint8 len);
void macMemReadRxFifo(uint8 * pData, uint8 len);


/**************************************************************************************************
 */
#endif
