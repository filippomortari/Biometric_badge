/**************************************************************************************************
  Filename:       hal_driver.c
  Revised:        $Date: 2005/04/29 01:36:04 $
  Revision:       $Revision$

  Description:

  This file contains the interface to the Drivers Service.

  Notes:

  Copyright (c) 2006 by Texas Instruments, Inc.
  All Rights Reserved.  Permission to use, reproduce, copy, prepare
  derivative works, modify, distribute, perform, display or sell this
  software and/or its documentation for any purpose is prohibited
  without the express written consent of Texas Instruments, Inc.
**************************************************************************************************/


/**************************************************************************************************
 *                                            INCLUDES
 **************************************************************************************************/
#include "hal_types.h"
#include "OSAL.h"
#include "hal_drivers.h"
#include "hal_adc.h"
#include "hal_key.h"
#include "hal_lcd.h"
#include "hal_led.h"
#include "hal_timer.h"
#include "hal_uart.h"

/**************************************************************************************************
 *                                            MACROS
 **************************************************************************************************/



/**************************************************************************************************
 *                                          CONSTANTS
 **************************************************************************************************/


/**************************************************************************************************
 *                                          TYPEDEFS
 **************************************************************************************************/


/**************************************************************************************************
 *                                      GLOBAL VARIABLES
 **************************************************************************************************/
uint8 Hal_TaskID;

extern void HalLedUpdate( void ); /* Notes: This for internal only so it shouldn't be in hal_led.h */

/**************************************************************************************************
 *                                      FUNCTIONS - API
 **************************************************************************************************/

/**************************************************************************************************
 * @fn      Hal_Init
 *
 * @brief   Hal Initialization function.
 *
 * @param   task_id - Hal TaskId
 *
 * @return  None
 **************************************************************************************************/
void Hal_Init( uint8 task_id )
{
  /* Register task ID */
  Hal_TaskID = task_id;

}

/**************************************************************************************************
 * @fn      Hal_DriverInit
 *
 * @brief   Initialize HW - These need to be initialized before anyone.
 *
 * @param   task_id - Hal TaskId
 *
 * @return  None
 **************************************************************************************************/
void HalDriverInit (void)
{

  /* TIMER */
  HalTimerInit();

  /* ADC */
#if ((defined HAL_ADC) && (HAL_ADC == TRUE))
  HalAdcInit();
#endif



  /* LED */
#if ((defined HAL_LED) && (HAL_LED == TRUE))

  HalLedInit();

#endif

  /* LCD */
#if ((defined HAL_LCD) && (HAL_LCD == TRUE))

  HalLcdInit();

#endif

  /* UART */
#if ((defined ZAPP_P1) || (defined ZAPP_P2) || (defined ZTOOL_P1) || (defined ZTOOL_P2))

HalUARTInit();

#endif

  /* KEY */
#if ((defined HAL_KEY) && (HAL_KEY == TRUE))

  HalKeyInit();

#endif
}


/**************************************************************************************************
 * @fn      Hal_ProcessEvent
 *
 * @brief   Hal Process Event
 *
 * @param   task_id - Hal TaskId
 *          events - events
 *
 * @return  None
 **************************************************************************************************/
uint16 Hal_ProcessEvent( uint8 task_id, uint16 events )
{
  uint8 *msgPtr;

  if ( events & SYS_EVENT_MSG )
  {
    msgPtr = osal_msg_receive(Hal_TaskID);

    while (msgPtr)
    {
      /* Do something here - for now, just deallocate the msg and move on */

      /* De-allocate */
      osal_msg_deallocate( msgPtr );
      /* Next */
      msgPtr = osal_msg_receive( Hal_TaskID );
    }
    return events ^ SYS_EVENT_MSG;
  }

#ifdef BLINK_LEDS
  if ( events & HAL_LED_BLINK_EVENT )
  {
    HalLedUpdate();
    return events ^ HAL_LED_BLINK_EVENT;
  }
#endif

  if (events & HAL_KEY_EVENT)
  {

#if (defined HAL_KEY) && (HAL_KEY == TRUE)
    /* Check for keys */
    HalKeyPoll();

    /* if interrupt disabled, do next polling */
    if (!Hal_KeyIntEnable)
    {
      osal_start_timerEx( Hal_TaskID, HAL_KEY_EVENT, 100);
    }
#endif // HAL_KEY

    return events ^ HAL_KEY_EVENT;
  }

  /* Nothing interested, discard the message */
  return 0;

}

/**************************************************************************************************
 * @fn      Hal_ProcessPoll
 *
 * @brief   This routine will be called by OSAL to poll UART, TIMER...
 *
 * @param   task_id - Hal TaskId
 *
 * @return  None
 **************************************************************************************************/
void Hal_ProcessPoll ()
{

  /* Timer Poll */
  HalTimerTick();

  /* UART Poll */
#if (defined ZAPP_P1) || (defined ZAPP_P2) || (defined ZTOOL_P1) || (defined ZTOOL_P2)
  HalUARTPoll();
#endif

}


/**************************************************************************************************
**************************************************************************************************/

