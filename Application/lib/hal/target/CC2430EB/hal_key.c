/**************************************************************************************************
    Filename:       hal_key.c
    Revised:        $Date: 2006-11-09 18:51:38 -0800 (Thu, 09 Nov 2006) $
    Revision:       $Revision: 12659 $

    Description:

    This file contains the interface to the HAL KEY Service.

    Copyright (c) 2006 by Texas Instruments, Inc.
    All Rights Reserved.  Permission to use, reproduce, copy, prepare
    derivative works, modify, distribute, perform, display or sell this
    software and/or its documentation for any purpose is prohibited
    without the express written consent of Texas Instruments, Inc.
**************************************************************************************************/
/*********************************************************************
 NOTE: If polling is used, the hal_driver task schedules the KeyRead()
       to occur every 100ms.  This should be long enough to naturally
       debounce the keys.  The KeyRead() function remembers the key
       state of the previous poll and will only return a non-zero
       value if the key state changes.

 NOTE: If interrupts are used, the KeyRead() function is scheduled
       25ms after the interrupt occurs by the ISR.  This delay is used
       for key debouncing.  The ISR disables any further Key interrupt
       until KeyRead() is executed.  KeyRead() will re-enable Key
       interrupts after executing.  Unlike polling, when interrupts
       are enabled, the previous key state is not remembered.  This
       means that KeyRead() will return the current state of the keys
       (not a change in state of the keys).

 NOTE: If interrupts are used, the KeyRead() fucntion is scheduled by
       the ISR.  Therefore, the joystick movements will only be detected
       during a pushbutton interrupt caused by S1 or the center joystick
       pushbutton.

 NOTE: When a switch like S1 is pushed, the S1 signal goes from a normally
       high state to a low state.  This transition is typically clean.  The
       duration of the low state is around 200ms.  When the signal returns
       to the high state, there is a high likelihood of signal bounce, which
       causes a unwanted interrupts.  Normally, we would set the interrupt
       edge to falling edge to generate an interrupt when S1 is pushed, but
       because of the signal bounce, it is better to set the edge to rising
       edge to generate an interrupt when S1 is released.  The debounce logic
       can then filter out the signal bounce.  The result is that we typically
       get only 1 interrupt per button push.  This mechanism is not totally
       foolproof because occasionally, signal bound occurs during the falling
       edge as well.  A similar mechanism is used to handle the joystick
       pushbutton on the DB.  For the EB, we do not have independent control
       of the interrupt edge for the S1 and center joystick pushbutton.  As
       a result, only one or the other pushbuttons work reasonably well with
       interrupts.  The default is the make the S1 switch on the EB work more
       reliably.

*********************************************************************/

/**************************************************************************************************
 *                                            INCLUDES
 **************************************************************************************************/
#include "hal_mcu.h"
#include "hal_defs.h"
#include "hal_types.h"
#include "hal_drivers.h"
#include "hal_adc.h"
#include "hal_key.h"
#include "osal.h"

/**************************************************************************************************
 *                                              MACROS
 **************************************************************************************************/

/**************************************************************************************************
 *                                            CONSTANTS
 **************************************************************************************************/
#define HAL_KEY_BIT0   0x01
#define HAL_KEY_BIT1   0x02
#define HAL_KEY_BIT2   0x04
#define HAL_KEY_BIT3   0x08
#define HAL_KEY_BIT4   0x10
#define HAL_KEY_BIT5   0x20
#define HAL_KEY_BIT6   0x40
#define HAL_KEY_BIT7   0x80

#define HAL_KEY_RISING_EDGE   0
#define HAL_KEY_FALLING_EDGE  1

#define HAL_KEY_PDUP2           0x80
#define HAL_KEY_PDUP1           0x40
#define HAL_KEY_PDUP0           0x20

#define HAL_KEY_DEBOUNCE_VALUE  25
#define HAL_KEY_POLLING_VALUE   100


