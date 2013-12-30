/**************************************************************************************************
    Filename:
    Revised:        $Date: 2006-12-05 14:28:04 -0800 (Tue, 05 Dec 2006) $
    Revision:       $Revision: 12971 $

    Description:

    Describe the purpose and contents of the file.

    Copyright (c) 2006 by Texas Instruments, Inc.
    All Rights Reserved.  Permission to use, reproduce, copy, prepare
    derivative works, modify, distribute, perform, display or sell this
    software and/or its documentation for any purpose is prohibited
    without the express written consent of Texas Instruments, Inc.
**************************************************************************************************/

#ifndef MAC_TX_H
#define MAC_TX_H


/* ------------------------------------------------------------------------------------------------
 *                                         Includes
 * ------------------------------------------------------------------------------------------------
 */
#include "hal_types.h"


/* ------------------------------------------------------------------------------------------------
 *                                          Define
 * ------------------------------------------------------------------------------------------------
 */

/* very specific values, modify with great care */
#define MAC_TXDONE_SUCCESS                  0x00
#define MAC_TXDONE_CHANNEL_BUSY             0x01
#define MAC_TXDONE_INSUFFICIENT_TIME        0x02


/* ------------------------------------------------------------------------------------------------
 *                                   Global Variable Externs
 * ------------------------------------------------------------------------------------------------
 */
extern uint8 macTxActive;
extern uint8 macTxFrameInQueueFlag;
extern uint8 macTxBe;
extern uint8 macTxType;
extern uint8 macTxCsmaBackoffDelay;
extern uint8 macTxListenForAck;


/* ------------------------------------------------------------------------------------------------
 *                                       Prototypes
 * ------------------------------------------------------------------------------------------------
 */
void macTxInit(void);
void macTxHaltCleanup(void);
void macTxStartQueuedFrame(void);
void macTxDoneCallback(uint8 status);
void macTxAckReceived(uint8 seqn, uint8 pendingFlag);
void macTxAckTimeoutCallback(void);
void macTxTimestampCallback(void);
void macTxCollisionWithRxCallback(void);


/**************************************************************************************************
 */
#endif
