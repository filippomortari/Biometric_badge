/**************************************************************************************************
    Filename:
    Revised:        $Date: 2006-11-15 11:49:52 -0800 (Wed, 15 Nov 2006) $
    Revision:       $Revision: 12720 $

    Description:

    Describe the purpose and contents of the file.

    Copyright (c) 2006 by Texas Instruments, Inc.
    All Rights Reserved.  Permission to use, reproduce, copy, prepare
    derivative works, modify, distribute, perform, display or sell this
    software and/or its documentation for any purpose is prohibited
    without the express written consent of Texas Instruments, Inc.
**************************************************************************************************/



/* ------------------------------------------------------------------------------------------------
 *                                           Includes
 * ------------------------------------------------------------------------------------------------
 */

/* hal */
#include "hal_types.h"

/* high-level specific */
#include "mac_spec.h"

/* exported low-level */
#include "mac_low_level.h"

/* low-level specific */
#include "mac_backoff_timer.h"
#include "mac_tx.h"

/* target specific */
#include "mac_radio_defs.h"

/* debug */
#include "mac_assert.h"


/* ------------------------------------------------------------------------------------------------
 *                                            Defines
 * ------------------------------------------------------------------------------------------------
 */
#define COMPARE_STATE_ROLLOVER_BV                 BV(0)
#define COMPARE_STATE_TRIGGER_BV                  BV(1)
#define COMPARE_STATE_ARM_BV                      BV(2)

#define COMPARE_STATE_TRIGGER                     COMPARE_STATE_TRIGGER_BV
#define COMPARE_STATE_ROLLOVER                    COMPARE_STATE_ROLLOVER_BV
#define COMPARE_STATE_ROLLOVER_AND_TRIGGER        (COMPARE_STATE_ROLLOVER_BV | COMPARE_STATE_TRIGGER_BV)
#define COMPARE_STATE_ROLLOVER_AND_ARM_TRIGGER    (COMPARE_STATE_ROLLOVER_BV | COMPARE_STATE_ARM_BV)


/*
 *  The datasheet mentions a small delay on both receive and transmit side of approximately
 *  two microseconds.  The precise characterization is given below.
 *  (This data is not given in rev 1.03 datasheet)
 */
#define RX_TX_PROP_DELAY_AVG_USEC         ((MAC_RADIO_RX_TX_PROP_DELAY_MIN_USEC + MAC_RADIO_RX_TX_PROP_DELAY_MAX_USEC) / 2)
#define RX_TX_PROP_DELAY_AVG_TIMER_TICKS  ((uint16)(MAC_RADIO_TIMER_TICKS_PER_USEC() * RX_TX_PROP_DELAY_AVG_USEC + 0.5))

/*
 *  For slotted receives, the SFD signal is expected to occur on a specifc symbol boundary.
 *  This does *not* correspond to the backoff boundary.  The SFD signal occurs at an
 *  offset from the backoff boundary.  This is done for efficiency of related algorithms.
 *
 *  Once transmit is strobed there is a fixed delay until the SFD signal occurs.  The frame
 *  does not start over-the-air transmit until after an internal radio delay of 12 symbols.
 *  Once transmitting over-the-air, the preamble is sent (8 symbols) followed by the
 *  SFD field (2 symbols). After the SFD field completes, the SFD signal occurs.  This
 *  adds up to a total of 22 symbols from strobe to SFD signal.
 *
 *  Since 22 symbols spans more than a backoff (20 symbols) the modulus operation is used
 *  to find the symbol offset which is 2 symbols.
 *
 *  This math is derived formally via the pre-processor.
 */
#define SYMBOLS_FROM_STROBE_TO_PREAMBLE   12 /* from datasheet */
#define SYMBOLS_FROM_PREAMBLE_TO_SFD      (MAC_SPEC_PREAMBLE_FIELD_LENGTH + MAC_SPEC_SFD_FIELD_LENGTH)
#define SYMBOLS_FROM_STROBE_TO_SFD        (SYMBOLS_FROM_STROBE_TO_PREAMBLE + SYMBOLS_FROM_PREAMBLE_TO_SFD)
#define SYMBOLS_EXPECTED_AT_SFD           (SYMBOLS_FROM_STROBE_TO_SFD % MAC_A_UNIT_BACKOFF_PERIOD)

/* after all that formal math, make sure the result is as expected */
#if (SYMBOLS_EXPECTED_AT_SFD != 2)
#error "ERROR! Internal problem with pre-processor math of slotted alignment."
#endif


/*
 *  The expected SFD signal occurs at the symbol offset *plus* a small internal propagation delay
 *  internal to the radio.  This delay is given as the sum of a receive side delay and a transmit
 *  side delay.  When this delay is subtracted from the internal timer, the internal time base
 *  actually becomes the actual receive time *minus* the transmit delay.  This works out though.
 *  The transmit logic does *not* take into account this delay.  Since the timer is skewed by the
 *  transmit delay already, the transmits go out precisely on time.
 */
