/**************************************************************************************************
    Filename:
    Revised:        $Date: 2006-11-16 11:03:15 -0800 (Thu, 16 Nov 2006) $
    Revision:       $Revision: 12749 $

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
#include "hal_assert.h"
#include "hal_types.h"
#include "hal_board.h"
#include "hal_defs.h"


/* ------------------------------------------------------------------------------------------------
 *                                           Defines
 * ------------------------------------------------------------------------------------------------
 */
#define NUM_BUTTON_HELDS_TO_ESCAPE_FLASHING   10


/* ------------------------------------------------------------------------------------------------
 *                                          Enumerations
 * ------------------------------------------------------------------------------------------------
 */
enum
{
  DEBUG_DATA_RSTACK_HIGH_OFS,
  DEBUG_DATA_RSTACK_LOW_OFS,
  DEBUG_DATA_HW_BYTE_OFS,
  DEBUG_DATA_SIZE
};


/* ------------------------------------------------------------------------------------------------
 *                                            Macros
 * ------------------------------------------------------------------------------------------------
 */
#define DEBOUNCE_WAIT(expr)    { int i; for (i=0; i<500; i++) { if (!(expr)) i = 0; } }


/* ------------------------------------------------------------------------------------------------
 *                                           Global Variables
 * ------------------------------------------------------------------------------------------------
 */
uint8 halDebugData[DEBUG_DATA_SIZE];
uint8 escapeFatalError = 0; /* set to '1' via the debugger to continue executing program */


/**************************************************************************************************
 * @fn          macFatalError
 *
 * @brief       Blink LEDs to indicate an error.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
void halAssertFatalError(void)
{
  volatile uint32 iFlash;
  uint8 buttonHeld;
  uint8 iBit;
  uint8 iByte;

  HAL_DISABLE_INTERRUPTS();

  /*-------------------------------------------------------------------------------
   *  Copy debug data into dump memory.
   */
#ifdef HAL_MCU_AVR
  {
    uint8 * pStack;
    pStack = (uint8 *) SP;
    pStack++; /* point to return address on stack */
    halDebugData[DEBUG_DATA_RSTACK_HIGH_OFS] = *pStack;
    pStack++;
    halDebugData[DEBUG_DATA_RSTACK_LOW_OFS] = *pStack;
  }
  halDebugData[DEBUG_DATA_HW_BYTE_OFS] = EIMSK;
#endif

  /*-------------------------------------------------------------------------------
   *  Turn off all LEDs.
   */
  HAL_TURN_OFF_LED1();
  HAL_TURN_OFF_LED2();
  HAL_TURN_OFF_LED3();
  HAL_TURN_OFF_LED4();

  /*-------------------------------------------------------------------------------
   *  Master infinite loop.
   */
  for (;;)
  {
    buttonHeld = 0;

    /*-------------------------------------------------------------------------------
     *  "Hazard lights" loop.  A held keypress will exit this loop.
     */
    while (buttonHeld != NUM_BUTTON_HELDS_TO_ESCAPE_FLASHING)
    {
      /* escape hatch to continue execution */
      if (escapeFatalError)
      {
        escapeFatalError = 0;
        return;
      }

      /*delay */
      for (iFlash=0; iFlash<HAL_LED_FLASH_COUNT; iFlash++)
      {
      }

      /* toggle LEDS, the #ifdefs are in case HAL has logically remapped non-existent LEDs */
#if (HAL_NUM_LEDS >= 1)
      HAL_TOGGLE_LED1();
#if (HAL_NUM_LEDS >= 2)
      HAL_TOGGLE_LED2();
#if (HAL_NUM_LEDS >= 3)
      HAL_TOGGLE_LED3();
#if (HAL_NUM_LEDS >= 4)
      HAL_TOGGLE_LED4();
#endif
#endif
#endif
#endif

      /* break out of loop if button is held long enough */
      if (HAL_PUSH_BUTTON1())
      {
        buttonHeld++;
      }
      else
      {
        buttonHeld = 0;
      }
    }

    /*-------------------------------------------------------------------------------
     *  Just exited from "hazard lights" loop.
     */

    /* turn off all LEDs */
    HAL_TURN_OFF_LED1();
    HAL_TURN_OFF_LED2();
    HAL_TURN_OFF_LED3();
    HAL_TURN_OFF_LED4();

    /* wait for button release */
    DEBOUNCE_WAIT(!HAL_PUSH_BUTTON1());


    /* initialize for data dump loop */
    iBit  = 0;
    iByte = 0;

    /*-------------------------------------------------------------------------------
     *  Data dump loop.  A button press cycles data bits to an LED.
     */
    while (iByte < DEBUG_DATA_SIZE)
    {
      /* wait for key press */
      while(!HAL_PUSH_BUTTON1());

      /* turn on all LEDs for first bit of byte, turn on three LEDs if not */
      HAL_TURN_ON_LED1();
      HAL_TURN_ON_LED2();
      HAL_TURN_ON_LED3();
      if (iBit == 0)
      {
        HAL_TURN_ON_LED4();
      }
      else
      {
        HAL_TURN_OFF_LED4();
      }

      /* wait for debounced key release */
      DEBOUNCE_WAIT(!HAL_PUSH_BUTTON1());

      /* turn off all LEDs */
      HAL_TURN_OFF_LED1();
      HAL_TURN_OFF_LED2();
      HAL_TURN_OFF_LED3();
      HAL_TURN_OFF_LED4();

      /* output value of data bit to LED1 */
      if (halDebugData[iByte] & (1 << iBit))
      {
        HAL_TURN_ON_LED1();
      }
      else
      {
        HAL_TURN_OFF_LED1();
      }

      /* advance to next bit */
      iBit++;
      if (iBit == 8)
      {
        iBit = 0;
        iByte++;
      }
    }

    /*
     *  About to enter "hazard lights" loop again.  Turn off LED1 in case the last bit
     *  displayed happened to be one.  This guarantees all LEDs are off at the start of
     *  the flashing loop which uses a toggle operation to change LED states.
     */
    HAL_TURN_OFF_LED1();
  }
}


/* ------------------------------------------------------------------------------------------------
 *                                    Compile Time Assertions
 * ------------------------------------------------------------------------------------------------
 */

/* integrity check of type sizes */
HAL_ASSERT_SIZE(  int8, 1);
HAL_ASSERT_SIZE( uint8, 1);
HAL_ASSERT_SIZE( int16, 2);
HAL_ASSERT_SIZE(uint16, 2);
HAL_ASSERT_SIZE( int32, 4);
HAL_ASSERT_SIZE(uint32, 4);



/**************************************************************************************************
*/
