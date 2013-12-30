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
 *                                           Includes
 * ------------------------------------------------------------------------------------------------
 */

/* hal */
#include "hal_types.h"
#include "hal_mcu.h"

/* high-level */
#include "mac_spec.h"
#include "mac_pib.h"

/* exported low-level */
#include "mac_low_level.h"

/* low-level specific */
#include "mac_csp_tx.h"
#include "mac_tx.h"
#include "mac_rx.h"
#include "mac_rx_onoff.h"

/* target specific */
#include "mac_radio_defs.h"

/* debug */
#include "mac_assert.h"


/* ------------------------------------------------------------------------------------------------
 *                                   CSP Defines / Macros
 * ------------------------------------------------------------------------------------------------
 */

/* very specific values, modify with extreme care */
#define CSPZ_CODE_SUCCESS           0
#define CSPZ_CODE_CHANNEL_BUSY      1
#define CSPZ_CODE_TX_RX_COLLISION   2
#define CSPZ_CODE_TX_ACK_TIME_OUT   0x55  /* arbitrary value that does not collide with MAC_TXDONE defines */

/* immediate strobe commands */
#define ISSTART     0xFE
#define ISSTOP      0xFF

/* strobe processor instructions */
#define SKIP(s,c)   (0x00 | (((s) & 0x07) << 4) | ((c) & 0x0F))   /* skip 's' instructions if 'c' is true  */
#define WHILE(c)    SKIP(0,c)              /* pend while 'c' is true (derived instruction)        */
#define WAITW(w)    (0x80 | ((w) & 0x1F))  /* wait for 'w' number of MAC timer overflows          */
#define WEVENT      (0xB8)                 /* wait for MAC timer compare                          */
#define WAITX       (0xBB)                 /* wait for CPSX number of MAC timer overflows         */
#define LABEL       (0xBA)                 /* set next instruction as start of loop               */
#define RPT(c)      (0xA0 | ((c) & 0x0F))  /* if condition is true jump to last label             */
#define INT         (0xB9)                 /* assert IRQ_CSP_INT interrupt                        */
#define INCY        (0xBD)                 /* increment CSPY                                      */
#define INCMAXY(m)  (0xB0 | ((m) & 0x07))  /* increment CSPY but not above maximum value of 'm'   */
#define DECY        (0xBE)                 /* decrement CSPY                                      */
#define DECZ        (0xBF)                 /* decrement CSPZ                                      */
#define RANDXY      (0xBC)                 /* load the lower CSPY bits of CSPX with random value  */

/* strobe processor command instructions */
#define SSTOP       (0xDF)    /* stop program execution                                      */
#define SNOP        (0xC0)    /* no operation                                                */
#define STXCALN     (0xC1)    /* enable and calibrate frequency synthesizer for TX           */
#define SRXON       (0xC2)    /* turn on receiver                                            */
#define STXON       (0xC3)    /* transmit after calibration                                  */
#define STXONCCA    (0xC4)    /* transmit after calibration if CCA indicates clear channel   */
#define SRFOFF      (0xC5)    /* turn off RX/TX                                              */
#define SFLUSHRX    (0xC6)    /* flush receive FIFO                                          */
#define SFLUSHTX    (0xC7)    /* flush transmit FIFO                                         */
#define SACK        (0xC8)    /* send ACK frame                                              */
#define SACKPEND    (0xC9)    /* send ACK frame with pending bit set                         */

/* conditions for use with instructions SKIP and RPT */
#define C_CCA_IS_VALID        0x00
#define C_SFD_IS_ACTIVE       0x01
#define C_CPU_CTRL_IS_ON      0x02
#define C_END_INSTR_MEM       0x03
#define C_CSPX_IS_ZERO        0x04
#define C_CSPY_IS_ZERO        0x05
#define C_CSPZ_IS_ZERO        0x06

