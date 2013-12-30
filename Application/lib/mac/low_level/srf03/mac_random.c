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


/* ------------------------------------------------------------------------------------------------
 *                                          Includes
 * ------------------------------------------------------------------------------------------------
 */

/* hal */
#include "hal_types.h"

/* low-level MAC */
#include "mac_random.h"

/* target specific */
#include "mac_radio_defs.h"

/* debug */
#include "mac_sleep.h"
#include "mac_assert.h"


/* ------------------------------------------------------------------------------------------------
 *                                            Defines
 * ------------------------------------------------------------------------------------------------
 */
#define DEFAULT_RANDOM_SEED    0xBEEF


/**************************************************************************************************
 * @fn          macRandomInit
 *
 * @brief       Initializes the random seed for use in the random byte function.
 *              Function designed to be called once during power-up initialization.
 *              The radio should be off at this point.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
void macRandomInit(void)
{
  uint16 seed;
  uint8  i;

  MAC_ASSERT(macSleepState != MAC_SLEEP_STATE_AWAKE); /* radio should be off */

  /* power up radio */
  MAC_RADIO_ENABLE_VREG_SEQUENCE();

  /* power up oscillator */
  MAC_RADIO_ENABLE_OSC_SEQUENCE();

  /*
   *  Set radio for infinite reception.  Once radio reaches this state,
   *  it will stay in receive mode regardless RF activity.
   */
  MAC_RADIO_CONFIG_RX_INFINITE_RECEPTION();

  /* turn on the receiver */
  MAC_RADIO_RX_ON();

  /*
   *  Wait for radio to reach infinite reception state.  Once it does,
   *  The least significant bit of ADTSTH should be pretty random.
   */
  while (!MAC_RADIO_RANDOM_FSMSTATE_RX_INF());


  /* put 16 random bits into the seed value */
  seed = 0;
  for(i=0; i<16; i++)
  {
    /* use most random bit of analog to digital receive conversion to populate the random seed */
    seed = (seed << 1) | MAC_RADIO_RANDOM_ADC_BIT();
  }

  /*
   *  The seed value must not be zero.  If it is, the psuedo random sequence will be always be zero.
   *  There is a one in a billion something chance this seed could randomly be zero, more likely some
   *  type of hardware problem would cause this.  The following check makes sure this does not happen.
   */
  if (seed == 0x0000)
  {
    seed = DEFAULT_RANDOM_SEED;
  }

  /* set the seed value for psuedo random sequence */
  MAC_RADIO_RANDOM_SET_SEED(seed);

  /* turn off the receiver */
  MAC_RADIO_RXTX_OFF();

  /* take receiver out of infinite reception mode; set back to normal operation */
  MAC_RADIO_CONFIG_RX_NORMAL_OPERATION();

  /* turn off radio */
  MAC_RADIO_TURN_OFF_VREG();
}


/**************************************************************************************************
 * @fn          macRandomByte
 *
 * @brief       Uses special hardware feature to generate new random value based on truly
 *              random seed value.
 *
 * @param       none
 *
 * @return      a random byte
 **************************************************************************************************
 */
uint8 macRandomByte(void)
{
  return(MAC_RADIO_RANDOM_BYTE());
}


/**************************************************************************************************
*/