#define TIMER_TICKS_EXPECTED_AT_SFD   ((SYMBOLS_EXPECTED_AT_SFD * MAC_RADIO_TIMER_TICKS_PER_SYMBOL()) \
                                          + RX_TX_PROP_DELAY_AVG_TIMER_TICKS)


/* ------------------------------------------------------------------------------------------------
 *                                         Local Variables
 * ------------------------------------------------------------------------------------------------
 */
static uint32 backoffTimerRollover;
static uint32 backoffTimerTrigger;
static uint8 compareState;


/**************************************************************************************************
 * @fn          macBackoffTimerInit
 *
 * @brief       -
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
void macBackoffTimerInit(void)
{
  compareState = COMPARE_STATE_ROLLOVER;
  MAC_RADIO_BACKOFF_SET_COUNT(0);
  macBackoffTimerSetRollover(MAC_BACKOFF_TIMER_DEFAULT_ROLLOVER);
  MAC_RADIO_BACKOFF_COMPARE_CLEAR_INTERRUPT();
  MAC_RADIO_BACKOFF_COMPARE_ENABLE_INTERRUPT();
}


/**************************************************************************************************
 * @fn          macBackoffTimerReset
 *
 * @brief       -
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
void macBackoffTimerReset(void)
{
  MAC_RADIO_BACKOFF_COMPARE_DISABLE_INTERRUPT();
  macBackoffTimerInit();
}


/**************************************************************************************************
 * @fn          macBackoffTimerSetRollover
 *
 * @brief       Set rollover count of backoff timer.
 *
 * @param       rolloverBackoff - backoff count where count is reset to zero
 *
 * @return      none
 **************************************************************************************************
 */
void macBackoffTimerSetRollover(uint32 rolloverBackoff)
{
  halIntState_t  s;

  MAC_ASSERT(rolloverBackoff > MAC_RADIO_BACKOFF_COUNT());  /* rollover value must be greater than count */

  HAL_ENTER_CRITICAL_SECTION(s);
  backoffTimerRollover = rolloverBackoff;
  MAC_RADIO_BACKOFF_SET_COMPARE(rolloverBackoff);
  HAL_EXIT_CRITICAL_SECTION(s);
}


/**************************************************************************************************
 * @fn          macBackoffTimerSetCount
 *
 * @brief       -
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
void macBackoffTimerSetCount(uint32 backoff)
{
  halIntState_t  s;

  MAC_ASSERT(compareState == COMPARE_STATE_ROLLOVER);   /* trigger cannot be active if changing count */
  MAC_ASSERT(backoff < backoffTimerRollover);  /* count must be less than rollover value */

  HAL_ENTER_CRITICAL_SECTION(s);
  MAC_RADIO_BACKOFF_SET_COUNT(backoff);
  HAL_EXIT_CRITICAL_SECTION(s);
}


/**************************************************************************************************
 * @fn          macBackoffTimerCount
 *
 * @brief       -
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
uint32 macBackoffTimerCount(void)
{
  halIntState_t  s;
  uint32 backoffCount;

  HAL_ENTER_CRITICAL_SECTION(s);
  backoffCount = MAC_RADIO_BACKOFF_COUNT();
  HAL_EXIT_CRITICAL_SECTION(s);
  return(backoffCount);
}


/**************************************************************************************************
 * @fn          macBackoffTimerGetTrigger
 *
 * @brief       -
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
uint32 macBackoffTimerGetTrigger(void)
{
  return(backoffTimerTrigger);
}


/**************************************************************************************************
 * @fn          macBackoffTimerSetTrigger
 *
 * @brief       -
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
void macBackoffTimerSetTrigger(uint32 triggerBackoff)
{
  halIntState_t  s;

  MAC_ASSERT(triggerBackoff < backoffTimerRollover); /* trigger backoff must be less than rollover backoff */

  HAL_ENTER_CRITICAL_SECTION(s);
  backoffTimerTrigger = triggerBackoff;
  if (triggerBackoff > MAC_RADIO_BACKOFF_COUNT())
  {
    compareState = COMPARE_STATE_TRIGGER;
    MAC_RADIO_BACKOFF_SET_COMPARE(triggerBackoff);
  }
  else
  {
    if (triggerBackoff == 0)
    {
      compareState = COMPARE_STATE_ROLLOVER_AND_TRIGGER;
    }
    else
    {
      compareState = COMPARE_STATE_ROLLOVER_AND_ARM_TRIGGER;
    }
    MAC_RADIO_BACKOFF_SET_COMPARE(backoffTimerRollover);
  }
  HAL_EXIT_CRITICAL_SECTION(s);
}