/* negated conditions for use with instructions SKIP and RPT */
#define C_NEGATE(c)   ((c) | 0x08)
#define C_CCA_IS_INVALID      C_NEGATE(C_CCA_IS_VALID)
#define C_SFD_IS_INACTIVE     C_NEGATE(C_SFD_IS_ACTIVE)
#define C_CPU_CTRL_IS_OFF     C_NEGATE(C_CPU_CTRL_IS_ON)
#define C_NOT_END_INSTR_MEM   C_NEGATE(C_END_INSTR_MEM)
#define C_CSPX_IS_NON_ZERO    C_NEGATE(C_CSPX_IS_ZERO)
#define C_CSPY_IS_NON_ZERO    C_NEGATE(C_CSPY_IS_ZERO)
#define C_CSPZ_IS_NON_ZERO    C_NEGATE(C_CSPZ_IS_ZERO)

///////////////////////////////////////////////////////////////////////////////////////
//  FIX_ON_REV_C : part of a workaround to help ameliorate chip bug #273.
//  This bug could actually be the source of extant every-once-in-a-while problems.
//  When Rev C is available, delete these defines and all references to it.
//  Compile errors should flag all related fixes.
#define __SNOP_SKIP__     1
#define __SNOP__          SNOP
///////////////////////////////////////////////////////////////////////////////////////


/* ------------------------------------------------------------------------------------------------
 *                                          Macros
 * ------------------------------------------------------------------------------------------------
 */
#define CSP_STOP_AND_CLEAR_PROGRAM()          st( RFST = ISSTOP;  )
#define CSP_START_PROGRAM()                   st( RFST = ISSTART; )
////////////////////////////////////////////////////////////////////////////////////////////////////
//  FIX_ON_REV_C : workaround for chip bug #574, delete this whole mess when Rev C arrives
//  Delete when Rev arrives. Compile errors will flag all instances of macro call.  Delete those too.
#ifndef _NEXTREV
#define __FIX_T2CMP_BUG__()   st( if (T2CMP == 0) { T2CMP = 1;} )
#else
#define __FIX_T2CMP_BUG__()
#endif
////////////////////////////////////////////////////////////////////////////////////////////////////


/*
 *  These macros improve readability of using T2CMP in conjunction with WEVENT.
 *
 *  The timer2 compare, T2CMP, only compares one byte of the 16-bit timer register.
 *  It is configurable and has been set to compare against the upper byte of the timer value.
 *  The CSP instruction WEVENT waits for the timer value to be greater than or equal
 *  the value of T2CMP.
 *
 *  Reading the timer value is done by reading the low byte first.  This latches the
 *  high byte.  A trick with the ternary operator is used by a macro below to force a
 *  read of the low byte when returning the value of the high byte.
 *
 *  CSP_WEVENT_SET_TRIGGER_NOW()      - sets the WEVENT trigger point at the current timer count
 *  CSP_WEVENT_SET_TRIGGER_SYMBOLS(x) - sets the WEVENT trigger point in symbols
 *  CSP_WEVENT_READ_COUNT_SYMBOLS()   - reads the current timer count in symbols
 */
#define T2THD_TICKS_PER_SYMBOL                (MAC_RADIO_TIMER_TICKS_PER_SYMBOL() >> 8)

#define CSP_WEVENT_SET_TRIGGER_NOW()          st( uint8 temp=T2TLD;  T2CMP = T2THD;  __FIX_T2CMP_BUG__(); )
#define CSP_WEVENT_SET_TRIGGER_SYMBOLS(x)     st( MAC_ASSERT(x <= MAC_A_UNIT_BACKOFF_PERIOD); \
                                                  T2CMP = (x) * T2THD_TICKS_PER_SYMBOL; \
                                                  __FIX_T2CMP_BUG__(); )
#define CSP_WEVENT_READ_COUNT_SYMBOLS()       (((T2TLD) ? T2THD : T2THD) / T2THD_TICKS_PER_SYMBOL)


/* ------------------------------------------------------------------------------------------------
 *                                       Local Prototypes
 * ------------------------------------------------------------------------------------------------
 */
static void cspTxClearProgramMemory(void);


