/**************************************************************************************************
    Filename:
    Revised:        $Date: 2006-11-14 11:20:09 -0800 (Tue, 14 Nov 2006) $
    Revision:       $Revision: 12700 $

    Description:

    Describe the purpose and contents of the file.

    Copyright (c) 2006 by Texas Instruments, Inc.
    All Rights Reserved.  Permission to use, reproduce, copy, prepare
    derivative works, modify, distribute, perform, display or sell this
    software and/or its documentation for any purpose is prohibited
    without the express written consent of Texas Instruments, Inc.
**************************************************************************************************/

#ifndef MAC_RX_H
#define MAC_RX_H

/* ------------------------------------------------------------------------------------------------
 *                                          Includes
 * ------------------------------------------------------------------------------------------------
 */
#include "hal_types.h"


/* ------------------------------------------------------------------------------------------------
 *                                           Defines
 * ------------------------------------------------------------------------------------------------
 */
#define RX_FILTER_OFF                   0
#define RX_FILTER_ALL                   1
#define RX_FILTER_NON_BEACON_FRAMES     2
#define RX_FILTER_NON_COMMAND_FRAMES    3


/* ------------------------------------------------------------------------------------------------
 *                                   Global Variable Externs
 * ------------------------------------------------------------------------------------------------
 */
extern uint8 macRxActive;
extern uint8 macRxFilter;


/* ------------------------------------------------------------------------------------------------
 *                                         Prototypes
 * ------------------------------------------------------------------------------------------------
 */
void macRxInit(void);
void macRxTxReset(void);
void macRxHaltCleanup(void);
void macRxThresholdIsr(void);
void macRxAckTxDoneCallback(void);


/**************************************************************************************************
 */
#endif
