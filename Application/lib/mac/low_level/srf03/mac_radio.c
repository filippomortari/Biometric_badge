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

/* high-level */
#include "mac_pib.h"

/* exported low-level */
#include "mac_low_level.h"

/* low-level specific */
#include "mac_radio.h"
#include "mac_tx.h"
#include "mac_rx.h"
#include "mac_rx_onoff.h"
#include "mac_sleep.h"
#include "mac_backoff_timer.h"

/* target specific */
#include "mac_radio_defs.h"

/* debug */
#include "mac_assert.h"


/* ------------------------------------------------------------------------------------------------
 *                                        Global Variables
 * ------------------------------------------------------------------------------------------------
 */
uint8 macPhyTxPower;
uint8 macPhyChannel;


/* ------------------------------------------------------------------------------------------------
 *                                        Local Variables
 * ------------------------------------------------------------------------------------------------
 */
static uint8 reqChannel;
static uint8 reqTxPower;


/**************************************************************************************************
 * @fn          macRadioInit
 *
 * @brief       Initialize radio software.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
void macRadioInit(void)
{
  /* variable initialization for this module */
  reqChannel    = MAC_RADIO_CHANNEL_DEFAULT;
  macPhyChannel = MAC_RADIO_CHANNEL_DEFAULT;
  reqTxPower    = MAC_RADIO_TX_POWER_DEFAULT;
  macPhyTxPower = MAC_RADIO_TX_POWER_DEFAULT;
}


/**************************************************************************************************
 * @fn          macRadioReset
 *
 * @brief       -
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
void macRadioReset(void)
{
  macRadioStopScan();
  macRadioEnergyDetectStop();
}


/**************************************************************************************************
 * @fn          macRadioSetPanCoordinator
 *
 * @brief       Configure the pan coordinator status of the radio
 *
 * @param       panCoordFlag - non-zero to configure radio to be pan coordinator
 *                             zero to configure radio as NON pan coordinator
 *
 * @return      none
 **************************************************************************************************
 */
void macRadioSetPanCoordinator(uint8 panCoordFlag)
{
  /* abstracted radio configuration */
  MAC_RADIO_SET_PAN_COORDINATOR(panCoordFlag);
}


/**************************************************************************************************
 * @fn          macRadioSetPanID
 *
 * @brief       Set the pan ID on the radio.
 *
 * @param       panID - 16 bit PAN identifier
 *
 * @return      none
 **************************************************************************************************
 */
void macRadioSetPanID(uint16 panID)
{
  /* abstracted radio configuration */
  MAC_RADIO_SET_PAN_ID(panID);
}


/**************************************************************************************************
 * @fn          macRadioSetShortAddr
 *
 * @brief       Set the short addrss on the radio.
 *
 * @param       shortAddr - 16 bit short address
 *
 * @return      none
 **************************************************************************************************
 */
void macRadioSetShortAddr(uint16 shortAddr)
{
  /* abstracted radio configuration */
  MAC_RADIO_SET_SHORT_ADDR(shortAddr);
}


/**************************************************************************************************
 * @fn          macRadioSetIEEEAddr
 *
 * @brief       Set the IEEE address on the radio.
 *
 * @param       pIEEEAddr - pointer to array holding 64 bit IEEE address; array must be little
 *                          endian format (starts with lowest signficant byte)
 *
 * @return      none
 **************************************************************************************************
 */
void macRadioSetIEEEAddr(uint8 * pIEEEAddr)
{
  /* abstracted radio configuration */
  MAC_RADIO_SET_IEEE_ADDR(pIEEEAddr);
}


/**************************************************************************************************
 * @fn          macRadioSetTxPower
 *
 * @brief       Set transmitter power of the radio.
 *
 * @param       minusDbm - the minus dBm for power but as a postive integer
 *
 * @return      none
 **************************************************************************************************
 */
void macRadioSetTxPower(uint8 minusDbm)
{
  halIntState_t  s;

  /* if the selected dBm is out of range, use the closest available */
  if (minusDbm > MAC_RADIO_TX_POWER_MAX_MINUS_DBM)
  {
    minusDbm = MAC_RADIO_TX_POWER_MAX_MINUS_DBM;
  }

  HAL_ENTER_CRITICAL_SECTION(s);

  /*
   *  Set the global variable reqTxPower.  This variable is referenced
   *  by the function macRadioUpdateTxPower() to write the radio register.
   *
   *  A lookup table is used to translate the power level to the register
   *  value.
   */
  reqTxPower = macRadioDefsTxPowerTable[minusDbm];

  /*
   *  If not receiving and not transmitting, update the radio power setting.  If power
   *  is not updated here, it will be updated and the end of receive or transmit.
   */
  if (!macRxActive && !macTxActive)
  {
    macRadioUpdateTxPower();
  }
  HAL_EXIT_CRITICAL_SECTION(s);
}