/**************************************************************************************************
 * @fn          macCspTxReset
 *
 * @brief       Reset the CSP.  Immediately halts any running program.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
void macCspTxReset(void)
{
  cspTxClearProgramMemory();
}


/*=================================================================================================
 * @fn          cspTxClearProgramMemory
 *
 * @brief       Clears CSP program memory.  After memory is cleared a new program can be loaded.
 *
 * @param       none
 *
 * @return      none
 *=================================================================================================
 */
static void cspTxClearProgramMemory(void)
{
  MAC_MCU_CSP_STOP_DISABLE_INTERRUPT();
  CSP_STOP_AND_CLEAR_PROGRAM();
  MAC_MCU_CSP_STOP_CLEAR_INTERRUPT();
  MAC_MCU_CSP_STOP_ENABLE_INTERRUPT();
}


/**************************************************************************************************
 * @fn          macCspTxPrepCsmaUnslotted
 *
 * @brief       Prepare CSP for "Unslotted CSMA" transmit.  Load CSP program and set CSP parameters.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
void macCspTxPrepCsmaUnslotted(void)
{
  CSPZ = CSPZ_CODE_CHANNEL_BUSY;
  CSP_WEVENT_SET_TRIGGER_NOW();

  /* load CSP program : Unslotted CSMA Transmit */
  cspTxClearProgramMemory();
  RFST = WAITX;
  RFST = WEVENT;
  RFST = SKIP(6+__SNOP_SKIP__, C_CPU_CTRL_IS_ON);
  RFST = SKIP(1+__SNOP_SKIP__, C_SFD_IS_ACTIVE);
  RFST = SRXON;
  RFST = __SNOP__;
  RFST = WAITW(1);
  RFST = WEVENT;
  RFST = SKIP(1+__SNOP_SKIP__, C_CCA_IS_VALID);
  RFST = SSTOP;
  RFST = __SNOP__;
  RFST = STXON;
  RFST = DECZ;

  RFST = WHILE(C_SFD_IS_INACTIVE);
  RFST = INT; /* causes ISR to run macTxTimestampCallback() */
  RFST = WHILE(C_SFD_IS_ACTIVE);
}


/**************************************************************************************************
 * @fn          macCspTxPrepCsmaSlotted
 *
 * @brief       Prepare CSP for "Slotted CSMA" transmit.  Load CSP program and set CSP parameters.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
void macCspTxPrepCsmaSlotted(void)
{
  CSPZ = CSPZ_CODE_CHANNEL_BUSY;

  /* load CSP program : Slotted CSMA Transmit */
  cspTxClearProgramMemory();
  RFST = WAITX;
  RFST = SKIP(3+__SNOP_SKIP__, C_CPU_CTRL_IS_ON);
  RFST = SKIP(1+__SNOP_SKIP__, C_SFD_IS_ACTIVE);
  RFST = SRXON;
  RFST = __SNOP__;
  RFST = WAITW(1);
  RFST = SKIP(1+__SNOP_SKIP__, C_CCA_IS_VALID);
  RFST = SSTOP;
  RFST = __SNOP__;
  RFST = WAITW(1);
  RFST = SKIP(1+__SNOP_SKIP__, C_CCA_IS_VALID);
  RFST = SSTOP;
  RFST = __SNOP__;
  RFST = STXON;
  RFST = SKIP(2+__SNOP_SKIP__, C_SFD_IS_INACTIVE);
  RFST = INT; /* rx-tx collision */
  RFST = WHILE(C_SFD_IS_ACTIVE);
  RFST = __SNOP__;
  RFST = DECZ;

  RFST = WHILE(C_SFD_IS_INACTIVE);
  RFST = INT; /* causes ISR to run macTxTimestampCallback() */
  RFST = WHILE(C_SFD_IS_ACTIVE);
}