#if defined (HAL_BOARD_CC2430DB)
  #define HAL_KEY_SW_6_ENABLE
  #define HAL_KEY_SW_6_PORT     P0                      /* Port location of SW1 */
  #define HAL_KEY_SW_6_BIT      HAL_KEY_BIT1            /* Bit location of SW1 */
  #define HAL_KEY_SW_6_SEL      P0SEL                   /* Port Select Register for SW1 */
  #define HAL_KEY_SW_6_DIR      P0DIR                   /* Port Direction Register for SW1 */
  #define HAL_KEY_SW_6_IEN      IEN1                    /* Interrupt Enable Register for SW1 */
  #define HAL_KEY_SW_6_IENBIT   HAL_KEY_BIT5            /* Interrupt Enable bit for SW1 */
  #define HAL_KEY_SW_6_EDGE     HAL_KEY_RISING_EDGE     /* Type of interrupt for SW1 */
  #define HAL_KEY_SW_6_EDGEBIT  HAL_KEY_BIT0            /* EdgeType enable bit SW1 */
  #define HAL_KEY_SW_6_ICTL     PICTL                   /* Port Interrupt Control for SW1 */
  #define HAL_KEY_SW_6_ICTLBIT  HAL_KEY_BIT3            /* Interrupt enable bit for SW1 */
  #define HAL_KEY_SW_6_PXIFG    P0IFG                   /* Port Interrupt Flag for SW1 */

  #define HAL_KEY_JOYSTICK_ENABLE
  #define HAL_KEY_JOY_CHN   HAL_ADC_CHANNEL_6

  #define HAL_KEY_SW_5_ENABLE   /* 2430DB - SW5 is enabled based on key interrupt enable or not - see config */
  #define HAL_KEY_SW_5_PORT     P2                      /* Port location of SW5 */
  #define HAL_KEY_SW_5_BIT      HAL_KEY_BIT0            /* Bit location of SW5 */
  #define HAL_KEY_SW_5_SEL      P2SEL                   /* Port Select Register for SW5 */
  #define HAL_KEY_SW_5_DIR      P2DIR                   /* Port Direction Register for SW5 */
  #define HAL_KEY_SW_5_INP      P2INP                   /* Port Input Mode Register for SW5 */
  #define HAL_KEY_SW_5_IEN      IEN2                    /* Interrupt Enable Register for SW5 */
  #define HAL_KEY_SW_5_IENBIT   HAL_KEY_BIT1            /* Interrupt Enable bit for SW5 */
  #define HAL_KEY_SW_5_EDGE     HAL_KEY_FALLING_EDGE    /* Type of interrupt for SW5 */
  #define HAL_KEY_SW_5_EDGEBIT  HAL_KEY_BIT2            /* EdgeType enable bit SW5 */
  #define HAL_KEY_SW_5_ICTL     PICTL                   /* Port Interrupt Control for SW5 */
  #define HAL_KEY_SW_5_ICTLBIT  HAL_KEY_BIT5            /* Interrupt enable bit for SW5 */
  #define HAL_KEY_SW_5_PXIFG    P2IFG                   /* Port Interrupt Flag for SW5 */
