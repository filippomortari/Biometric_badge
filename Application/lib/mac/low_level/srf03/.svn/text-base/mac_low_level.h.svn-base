/**************************************************************************************************
    Filename:
    Revised:        $Date: 2006-11-16 13:51:15 -0800 (Thu, 16 Nov 2006) $
    Revision:       $Revision: 12756 $

    Description:

    Describe the purpose and contents of the file.

    Copyright (c) 2006 by Texas Instruments, Inc.
    All Rights Reserved.  Permission to use, reproduce, copy, prepare
    derivative works, modify, distribute, perform, display or sell this
    software and/or its documentation for any purpose is prohibited
    without the express written consent of Texas Instruments, Inc.
**************************************************************************************************/

#ifndef MAC_LOW_LEVEL_H
#define MAC_LOW_LEVEL_H

/* ------------------------------------------------------------------------------------------------
 *                                            Includes
 * ------------------------------------------------------------------------------------------------
 */
#include "hal_types.h"
#include "mac_high_level.h"


 /* ------------------------------------------------------------------------------------------------
 *                                            Defines
 * ------------------------------------------------------------------------------------------------
 */
/* identifies low-level code as specific to Chipcon SmartRF03(tm) technology */
#define MAC_LOW_LEVEL_SMARTRF03

#define MAC_BACKOFF_TIMER_DEFAULT_ROLLOVER  (((uint32) MAC_A_BASE_SUPERFRAME_DURATION) << 14)

/* macTxFrame() parameter values for txType */
#define MAC_TX_TYPE_SLOTTED_CSMA            0x00
#define MAC_TX_TYPE_UNSLOTTED_CSMA          0x01
#define MAC_TX_TYPE_SLOTTED                 0x02

/* macSleep() parameter values for sleepState */
#define MAC_SLEEP_STATE_OSC_OFF             0x01
#define MAC_SLEEP_STATE_CHIP_OFF            0x02

/* macRxPromiscuousMode() parameter values */
#define MAC_PROMISCUOUS_MODE_OFF            0x00
#define MAC_PROMISCUOUS_MODE_COMPLIANT      0x01
#define MAC_PROMISCUOUS_MODE_WITH_BAD_CRC   0x02


/* ------------------------------------------------------------------------------------------------
 *                                           Global Externs
 * ------------------------------------------------------------------------------------------------
 */
extern uint8 macTxSlottedDelay;


/* ------------------------------------------------------------------------------------------------
 *                                           Prototypes
 * ------------------------------------------------------------------------------------------------
 */

/* mac_low_level.c */
void macLowLevelInit(void);
void macLowLevelReset(void);

/* mac_random.c */
uint8 macRandomByte(void);

/* mac_sleep.c */
void macSleepWakeUp(void);
uint8 macSleep(uint8 sleepState);

/* mac_radio.c */
void macRadioSetPanID(uint16 panID);
void macRadioSetShortAddr(uint16 shortAddr);
void macRadioSetIEEEAddr(uint8 * pIEEEAddr);
void macRadioSetPanCoordinator(uint8 panCoordinator);
void macRadioSetTxPower(uint8 txPower);
void macRadioSetChannel(uint8 channel);
void macRadioStartScan(uint8 scanType);
void macRadioStopScan(void);
void macRadioEnergyDetectStart(void);
uint8 macRadioEnergyDetectStop(void);

/* mac_backoff_timer.c */
void macBackoffTimerSetRollover(uint32 rolloverBackoff);
void macBackoffTimerSetCount(uint32 backoff);
uint32 macBackoffTimerCount(void);
uint32 macBackoffTimerGetTrigger(void);
void macBackoffTimerSetTrigger(uint32 triggerBackoff);
void macBackoffTimerCancelTrigger(void);
void macBackoffTimerTriggerCallback(void);
void macBackoffTimerRolloverCallback(void);
int32 macBackoffTimerRealign(macRx_t *pMsg);

/* mac_tx.c */
void macTxFrame(uint8 txType);
void macTxFrameRetransmit(void);
void macTxCompleteCallback(uint8 failFlag);

/* mac_rx.c */
uint8 macRxCheckPendingCallback(void);
void macRxCompleteCallback(macRx_t * pMsg);
void macRxPromiscuousMode(uint8 mode);

/* mac_rx_onoff.c */
void macRxEnable(uint8 flags);
void macRxSoftEnable(uint8 flags);
void macRxDisable(uint8 flags);
void macRxHardDisable(void);

/**************************************************************************************************
 */
#endif