/**************************************************************************************************
 * @fn          macBackoffTimerCancelTrigger
 *
 * @brief       -
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
void macBackoffTimerCancelTrigger(void)
{
  halIntState_t  s;

  HAL_ENTER_CRITICAL_SECTION(s);
  compareState = COMPARE_STATE_ROLLOVER;
  MAC_RADIO_BACKOFF_SET_COMPARE(backoffTimerRollover);
  HAL_EXIT_CRITICAL_SECTION(s);
}


/**************************************************************************************************
 * @fn          macBackoffTimerRealign
 *
 * @brief       -
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */

/*
 *  Realignment is accomplished by adjusting the internal time base to align with the expected
 *  reception time of an incoming frame.  The difference between the expected reception time and
 *  the actual reception time is computed and this difference is used to adjust the hardware
 *  timer count and backoff count.
 *
 *  The realignment is based on the SFD signal for the incoming frame.  The timer is aligned
 *  by adjusting it with the difference between the expected SFD time and the actual SFD time.
 */
int32 macBackoffTimerRealign(macRx_t *pMsg)
{
  uint16 timerDelayTicks;
  int32 backoffDelta;
  int32 backoffCount;

  MAC_ASSERT(!macTxActive);  /* cannot realign during transmit */

  /*-------------------------------------------------------------------------------
   *  Calculate the delta backoff difference between expected backoff count,
   *  which is zero, and the backoff count of the received frame.
   */

  /* since expected receive time is zero, the delta is simply the receive time */
  backoffDelta = pMsg->mac.timestamp;

  /* if the frame was received more than halfway to the rollover count, use a negative delta value */
  if (((uint32) backoffDelta) > (backoffTimerRollover / 2))
  {
    backoffDelta = backoffDelta - backoffTimerRollover;    /* result will be negative */
  }

  /*-------------------------------------------------------------------------------
   *  Calculate the number of timer ticks to delay that will align the internal
   *  time base with the received frame.
   */

  /* retrieve the timer count when frame was received */
  timerDelayTicks = pMsg->mac.timestamp2;

  /*
   *  Subtract the expected SFD time from the actual SFD time to find the needed
   *  timer adjustment. If subtracting the offset would result in a negative value,
   *  the tick delay must wrap around.
   */
  if (timerDelayTicks >= TIMER_TICKS_EXPECTED_AT_SFD)
  {
    /* since delay count is greater than or equal to offset, subtract it directly */
    timerDelayTicks = timerDelayTicks - TIMER_TICKS_EXPECTED_AT_SFD;
  }
  else
  {
    /*
     *  The expected time is greater that actualy time so it cannot be subtracted directly.
     *  The tick count per backoff is added to wrap around within the backoff.
     *  Since a wrap around did happen, the backoff delta is adjusted by one.
     */
    timerDelayTicks = timerDelayTicks - TIMER_TICKS_EXPECTED_AT_SFD + MAC_RADIO_TIMER_TICKS_PER_BACKOFF();
    backoffDelta--;
  }

  /*-------------------------------------------------------------------------------
   *  Calculate the new backoff count.
   */

  backoffCount = MAC_RADIO_BACKOFF_COUNT() - backoffDelta;

  if (backoffCount >= ((int32) backoffTimerRollover))
  {
    backoffCount -= backoffTimerRollover;
  }
  else if (backoffCount < 0)
  {
    backoffCount += backoffTimerRollover;
  }

  /////////////////////////////////////////////////////////////////////////////////////////
  // TBD : must account for rare "rollover just as timer is adjusted" condition
  /////////////////////////////////////////////////////////////////////////////////////////

  MAC_RADIO_TIMER_FORCE_DELAY(timerDelayTicks);
  MAC_RADIO_BACKOFF_SET_COUNT(backoffCount);

  return(backoffDelta);
}


/**************************************************************************************************
 * @fn          macBackoffTimerCompareIsr
 *
 * @brief       -
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
void macBackoffTimerCompareIsr(void)
{
  uint8 oldState;
  halIntState_t  s;

  HAL_ENTER_CRITICAL_SECTION(s);
  oldState = compareState;

  /* if compare is a rollover, set count to zero */
  if (oldState & COMPARE_STATE_ROLLOVER_BV)
  {
    MAC_RADIO_BACKOFF_SET_COUNT(0);
    macBackoffTimerRolloverCallback();
  }

  /* if compare is a trigger, reset for rollover and run the trigger callback */
  if (oldState & COMPARE_STATE_TRIGGER_BV)
  {
    compareState = COMPARE_STATE_ROLLOVER;
    MAC_RADIO_BACKOFF_SET_COMPARE(backoffTimerRollover);
    HAL_EXIT_CRITICAL_SECTION(s);
    macBackoffTimerTriggerCallback();
  }
  else if (oldState == COMPARE_STATE_ROLLOVER_AND_ARM_TRIGGER)
  {
    compareState = COMPARE_STATE_TRIGGER;
    MAC_RADIO_BACKOFF_SET_COMPARE(backoffTimerTrigger);
  }

  HAL_EXIT_CRITICAL_SECTION(s);
}


/**************************************************************************************************
*/