#endif
#if defined (HAL_BOARD_CC2430EB)
  #define HAL_KEY_SW_6_ENABLE
  #define HAL_KEY_SW_6_PORT     P0                      /* Port location of SW1 */
  #define HAL_KEY_SW_6_BIT      HAL_KEY_BIT1            /* Bit location of SW1 */
  #define HAL_KEY_SW_6_SEL      P0SEL                   /* Port Select Register for SW1 */
  #define HAL_KEY_SW_6_DIR      P0DIR                   /* Port Direction Register for SW1 */
  #define HAL_KEY_SW_6_IEN      IEN1                    /* Interrupt Enable Register for SW1 */
  #define HAL_KEY_SW_6_IENBIT   HAL_KEY_BIT5            /* Interrupt Enable bit for SW1 */
  #define HAL_KEY_SW_6_EDGE     HAL_KEY_RISING_EDGE     /* Type of interrupt for SW1 */
  #define HAL_KEY_SW_6_EDGEBIT  HAL_KEY_BIT0            /* EdgeType enable bit SW1 */
  #define HAL_KEY_SW_6_ICTL     PICTL                   /* Port Interrupt Control for SW1 */
  #define HAL_KEY_SW_6_ICTLBIT  HAL_KEY_BIT3            /* Interrupt enable bit for SW1 */
  #define HAL_KEY_SW_6_PXIFG    P0IFG                   /* Port Interrupt Flag for SW1 */

  #define HAL_KEY_JOYSTICK_ENABLE
  #define HAL_KEY_JOY_CHN   HAL_ADC_CHANNEL_6

  #define HAL_KEY_SW_5_ENABLE
  #define HAL_KEY_SW_5_PORT     P0                      /* Port location of SW5 */
  #define HAL_KEY_SW_5_BIT      HAL_KEY_BIT5            /* Bit location of SW5 */
  #define HAL_KEY_SW_5_SEL      P0SEL                   /* Port Select Register for SW5 */
  #define HAL_KEY_SW_5_DIR      P0DIR                   /* Port Direction Register for SW5 */
  #define HAL_KEY_SW_5_INP      P0INP                   /* Port Input Mode Register for SW5 */
  #define HAL_KEY_SW_5_IEN      IEN1                    /* Interrupt Enable Register for SW5 */
  #define HAL_KEY_SW_5_IENBIT   HAL_KEY_BIT5            /* Interrupt Enable bit for SW5 */
  #define HAL_KEY_SW_5_EDGE     HAL_KEY_RISING_EDGE     /* Type of interrupt for SW5 */
  #define HAL_KEY_SW_5_EDGEBIT  HAL_KEY_BIT2            /* EdgeType enable bit SW5 */
  #define HAL_KEY_SW_5_ICTL     PICTL                   /* Port Interrupt Control for SW5 */
  #define HAL_KEY_SW_5_ICTLBIT  HAL_KEY_BIT4            /* Interrupt enable bit for SW5 */
  #define HAL_KEY_SW_5_PXIFG    P0IFG                   /* Port Interrupt Flag for SW5 */
#endif

/**************************************************************************************************
 *                                            TYPEDEFS
 **************************************************************************************************/


/**************************************************************************************************
 *                                        GLOBAL VARIABLES
 **************************************************************************************************/
static uint8 halKeySavedKeys;     /* used to store previous key state in polling mode */
static halKeyCBack_t pHalKeyProcessFunction;
bool Hal_KeyIntEnable;            /* interrupt enable/disable flag */
uint8 halSaveIntKey;              /* used by ISR to save state of interrupt-driven keys */

static uint8 HalKeySleepActive;
static uint8 HalKeyConfigured;

/**************************************************************************************************
 *                                        FUNCTIONS - Local
 **************************************************************************************************/
void halProcessKeyInterrupt (void);


/**************************************************************************************************
 *                                        FUNCTIONS - API
 **************************************************************************************************/
/**************************************************************************************************
 * @fn      HalKeyInit
 *
 * @brief   Initilize Key Service
 *
 * @param   none
 *
 * @return  None
 **************************************************************************************************/
void HalKeyInit( void )
{
  /* Initialize previous key to 0 */
  halKeySavedKeys = 0;

#if defined (HAL_KEY_SW_6_ENABLE)
  HAL_KEY_SW_6_SEL &= ~(HAL_KEY_SW_6_BIT);    /* Set pin function to GPIO */
  HAL_KEY_SW_6_DIR &= ~(HAL_KEY_SW_6_BIT);    /* Set pin direction to Input */
#endif

#if defined (HAL_KEY_SW_5_ENABLE)
  HAL_KEY_SW_5_SEL &= ~(HAL_KEY_SW_5_BIT);    /* Set pin function to GPIO */
  HAL_KEY_SW_5_DIR &= ~(HAL_KEY_SW_5_BIT);    /* Set pin direction to Input */
  HAL_KEY_SW_5_INP |= HAL_KEY_SW_5_BIT;       /* Set pin input mode to tri-state */
#endif

  /* Initialize callback function */
  pHalKeyProcessFunction  = NULL;

  /* Initialize sleep mode flag */
  HalKeySleepActive = FALSE;

  /* Start with key is not configured */
  HalKeyConfigured = FALSE;
}

