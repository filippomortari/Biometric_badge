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


/* ------------------------------------------------------------------------------------------------
 *                                          Includes
 * ------------------------------------------------------------------------------------------------
 */

/* hal */
#include "hal_types.h"
#include "hal_mcu.h"

/* exported low-level */
#include "mac_low_level.h"

/* low-level specific */
#include "mac_random.h"
#include "mac_radio.h"
#include "mac_rx.h"
#include "mac_tx.h"
#include "mac_rx_onoff.h"
#include "mac_backoff_timer.h"

/* target specific */
#include "mac_radio_defs.h"

/* debug */
#include "mac_assert.h"


/**************************************************************************************************
 * @fn          macLowLevelInit
 *
 * @brief       Initialize low-level MAC.  Called only once on system power-up.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
void macLowLevelInit(void)
{
  /* initialize mcu before anything else */
  MAC_RADIO_INIT();

  /* do any radio related power-up initialization */
  macRandomInit();

  /* software initialziation */
  macRadioInit();
  macRxOnOffInit();
  macRxInit();
  macTxInit();
  macBackoffTimerInit();

  /* enable RX interrtupts */
  MAC_RADIO_ENABLE_RX_THRESHOLD_INTERRUPT();
}


/**************************************************************************************************
 * @fn          macLowLevelReset
 *
 * @brief       Reset low-level MAC.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
void macLowLevelReset(void)
{
  MAC_ASSERT(!HAL_INTERRUPTS_ARE_ENABLED());   /* interrupts must be disabled */

  /* reset modules */
  macRxTxReset();
  macRadioReset();
  macBackoffTimerReset();

  /* power up the radio */
  macSleepWakeUp();
}


/**************************************************************************************************
*/