/**************************************************************************************************
 * @fn          macCspTxPrepSlotted
 *
 * @brief       Prepare CSP for "Slotted" (non-CSMA) transmit.
 *              Load CSP program and set CSP parameters.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
void macCspTxPrepSlotted(void)
{
  /* configure parameters for slotted transmit */
  CSPX = macTxSlottedDelay - 1;
  CSPZ = CSPZ_CODE_TX_RX_COLLISION;

  /* load CSP program : Slotted Transmit */
  cspTxClearProgramMemory();
  RFST = WAITX;
  RFST = STXON;
  RFST = SKIP(2+__SNOP_SKIP__, C_SFD_IS_INACTIVE);
  RFST = INT; /* rx-tx collision */
  RFST = WHILE(C_SFD_IS_ACTIVE);
  RFST = __SNOP__;
  RFST = DECZ;
  RFST = DECZ;

  RFST = WHILE(C_SFD_IS_INACTIVE);
  RFST = INT; /* causes ISR to run macTxTimestampCallback() */
  RFST = WHILE(C_SFD_IS_ACTIVE);
}


/**************************************************************************************************
 * @fn          macCspTxGoCsma
 *
 * @brief       Run previously loaded CSP program for CSMA transmit.  Handles either
 *              slotted or unslotted CSMA transmits.  When CSP program has finished,
 *              an interrupt occurs and macCspTxStopIsr() is called.  This ISR will in
 *              turn call macTxDoneCallback().
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
void macCspTxGoCsma(void)
{
  CSPX = macTxCsmaBackoffDelay;

  CSPCTRL = CPU_CTRL_OFF;
  /* there is a built-in one backoff delay to turn on RX, subtract that out */
  if (CSPX != 0)
  {
    CSPX--;
  }

  CSP_START_PROGRAM();
}


/**************************************************************************************************
 * @fn          macCspTxGoSlotted
 *
 * @brief       Run previously loaded CSP program for non-CSMA slotted transmit.   When CSP
 *              program has finished, an interrupt occurs and macCspTxStopIsr() is called.
 *              This ISR will in turn call macTxDoneCallback().
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
void macCspTxGoSlotted(void)
{
  CSP_START_PROGRAM();
}


/**************************************************************************************************
 * @fn          macCspTxRequestAckTimeoutCallback
 *
 * @brief       Requests a callback after the ACK timeout period has expired.  At that point,
 *              the function macTxAckTimeoutCallback() is called via an interrupt.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
void macCspTxRequestAckTimeoutCallback(void)
{
  uint8 startSymbol;
  uint8 symbols;
  uint8 rollovers;

  /* record current symbol count */
  startSymbol = CSP_WEVENT_READ_COUNT_SYMBOLS();

  /* set symbol timeout from PIB */
  symbols = macPib.ackWaitDuration;

  /* make sure delay value is not too small for logic to handle */
  MAC_ASSERT(symbols > MAC_A_UNIT_BACKOFF_PERIOD);  /* symbols timeout period must be great than a backoff */

  /* subtract out symbols left in current backoff period */
  symbols = symbols - (MAC_A_UNIT_BACKOFF_PERIOD - startSymbol);

  /* calculate rollovers needed for remaining symbols */
  rollovers = symbols / MAC_A_UNIT_BACKOFF_PERIOD;

  /* calculate symbols that still need counted after last rollover */
  symbols = symbols - (rollovers * MAC_A_UNIT_BACKOFF_PERIOD);

  /* add one to rollovers to account for symbols remaining in the current backoff period */
  rollovers++;

  /* set CSP parameters */
  CSPZ = CSPZ_CODE_TX_ACK_TIME_OUT;
  CSPX = rollovers;
  CSP_WEVENT_SET_TRIGGER_SYMBOLS(symbols);

  /* load CSP program */
  cspTxClearProgramMemory();
  RFST = WAITX;
  RFST = WEVENT;

  /* run CSP program */
  CSP_START_PROGRAM();

  /*
   *  For bullet proof operation, must account for the boundary condition
   *  where a rollover occurs after count was read but before CSP program
   *  was started.
   *
   *  If current symbol count is less that the symbol count recorded at the
   *  start of this function, a rollover has occurred.
   */
  if (CSP_WEVENT_READ_COUNT_SYMBOLS() < startSymbol)
  {
    /* a rollover has occurred, make sure it was accounted for */
    if (CSPX == rollovers)
    {
      /* rollover event missed, manually decrement CSPX to adjust */
      CSPX--;
    }
  }
}