/**************************************************************************************************
 * @fn      HalKeyConfig
 *
 * @brief   Configure the Key serivce
 *
 * @param   interruptEnable - TRUE/FALSE, enable/disable interrupt
 *          cback - pointer to the CallBack function
 *
 * @return  None
 **************************************************************************************************/
void HalKeyConfig (bool interruptEnable, halKeyCBack_t cback)
{
  /* Enable/Disable Interrupt or */
  Hal_KeyIntEnable = interruptEnable;

  /* Register the callback fucntion */
  pHalKeyProcessFunction = cback;

  /* Determine if interrupt is enable or not */
  if (Hal_KeyIntEnable)
  {

    /*
       Work around for CC2430DB when interrupt is enabled and SW5 (center joystick)
       is used. This SW5 uses P2 which also has debug lines connected to it. This
       causes contant interruption on P2INT_VECTOR. Disable the usage of P2 interrupt
       will stop this problem.
    */
    #if defined (HAL_BOARD_CC2430DB)
      #undef HAL_KEY_SW_5_ENABLE                      /* Dis-allow SW5 when key interrupt is enable */
    #endif

#if defined (HAL_KEY_SW_5_ENABLE)
    PICTL &= ~(HAL_KEY_SW_5_EDGEBIT);                 /* Set rising or falling edge */
  #if (HAL_KEY_SW_5_EDGE == HAL_KEY_FALLING_EDGE)
    PICTL |= HAL_KEY_SW_5_EDGEBIT;
  #endif
    HAL_KEY_SW_5_ICTL |= HAL_KEY_SW_5_ICTLBIT;        /* Set interrupt enable bit */
    HAL_KEY_SW_5_IEN |= HAL_KEY_SW_5_IENBIT;
    HAL_KEY_SW_5_PXIFG &= ~(HAL_KEY_SW_5_BIT);        /* Clear any pending interrupts */
#endif

#if defined (HAL_KEY_SW_6_ENABLE)
    PICTL &= ~(HAL_KEY_SW_6_EDGEBIT);                 /* Set rising or falling edge */
  #if (HAL_KEY_SW_6_EDGE == HAL_KEY_FALLING_EDGE)
    PICTL |= HAL_KEY_SW_6_EDGEBIT;
  #endif
    HAL_KEY_SW_6_ICTL |= HAL_KEY_SW_6_ICTLBIT;        /* Set interrupt enable bit */
    HAL_KEY_SW_6_IEN |= HAL_KEY_SW_6_IENBIT;
    HAL_KEY_SW_6_PXIFG &= ~(HAL_KEY_SW_6_BIT);        /* Clear any pending interrupts */
#endif

    /* Do this only after the hal_key is configured - to work with sleep stuff */
    if (HalKeyConfigured == TRUE)
    {
      osal_stop_timerEx( Hal_TaskID, HAL_KEY_EVENT);  /* Cancel polling if active */
    }
  }
  else    /* Interrupts NOT enabled */
  {

    /*
       Work around for CC2430DB when interrupt is enabled and SW5 (center joystick)
       is used. This SW5 uses P2 which also has debug lines connected to it. This
       causes contant interruption on P2INT_VECTOR. Disable the usage of P2 interrupt
       will stop this problem.
    */
    #if defined (HAL_BOARD_CC2430DB)
      #define HAL_KEY_SW_5_ENABLE                     /* Allow SW5 only when key interrupt is disable */
    #endif

#if defined (HAL_KEY_SW_6_ENABLE)
    HAL_KEY_SW_6_ICTL &= ~(HAL_KEY_SW_6_ICTLBIT);     /* Clear interrupt enable bit */
    HAL_KEY_SW_6_IEN &= ~(HAL_KEY_SW_6_IENBIT);
#endif

#if defined (HAL_KEY_SW_5_ENABLE)
    HAL_KEY_SW_5_ICTL &= ~(HAL_KEY_SW_5_ICTLBIT);     /* Clear interrupt enable bit */
    HAL_KEY_SW_5_IEN &= ~(HAL_KEY_SW_5_IENBIT);
#endif
    osal_start_timerEx (Hal_TaskID, HAL_KEY_EVENT, HAL_KEY_POLLING_VALUE);    /* Kick off polling */
  }

  /* Key now is configured */
  HalKeyConfigured = TRUE;
}

