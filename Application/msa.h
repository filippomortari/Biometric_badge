#ifndef MACSAMPLEAPP_H
#define MACSAMPLEAPP_H


/**************************************************************************************************
    Filename:       msa.h
    Revised:        $Date: 2006-11-14 15:00:46 -0800 (Tue, 14 Nov 2006) $
    Revision:       $Revision: 12706 $

    Description: This file contains the the Mac Sample Application protypes and
                 definitions

    Notes:

    Copyright (c) 2004 by Figure 8 Wireless, Inc., All Rights Reserved.
    Permission to use, reproduce, copy, prepare derivative works,
    modify, distribute, perform, display or sell this software and/or
    its documentation for any purpose is prohibited without the express
    written consent of Figure 8 Wireless, Inc.
**************************************************************************************************/

#ifdef __cplusplus
extern "C"
{
#endif

/**************************************************************************************************
 * INCLUDES
 **************************************************************************************************/
#include "hal_types.h"
#include "hal_uart.h"

/**************************************************************************************************
 *                                        User's  Defines
 **************************************************************************************************/

#define MSA_MAC_CHANNEL           MAC_CHAN_11   /* Default channel - change it to desired channel */

#define MSA_MAC_CHANNEL_ALL		  16

#define MSA_MAC_CHANNEL_ALL_MASK		  MAC_CHAN_11_MASK | MAC_CHAN_12_MASK | MAC_CHAN_13_MASK | MAC_CHAN_14_MASK | MAC_CHAN_15_MASK | MAC_CHAN_16_MASK | MAC_CHAN_17_MASK | MAC_CHAN_18_MASK | MAC_CHAN_19_MASK | MAC_CHAN_20_MASK | MAC_CHAN_21_MASK | MAC_CHAN_22_MASK |MAC_CHAN_23_MASK | MAC_CHAN_24_MASK | MAC_CHAN_25_MASK | MAC_CHAN_26_MASK

#define MSA_WAIT_PERIOD           100           /* Time between each packet - Lower = faster sending rate */

#define MSA_POLL_PERIOD           100           /* Period between each polling from the end device */

#define MSA_PAN_ID                0x11CC        /* PAN ID : 11CC*/
#define MSA_COORD_SHORT_ADDR      0x0030        /* Coordinator short address */

#define MSA_MAC_BEACON_ORDER      4            /* Setting beacon order to 15 will disable the beacon */
#define MSA_MAC_SUPERFRAME_ORDER  4            /* Setting superframe order to 15 will disable the superframe */

#define MSA_COORDINATOR			  0				/*variable to declare the device as a coordinator */

#define MSA_END_DEVICE			  1				/*variable to declare the device as an end device */

#define MSA_SENSOR  			  2				/*variable to declare the device as an unattached node */



#define MSA_PACKET_LENGTH         60            /* Min = 4, Max = 102 */


#define MSA_PWR_MGMT_ENABLED      FALSE         /* Enable or Disable power saving */

#define MSA_KEY_INT_ENABLED       FALSE         /*
                                                 * FALSE = Key Polling
                                                 * TRUE  = Key interrupt
                                                 *
                                                 * Notes: Key interrupt will not work well with 2430 EB because
                                                 *        all the operations using up/down/left/right switch will
                                                 *        no longer work. Normally S1 + up/down/left/right is used
                                                 *        to invoke the switches but on the 2430 EB board,  the
                                                 *        GPIO for S1 is used by the LCD.
                                                 */

#define UART_MAX_BUFFER_SIZE	MSA_PACKET_LENGTH	         /* UART max buffer in Byte = MSA_PACKET_LENGTH + MSA_HEADER_LENGTH */

#define HAL_UART_PORT 			HAL_UART_PORT_0

/**************************************************************************************************
 * CONSTANTS
 **************************************************************************************************/

/* Event IDs */
#define MSA_POLL_EVENT    	0x0002
#define PRINT_NEXT_ENERGY 	0x0004
//#define MSA_UART_RX_TIMEOUT	0x0008
//#define MSA_SEND_EVENT    	0x0010

#define MSA_DISASSOCIATE			24    /* disassociate*/
#define MSA_UART_RX_TIMEOUT 		25
#define MSA_SEND_EVENT 				26


/* Application State */
#define MSA_IDLE_STATE     0x00
#define MSA_SEND_STATE     0x01

/**************************************************************************************************
 * GLOBALS
 **************************************************************************************************/
extern uint8 MSA_TaskId;
extern uint8* mymessage;

/*uart buffers*/

extern halUARTBufControl_t RxUART;
extern halUARTBufControl_t TxUART;



/*********************************************************************
 * FUNCTIONS
 */

/*
 * Task Initialization for the Mac Sample Application
 */
extern void MSA_Init( uint8 task_id );

/*
 * Task Event Processor for the Mac Sample Application
 */
extern uint16 MSA_ProcessEvent( uint8 task_id, uint16 events );

/*
 * Handle keys
 */
extern void MSA_HandleKeys( uint8 keys, uint8 shift );

/*
 * Handle power saving
 */
extern void MSA_PowerMgr (uint8 mode);


extern void Msa_Uart_Received_Msg (void);

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* MACSAMPLEAPP_H */