/**************************************************************************************************
 * @fn          macCspTxCancelAckTimeoutCallback
 *
 * @brief       Cancels previous request for ACK timeout callback.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
void macCspTxCancelAckTimeoutCallback(void)
{
  /* just stop program to cancel */
  cspTxClearProgramMemory();
}


/**************************************************************************************************
 * @fn          macCspTxIntIsr
 *
 * @brief       Interrupt service routine for handling INT type interrupts from CSP.
 *              This interrupt happens when the CSP instruction INT is executed.
 *              Two cases are handled.  This interrupt usually occurs when a transmit is
 *              successful and the timestamp can be stored.  This interrupt can also occur if
 *              an rx-tx collision occurred.  The value of CSPZ indicates what needs to be
 *              handled.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
void macCspTxIntIsr(void)
{
///////////////////////////////////////////////////////////////////////////////////////
//  FIX_ON_REV_C : work workaround for chip bug #273.  The instruction DECZ might
//  be incorrectly executed twice, resulting an illegal value for CSPZ.
//  Delete when Rev C arrives.
///////////////////////////////////////////////////////////////////////////////////////
#ifndef _NEXTREV
  if (CSPZ >= 0xF8) { CSPZ = 0; }
#endif
///////////////////////////////////////////////////////////////////////////////////////

  if (CSPZ == MAC_TXDONE_SUCCESS)
  {
    /*
     *  Record the timestamp.  This means transmit was successful.  Use this as
     *  an opportunity to indicate the receiver was strobed during CSP execution
     *  (the regular function macRxOn interface was not called).
     */
    MAC_RX_WAS_FORCED_ON();

    /* execute callback function that records transmit timestamp */
    macTxTimestampCallback();
  }
  else
  {
    macTxCollisionWithRxCallback();
  }
}


/**************************************************************************************************
 * @fn          macCspTxStopIsr
 *
 * @brief       Interrupt service rountine for handling STOP type interrupts from CSP.
 *              This interrupt occurs when the CSP program stops by 1) reaching the end of the
 *              program, 2) executing SSTOP within the program, 3) executing immediate
 *              instruction ISSTOP.
 *
 *              The value of CSPZ indicates if interrupt is being used for ACK timeout or
 *              is the end of a transmit.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
void macCspTxStopIsr(void)
{
///////////////////////////////////////////////////////////////////////////////////////
//  FIX_ON_REV_C : work workaround for chip bug #273.  The instruction DECZ might
//  be incorrectly executed twice, resulting an illegal value for CSPZ.
//  Delete when Rev C arrives.
///////////////////////////////////////////////////////////////////////////////////////
#ifndef _NEXTREV
  if (CSPZ >= 0xF8) { CSPZ = 0; }
#endif
///////////////////////////////////////////////////////////////////////////////////////

  /* see if CSPZ indicates an ACK timeout interrupt */
  if (CSPZ == CSPZ_CODE_TX_ACK_TIME_OUT)
  {
    macTxAckTimeoutCallback();
  }
  else
  {
    MAC_ASSERT((CSPZ == MAC_TXDONE_SUCCESS) ||
               (CSPZ == MAC_TXDONE_CHANNEL_BUSY));     /* unexpected CSPZ value */

    /* not an ACK timeout interrupt, it is a transmit complete interrupt */
    macTxDoneCallback(CSPZ);
  }
}



/**************************************************************************************************
 *                                  Compile Time Integrity Checks
 **************************************************************************************************
 */
#if (MAC_TX_TYPE_SLOTTED_CSMA != 0)
#error "WARNING!  This define value changed.  It was selected for optimum performance."
#endif

#if ((CSPZ_CODE_SUCCESS != MAC_TXDONE_SUCCESS) || (CSPZ_CODE_CHANNEL_BUSY != MAC_TXDONE_CHANNEL_BUSY))
#error "ERROR! Mismatch between internal CSPZ codes and external return values."
#endif

#if (T2THD_TICKS_PER_SYMBOL == 0)
#error "ERROR!  Timer compare will not work on high byte.  Clock speed is probably too slow."
#endif


/**************************************************************************************************
*/