/**************************************************************************************************
 * @fn      HalKeyRead
 *
 * @brief   Read the current value of a key
 *
 * @param   None
 *
 * @return  keys - current keys status
 **************************************************************************************************/

uint8 HalKeyRead ( void )
{

  uint8 keys = 0;

#if defined (HAL_KEY_JOYSTICK_ENABLE)
  uint8 ksave0 = 0;
  uint8 ksave1;
  uint8 adc;
#endif

  /*
  *  If interrupts are enabled, get the status of the interrupt-driven keys from 'halSaveIntKey'
  *  which is updated by the key ISR.  If Polling, read these keys directly.
  */
  if (Hal_KeyIntEnable)
  {
    keys = halSaveIntKey;   /* Use key states saved by key ISR */
    halSaveIntKey = 0;
  }
  else    /* Interrupts Disabled, so Polling */
  {
#if defined (HAL_KEY_SW_6_ENABLE)
    if (!(HAL_KEY_SW_6_PORT & HAL_KEY_SW_6_BIT))    /* Key is active low */
    {
      keys |= HAL_KEY_SW_6;
    }
#endif
#if defined (HAL_KEY_SW_5_ENABLE)
    if (HAL_KEY_SW_5_PORT & HAL_KEY_SW_5_BIT)       /* Key is active high */
    {
      keys |= HAL_KEY_SW_5;
    }
#endif
  }

#if defined (HAL_KEY_JOYSTICK_ENABLE)
/*
*  The joystick control is encoded as an analog voltage.  Keep on reading
*  the ADC until two consecutive key decisions are the same.
*/

  do
  {
    ksave1 = ksave0;    /* save previouse key reading */

    adc = HalAdcRead (HAL_KEY_JOY_CHN, HAL_ADC_RESOLUTION_8);

    if  (CHVER == 0x01)
    {
      /* Rev B */
      if ((adc >= 90) && (adc <= 100))
      {
         ksave0 |= HAL_KEY_UP;
      }
      else if ((adc >= 75) && (adc <= 85))
      {
        ksave0 |= HAL_KEY_RIGHT;
      }
      else if ((adc >= 45) && (adc <= 55))
      {
        ksave0 |= HAL_KEY_LEFT;
      }
      else if (adc <= 10)
      {
        ksave0 |= HAL_KEY_DOWN;
      }
      else if ((adc >= 101) && (adc <= 115))
      {
      }
    }
    else
    {
       /* Rev C */
      if ((adc >= 90)  && (adc <= 104))
      {
         ksave0 |= HAL_KEY_UP;
      }
      else if ((adc >= 75)  && (adc <= 89))
      {
        ksave0 |= HAL_KEY_RIGHT;
      }
      else if ((adc >= 45)  && (adc <= 56))
      {
        ksave0 |= HAL_KEY_LEFT;
      }
      else if (adc <= 10)
      {
        ksave0 |= HAL_KEY_DOWN;
      }
      else if ((adc >= 105) && (adc <= 121))
      {
      }
    }

  } while (ksave0 != ksave1);

  keys |= ksave0;

#endif

  return keys;

}


/**************************************************************************************************
 * @fn      HalKeyPoll
 *
 * @brief   Called by hal_driver to poll the keys
 *
 * @param   None
 *
 * @return  None
 **************************************************************************************************/
