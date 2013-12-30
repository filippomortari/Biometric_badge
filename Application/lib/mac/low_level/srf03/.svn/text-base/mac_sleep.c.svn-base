/**************************************************************************************************
    Filename:
    Revised:        $Date: 2006-11-02 23:23:21 -0800 (Thu, 02 Nov 2006) $
    Revision:       $Revision: 12526 $

    Description:

    Describe the purpose and contents of the file.

    Copyright (c) 2006 by Texas Instruments, Inc.
    All Rights Reserved.  Permission to use, reproduce, copy, prepare
    derivative works, modify, distribute, perform, display or sell this
    software and/or its documentation for any purpose is prohibited
    without the express written consent of Texas Instruments, Inc.
**************************************************************************************************/



/* ------------------------------------------------------------------------------------------------
 *                                          Includes
 * ------------------------------------------------------------------------------------------------
 */

/* hal */
#include "hal_types.h"

/* exported low-level */
#include "mac_low_level.h"

/* low-level specific */
#include "mac_sleep.h"
#include "mac_radio.h"
#include "mac_tx.h"
#include "mac_rx.h"
#include "mac_rx_onoff.h"

/* target specific */
#include "mac_radio_defs.h"

/* debug */
#include "mac_assert.h"

/* ------------------------------------------------------------------------------------------------
 *                                         Global Variables
 * ------------------------------------------------------------------------------------------------
 */

/* this variable is global so MAC_ASSERT can access it, unused outside this file otherwise */
uint8 macSleepState = MAC_SLEEP_STATE_CHIP_OFF;


/**************************************************************************************************
 * @fn          macSleepWakeUp
 *
 * @brief       Wake up the radio from sleep mode.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
void macSleepWakeUp(void)
{
  /* don't wake up radio if it's already awake */
  if (macSleepState == MAC_SLEEP_STATE_AWAKE)
  {
    return;
  }

  /* wake up MAC timer */
  MAC_RADIO_TIMER_WAKE_UP();

  /* if chip was completely off, restore from that state first */
  if (macSleepState == MAC_SLEEP_STATE_CHIP_OFF)
  {
    /* enable chip power, initialize */
    MAC_RADIO_ENABLE_VREG_SEQUENCE();
    MAC_RADIO_VREG_ON_RADIO_INIT();

    /* restore settings from pre-sleep */
    MAC_RADIO_RESTORE_FROM_VREG_OFF();
  }

  /* turn on oscillator, restore settings from pre-sleep */
  MAC_RADIO_ENABLE_OSC_SEQUENCE();
  MAC_RADIO_RESTORE_FROM_OSC_OFF();

  /* turn on the receiver if enabled */
  macRxOnRequest();

  /* set state variable */
  macSleepState = MAC_SLEEP_STATE_AWAKE;
}


/**************************************************************************************************
 * @fn          macSleep
 *
 * @brief       Puts radio into the selected sleep mode.
 *
 * @param       sleepState - selected sleep level, see #defines in .h file
 *
 * @return      TRUE if radio was successfully put into selected sleep mode.
 *              FALSE if it was not safe for radio to go to sleep.
 **************************************************************************************************
 */
uint8 macSleep(uint8 sleepState)
{
  MAC_ASSERT(macSleepState == MAC_SLEEP_STATE_AWAKE); /* radio must be awake to put it to sleep */
  MAC_ASSERT(macRxFilter == RX_FILTER_OFF); /* do not sleep when scanning or in promiscuous mode */
  MAC_ASSERT(!HAL_INTERRUPTS_ARE_ENABLED()); /* interrupts should be disabled to call this function */

 /* check to see if anything would prevent sleep */
 if (macRxActive || macTxActive || macRxEnableFlags)
 {
   /* sleep is not allowed */
   return(FALSE);
  }

  /* put MAC timer to sleep */
  MAC_RADIO_TIMER_SLEEP();

  /* update sleep state variable */
  macSleepState = sleepState;

  /* put radio in selected sleep mode */
  if (sleepState == MAC_SLEEP_STATE_OSC_OFF)
  {
    MAC_RADIO_TURN_OFF_OSC();
  }
  else
  {
    MAC_ASSERT(sleepState == MAC_SLEEP_STATE_CHIP_OFF);
    MAC_RADIO_TURN_OFF_VREG();
  }

  /* radio successfully put to sleep */
  return(TRUE);
}



/**************************************************************************************************
 *                                  Compile Time Integrity Checks
 **************************************************************************************************
 */
#if ((MAC_SLEEP_STATE_AWAKE == MAC_SLEEP_STATE_OSC_OFF) ||  \
     (MAC_SLEEP_STATE_AWAKE == MAC_SLEEP_STATE_CHIP_OFF))
#error "ERROR!  Non-unique state values."
#endif


/**************************************************************************************************
*/