/**************************************************************************************************
 * @fn          macRadioUpdateTxPower
 *
 * @brief       Update the radio's transmit power if a new power level has been requested
 *
 * @param       reqTxPower - file scope variable that holds the last request power level
 *              macPhyTxPower - global variable that holds radio's set power level
 *
 * @return      none
 **************************************************************************************************
 */
void macRadioUpdateTxPower(void)
{
  halIntState_t  s;

  HAL_ENTER_CRITICAL_SECTION(s);
  /* see if the requested power setting is different from the actual radio setting */
  if (reqTxPower != macPhyTxPower)
  {
    /*
     *  A new power setting has been requested.  Update the shadow value and write the
     *  new value to the radio.
     */
    macPhyTxPower = reqTxPower;
    MAC_RADIO_SET_TX_POWER(macPhyTxPower);
  }
  HAL_EXIT_CRITICAL_SECTION(s);
}


/**************************************************************************************************
 * @fn          macRadioSetChannel
 *
 * @brief       Set radio channel.
 *
 * @param       channel - channel number, valid range is 11 through 26
 *
 * @return      none
 **************************************************************************************************
 */
void macRadioSetChannel(uint8 channel)
{
  MAC_ASSERT((channel >= 11) && (channel <= 28));  /* illegal channel */

  /* set requested channel */
  reqChannel = channel;

  /*
   *  If transmit is not active, update the radio hardware immediately.  If transmit is active,
   *  the channel will be updated at the end of the current transmit.
   */
  if (!macTxActive)
  {
    macRadioUpdateChannel();
  }
}


/**************************************************************************************************
 * @fn          macRadioUpdateChannel
 *
 * @brief       Update the radio channel if a new channel has been requested.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
void macRadioUpdateChannel(void)
{
  halIntState_t  s;

  /* if the channel has changed, set the radio to the new channel */
  HAL_ENTER_CRITICAL_SECTION(s);
  if (reqChannel != macPhyChannel)
  {
    macPhyChannel = reqChannel;
    HAL_EXIT_CRITICAL_SECTION(s);

    /* changing the channel stops any receive in progress */
    macRxOff();
    MAC_RADIO_SET_CHANNEL(macPhyChannel);
    macRxOnRequest();
  }
  else
  {
    HAL_EXIT_CRITICAL_SECTION(s);
  }
}


/**************************************************************************************************
 * @fn          macRadioStartScan
 *
 * @brief       Puts radio into selected scan mode.
 *
 * @param       scanMode - scan mode, see #defines in .h file
 *
 * @return      none
 **************************************************************************************************
 */
void macRadioStartScan(uint8 scanMode)
{
  MAC_ASSERT(macSleepState == MAC_SLEEP_STATE_AWAKE); /* radio must be awake */
  MAC_ASSERT(macRxFilter == RX_FILTER_OFF); /* all filtering must be off to start scan */

  /* set the receive filter based on the selected scan mode */
  if (scanMode == MAC_SCAN_ED)
  {
    macRxFilter = RX_FILTER_ALL;
  }
  else if (scanMode == MAC_SCAN_ORPHAN)
  {
    macRxFilter = RX_FILTER_NON_COMMAND_FRAMES;
  }
  else
  {
    MAC_ASSERT((scanMode == MAC_SCAN_ACTIVE) || (scanMode == MAC_SCAN_PASSIVE)); /* invalid scan type */
    macRxFilter = RX_FILTER_NON_BEACON_FRAMES;

    /* for active and passive scans, per spec the pan ID must be 0xFFFF */
    MAC_RADIO_SET_PAN_ID(0xFFFF);
  }
}


/**************************************************************************************************
 * @fn          macRadioStopScan
 *
 * @brief       Takes radio out of scan mode.  Note can be called if
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
void macRadioStopScan(void)
{
  macRxFilter = RX_FILTER_OFF;

  /* restore the pan ID (passive and active scans set pan ID to 0xFFFF) */
  MAC_RADIO_SET_PAN_ID(macPib.panId);
}


/**************************************************************************************************
 * @fn          macRadioEnergyDetectStart
 *
 * @brief       Initiates energy detect.  The highest energy detected is recorded from the time
 *              when this function is called until the energy detect is stopped.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
void macRadioEnergyDetectStart(void)
{
  MAC_RADIO_ENERGY_DETECT_START();
}


/**************************************************************************************************
 * @fn          macRadioEnergyDetectStop
 *
 * @brief       Called at completion of an energy detect.  Note:  can be called even if energy
 *              detect is already stopped (needed by reset).
 *
 * @param       none
 *
 * @return      highest energy detected
 **************************************************************************************************
 */
uint8 macRadioEnergyDetectStop(void)
{
  return (MAC_RADIO_ENERGY_DETECT_STOP());
}


/**************************************************************************************************
*/