void HalKeyPoll (void)
{
  uint8 keys = 0;

#if defined (HAL_KEY_JOYSTICK_ENABLE)
  uint8 ksave0 = 0;
  uint8 ksave1;
  uint8 adc;
#endif

  /*
  *  If interrupts are enabled, get the status of the interrupt-driven keys from 'halSaveIntKey'
  *  which is updated by the key ISR.  If Polling, read these keys directly.
  */
  if (Hal_KeyIntEnable)
  {
    keys = halSaveIntKey;   /* Use key states saved by key ISR */
    halSaveIntKey = 0;
  }
  else    /* Interrupts Disabled, so Polling */
  {
#if defined (HAL_KEY_SW_6_ENABLE)
    if (!(HAL_KEY_SW_6_PORT & HAL_KEY_SW_6_BIT))    /* Key is active low */
    {
      keys |= HAL_KEY_SW_6;
    }
#endif
#if defined (HAL_KEY_SW_5_ENABLE)
    if (HAL_KEY_SW_5_PORT & HAL_KEY_SW_5_BIT)       /* Key is active high */
    {
      keys |= HAL_KEY_SW_5;
    }
#endif
  }

#if defined (HAL_KEY_JOYSTICK_ENABLE)
/*
*  The joystick control is encoded as an analog voltage.  Keep on reading
*  the ADC until two consecutive key decisions are the same.
*/

  do
  {
    ksave1 = ksave0;    /* save previouse key reading */

    adc = HalAdcRead (HAL_KEY_JOY_CHN, HAL_ADC_RESOLUTION_8);

    if  (CHVER == 0x01)
    {
      /* Rev B */
      if ((adc >= 90) && (adc <= 100))
      {
         ksave0 |= HAL_KEY_UP;
      }
      else if ((adc >= 75) && (adc <= 85))
      {
        ksave0 |= HAL_KEY_RIGHT;
      }
      else if ((adc >= 45) && (adc <= 55))
      {
        ksave0 |= HAL_KEY_LEFT;
      }
      else if (adc <= 10)
      {
        ksave0 |= HAL_KEY_DOWN;
      }
      else if ((adc >= 101) && (adc <= 115))
      {
      }
    }
    else
    {
       /* Rev C */
      if ((adc >= 90)  && (adc <= 104))
      {
         ksave0 |= HAL_KEY_UP;
      }
      else if ((adc >= 75)  && (adc <= 89))
      {
        ksave0 |= HAL_KEY_RIGHT;
      }
      else if ((adc >= 45)  && (adc <= 56))
      {
        ksave0 |= HAL_KEY_LEFT;
      }
      else if (adc <= 10)
      {
        ksave0 |= HAL_KEY_DOWN;
      }
      else if ((adc >= 105) && (adc <= 121))
      {
      }
    }


  } while (ksave0 != ksave1);

  keys |= ksave0;

#endif

  /* Exit if polling and no keys have changed */
  if ((!Hal_KeyIntEnable) && (keys == halKeySavedKeys))
  {
    return;
  }

  halKeySavedKeys = keys;     /* Store the current keys for comparation next time */

  /* Invoke Callback if new keys were depressed */
  if (keys && (pHalKeyProcessFunction))
  {
    (pHalKeyProcessFunction) (keys, HAL_KEY_STATE_NORMAL);
  }

  if (Hal_KeyIntEnable)   /* re-enable interrupts */
  {
#if defined (HAL_KEY_SW_6_ENABLE)
    HAL_KEY_SW_6_ICTL |= HAL_KEY_SW_6_ICTLBIT;          /* Set interrupt enable bit */
#endif
#if defined (HAL_KEY_SW_5_ENABLE)
    HAL_KEY_SW_5_ICTL |= HAL_KEY_SW_5_ICTLBIT;          /* Set interrupt enable bit */
#endif
  }

}


/**************************************************************************************************
 * @fn      halProcessKeyInterrupt
 *
 * @brief   Checks to see if it's a valid key interrupt, saves interrupt driven key states for
 *          processing by HalKeyRead(), and debounces keys by scheduling HalKeyRead() 25ms later.
 *
 * @param
 *
 * @return
 **************************************************************************************************/
