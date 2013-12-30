/*********************************************************************
    Filename:       msa_Main.c
    Revised:        $Date: 2006-11-02 17:12:42 -0800 (Thu, 02 Nov 2006) $
    Revision:       $Revision: 12518 $

    Description: This file contains the main and callback functions

    Notes:

    Copyright (c) 2004 by Figure 8 Wireless, Inc., All Rights Reserved.
    Permission to use, reproduce, copy, prepare derivative works,
    modify, distribute, perform, display or sell this software and/or
    its documentation for any purpose is prohibited without the express
    written consent of Figure 8 Wireless, Inc.
*********************************************************************/


/**************************************************************************************************
 *                                           Includes
 **************************************************************************************************/
/* Hal Drivers */
#include "hal_types.h"
#include "hal_key.h"
#include "hal_timer.h"
#include "hal_uart.h"
#include "hal_drivers.h"


/* MAC Application Interface */
#include "mac_api.h"

/* Application */
#include "msa.h"

/* OSAL */
#include "OSAL.h"
#include "OSAL_Tasks.h"
#include "OnBoard.h"
#include "OSAL_PwrMgr.h"

/*******************************************
 *
 * VARIABLES
 *
 ********************************************/

/* Uart Variables*/
halUARTCfg_t UartCnfg;

/**************************************************************************************************
 * FUNCTIONS
 **************************************************************************************************/
/* This callback is triggered when the timer finish its tick */
void MSA_Main_TimerCallBack(uint8 timerId, uint8 channel, uint8 channelMode);

/* This callback is triggered when a key is pressed */
void MSA_Main_KeyCallback(uint8 keys, uint8 state);

/* callback triggered when an uart event is generated*/

void HalUARTCBack (uint8 port, uint8 event);

/**************************************************************************************************
 * @fn          main
 *
 * @brief       Start of application.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
int main(void)
{
  /* Initialize hardware */
  HAL_BOARD_INIT();

  /* Initialze the HAL driver */
  HalDriverInit();

  /* Initialize MAC */
  MAC_Init();

  /* Initialize the operating system */
  osal_init_system();

  /* Enable interrupts */
  HAL_ENABLE_INTERRUPTS();

  /* Setup OSAL Timer */
  HalTimerConfig ( OSAL_TIMER,                         // 16bit timer3
                   HAL_TIMER_MODE_CTC,                 // Clear Timer on Compare
                   HAL_TIMER_CHANNEL_SINGLE,           // Channel 1 - default
                   HAL_TIMER_CH_MODE_OUTPUT_COMPARE,   // Output Compare mode
                   FALSE,                              // Use interrupt
                   MSA_Main_TimerCallBack);            // Channel Mode

  /* Setup Keyboard callback */
  HalKeyConfig(MSA_KEY_INT_ENABLED, MSA_Main_KeyCallback);

  /* Initialize UART */
  UartCnfg.baudRate = HAL_UART_BR_9600;
  UartCnfg.callBackFunc = HalUARTCBack;
  UartCnfg.flowControl = FALSE;
  UartCnfg.flowControlThreshold = 0;  /* max Buffer Size in Byte*/
  UartCnfg.idleTimeout = 200;

  /*
   * halUARTOpen provvederà tramite la funzione halUartAllocBuffers ad allocare e inizializzare
   * le strutture dati RxUART e TxUART.
   */

  UartCnfg.rx = RxUART;
  UartCnfg.tx = TxUART;
  UartCnfg.rx.maxBufSize = UART_MAX_BUFFER_SIZE;
  UartCnfg.tx.maxBufSize = UART_MAX_BUFFER_SIZE;
  UartCnfg.intEnable = TRUE ;  /* enable or disable the interrupts */
  UartCnfg.configured = TRUE;

  uint8 status = HalUARTOpen(HAL_UART_PORT, &UartCnfg); /* passo l'indirizzo di memoria della struttura dati
  	  	  	  	  	  	  	  	  	  	  UartCnfg, Passaggio per riferimento!!!*/




  /* Start OSAL */
  osal_start_system(); // No Return from here

  return 0;
}

/**************************************************************************************************
                                           CALL-BACKS
**************************************************************************************************/


/**************************************************************************************************
 *
 * @fn      Osal_TimerCallBack()
 *
 * @brief   Update the timer per tick
 *
 * @param   timerId - The id of the timer that triggers the callback
 *          channel - the channel of the timer
 *          channelMode - the mode of the timer
 *
 * @return  local clock in milliseconds
 *
 **************************************************************************************************/
void MSA_Main_TimerCallBack ( uint8 timerId, uint8 channel, uint8 channelMode)
{
  /* Update OSAL timer tick if it's OSAL_TIMER */
  if ((timerId == OSAL_TIMER))
    osal_update_timers();
}

/**************************************************************************************************
 * @fn      MSA_KeyCallback
 *
 * @brief   Callback service for keys
 *
 * @param   keys  - keys that were pressed
 *          state - shifted
 *
 * @return  void
 **************************************************************************************************/
void MSA_Main_KeyCallback(uint8 keys, uint8 state)
{
  if ( MSA_TaskId != TASK_NO_TASK )
  {
    MSA_HandleKeys (keys, state);
  }
}

/**************************************************************************************************
 *
 * @fn      MSA_PowerMgr
 *
 * @brief   Enable/Disable and setup power saving related stuff
 *
 * @param   mode - PWRMGR_ALWAYS_ON or PWRMGR_BATTERY
 *
 * @return  void
 *
 **************************************************************************************************/
void MSA_PowerMgr(uint8 enable)
{
  /* enable OSAL power management */
  if (enable)
   osal_pwrmgr_device(PWRMGR_BATTERY);
  else
   osal_pwrmgr_device(PWRMGR_ALWAYS_ON);
}

/**************************************************************************************************
 *
 * @fn          HalUARTCBack
 *
 * @brief       This routine handles events of UART
 *
 * @param       port - serial port that has the event
 *              event - Event that cause the callback
 *
 * @return
 *
 **************************************************************************************************/
void HalUARTCBack (uint8 port, uint8 event){
	/*only idle timeout on rx buffer is handled*/
	//printvalue("UART callback evt",event);
	if ((port == HAL_UART_PORT) && (event == HAL_UART_RX_TIMEOUT)){
		mymessage = (uint8*) osal_msg_allocate(sizeof (uint8));
		if (mymessage!= NULL){
			*mymessage = MSA_UART_RX_TIMEOUT;
			osal_msg_send(MSA_TaskId,mymessage);
		}

	}
}


/*************************************************************************************************
**************************************************************************************************/