void halProcessKeyInterrupt (void)
{
  bool    valid=FALSE;

#if defined (HAL_KEY_SW_6_ENABLE)
  if (HAL_KEY_SW_6_PXIFG & HAL_KEY_SW_6_BIT)      /* Interrupt Flag has been set */
  {
    halSaveIntKey |= HAL_KEY_SW_6;
    HAL_KEY_SW_6_PXIFG &= ~(HAL_KEY_SW_6_BIT);    /* Clear Interrupt Flag */
    valid = TRUE;
  }
#endif

#if defined (HAL_KEY_SW_5_ENABLE)
  if (HAL_KEY_SW_5_PXIFG & HAL_KEY_SW_5_BIT)      /* Interrupt Flag has been set */
  {
    halSaveIntKey |= HAL_KEY_SW_5;
    HAL_KEY_SW_5_PXIFG &= ~(HAL_KEY_SW_5_BIT);    /* Clear Interrupt Flag */
    valid = TRUE;
  }
#endif

  /*
  *  If a valid key interrupt occurred, disable any further key interrupts and perform
  *  debouncing by scheduling the Key Event 25ms later.
  *  Special case when power saving is active and it's in sleep mode, dont process the
  *  key, store the key press and process it again when exit sleep
  */

  if (valid)
  {
#if defined (HAL_KEY_SW_6_ENABLE)
    HAL_KEY_SW_6_ICTL &= ~(HAL_KEY_SW_6_ICTLBIT);     /* Clear interrupt enable bit */
#endif
#if defined (HAL_KEY_SW_5_ENABLE)
    HAL_KEY_SW_5_ICTL &= ~(HAL_KEY_SW_5_ICTLBIT);     /* Clear interrupt enable bit */
#endif
    if (!HalKeySleepActive)
    {
      osal_start_timerEx (Hal_TaskID, HAL_KEY_EVENT, HAL_KEY_DEBOUNCE_VALUE);
    }
  }
}

/**************************************************************************************************
 * @fn      HalKeyEnterSleep
 *
 * @brief  - Get called to enter sleep mode
 *
 * @param
 *
 * @return
 **************************************************************************************************/
void HalKeyEnterSleep ( void )
{
  /* Sleep!!! */
  HalKeySleepActive = TRUE;
}

/**************************************************************************************************
 * @fn      HalKeyExitSleep
 *
 * @brief   - Get called when sleep is over
 *
 * @param
 *
 * @return  - return saved keys
 **************************************************************************************************/
uint8 HalKeyExitSleep ( void )
{
  /* Wake up!!! */
  HalKeySleepActive = FALSE;

  /* Read the keys and process as normal */
  HalKeyPoll();

  /* return keys */
  return ( halKeySavedKeys );
}

/***************************************************************************************************
 *                                    INTERRUPT SERVICE ROUTINE
 ***************************************************************************************************/

/**************************************************************************************************
 * @fn      halKeyPort0Isr
 *
 * @brief   Port0 ISR
 *
 * @param
 *
 * @return
 **************************************************************************************************/
HAL_ISR_FUNCTION( halKeyPort0Isr, P0INT_VECTOR )
{
  halProcessKeyInterrupt ();
}

/**************************************************************************************************
 * @fn      halKeyPort1Isr
 *
 * @brief   Port1 ISR
 *
 * @param
 *
 * @return
 **************************************************************************************************/
HAL_ISR_FUNCTION( halKeyPort1Isr, P1INT_VECTOR )
{
  halProcessKeyInterrupt ();
}

/**************************************************************************************************
 * @fn      halKeyPort2Isr
 *
 * @brief   Port2 ISR
 *
 * @param
 *
 * @return
 **************************************************************************************************/
HAL_ISR_FUNCTION( halKeyPort2Isr, P2INT_VECTOR )
{
  halProcessKeyInterrupt ();
}

/**************************************************************************************************
**************************************************************************************************/



