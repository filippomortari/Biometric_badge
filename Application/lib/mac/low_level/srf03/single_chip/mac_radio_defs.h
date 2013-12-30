/**************************************************************************************************
    Filename:
    Revised:        $Date: 2006-11-29 17:00:01 -0800 (Wed, 29 Nov 2006) $
    Revision:       $Revision: 12892 $

    Description:

    Describe the purpose and contents of the file.

    Copyright (c) 2006 by Texas Instruments, Inc.
    All Rights Reserved.  Permission to use, reproduce, copy, prepare
    derivative works, modify, distribute, perform, display or sell this
    software and/or its documentation for any purpose is prohibited
    without the express written consent of Texas Instruments, Inc.
**************************************************************************************************/

#ifndef MAC_RADIO_DEFS_H
#define MAC_RADIO_DEFS_H

/* ------------------------------------------------------------------------------------------------
 *                                             Includes
 * ------------------------------------------------------------------------------------------------
 */
#include "hal_defs.h"
#include "mac_spec.h"
#include "mac_mcu.h"
#include "mac_mem.h"
#include "mac_csp_tx.h"
#include "hal_board_cfg.h"
#include "mac_assert.h"


/* ------------------------------------------------------------------------------------------------
 *                                      Target Specific Defines
 * ------------------------------------------------------------------------------------------------
 */

/* immediate strobe commands */
#define ISTXCALN      0xE1
#define ISRXON        0xE2
#define ISTXON        0xE3
#define ISTXONCCA     0xE4
#define ISRFOFF       0xE5
#define ISFLUSHRX     0xE6
#define ISFLUSHTX     0xE7
#define ISACK         0xE8
#define ISACKPEND     0xE9

/* TXCTRLL */
#define PA_CURRENT(x)                 ((x) << 5)
#define PA_CURRENT_RESET_VALUE        PA_CURRENT(3)

/* FSCTRLL */
#define FREQ_2405MHZ                  0x65

/* RFSTATUS */
#define TX_ACTIVE                     BV(4)
#define FIFO                          BV(3)
#define FIFOP                         BV(2)
#define SFD                           BV(1)
#define CCA                           BV(0)

/* IEN2 */
#define RFIE                          BV(0)

/* MDMCTRL0L */
#define AUTOACK                       BV(4)

/* MDMCTRL0H */
#define PAN_COORDINATOR               BV(4)
#define ADDR_DECODE                   BV(3)

/* MDMCTRL1L */
#define MDMCTRL1L_RESET_VALUE         0x00
#define RX_MODE(x)                    ((x) << 0)
#define RX_MODE_INFINITE_RECEPTION    RX_MODE(2)
#define RX_MODE_NORMAL_OPERATION      RX_MODE(0)

/* FSMSTATE */
#define FSM_FFCTRL_STATE_RX_INF       31      /* infinite reception state - not documented in data sheet */

/* RFPWR */
#define ADI_RADIO_PD                  BV(4)
#define RREG_RADIO_PD                 BV(3)

/* ADCCON1 */
#define RCTRL1                        BV(3)
#define RCTRL0                        BV(2)
#define RCTRL_BITS                    (RCTRL1 | RCTRL0)
#define RCTRL_CLOCK_LFSR              RCTRL0

/* FSMTC1 */
#define ABORTRX_ON_SRXON              BV(5)
#define PENDING_OR                    BV(1)

/* CSPCTRL */
#define CPU_CTRL                      BV(0)
#define CPU_CTRL_ON                   CPU_CTRL
#define CPU_CTRL_OFF                  (!(CPU_CTRL))


/* ------------------------------------------------------------------------------------------------
 *                                    Unique Radio Define
 * ------------------------------------------------------------------------------------------------
 */
#define MAC_RADIO_CC2430
///////////////////////////////////////////////////////////////////////////////////////
//  FIX_ON_REV_C : workaround to reduce effects of chip bug #353.  Remove with Rev C.
#ifndef _NEXTREV
#define MAC_RADIO_RXBUFF_CHIP_BUG
#endif
///////////////////////////////////////////////////////////////////////////////////////


/* ------------------------------------------------------------------------------------------------
 *                                    Common Radio Defines
 * ------------------------------------------------------------------------------------------------
 */
#define MAC_RADIO_CHANNEL_DEFAULT               11
#define MAC_RADIO_TX_POWER_DEFAULT              0x1F
#define MAC_RADIO_TX_POWER_MAX_MINUS_DBM        25

/*
 *  Precise values for small delay on both receive and transmit side.
 *  These values not originally available from datatsheet.
 */
#define MAC_RADIO_RX_TX_PROP_DELAY_MIN_USEC     3.076  /* usec */
#define MAC_RADIO_RX_TX_PROP_DELAY_MAX_USEC     3.284  /* usec */


/* ------------------------------------------------------------------------------------------------
 *                                      Common Radio Macros
 * ------------------------------------------------------------------------------------------------
 */
#define MAC_RADIO_INIT()                              macMcuInit()

#define MAC_RADIO_ENABLE_VREG_SEQUENCE()              st( RFPWR &= ~RREG_RADIO_PD; while((RFPWR & ADI_RADIO_PD)); )
#define MAC_RADIO_TURN_OFF_VREG()                     st( RFPWR |=  RREG_RADIO_PD; )
#define MAC_RADIO_VREG_ON_RADIO_INIT()                /* no action required */
#define MAC_RADIO_RESTORE_FROM_VREG_OFF()             /* no action required */

#define MAC_RADIO_ENABLE_OSC_SEQUENCE()               MAC_ASSERT(SLEEP & XOSC_STB)/* oscillator should already be stable, it's mcu oscillator too */
#define MAC_RADIO_TURN_OFF_OSC()                      /* don't do anything, oscillator is also mcu oscillator */
#define MAC_RADIO_RESTORE_FROM_OSC_OFF()              /* no action required */

#define MAC_RADIO_CCA_IS_VALID()                      (RFSTATUS & CCA)

#define MAC_RADIO_RX_FIFO_OVERFLOW()                  ((RFSTATUS & FIFOP) && !(RFSTATUS & FIFO))

#define MAC_RADIO_GET_RSSI()                          (RSSIL)

#define RSSI_OFFSET_VALUE                             (-38)
#define ED_2_LQI(ed)                                  (((ed) > 63 ? 255 : ((ed) << 2)))
#define RSSI_2_ED(rssi)                               ((rssi) < RSSI_OFFSET_VALUE ? 0 : ((rssi) - (RSSI_OFFSET_VALUE)))
#define MAC_RADIO_CONVERT_RSSI_TO_LQI(x)              ED_2_LQI(RSSI_2_ED((int8)x))

#define MAC_RADIO_SET_RX_THRESHOLD(x)                 st( IOCFG0 = ((x)-1); )
#define MAC_RADIO_SET_DYNAMIC_RX_THRESHOLD(x)         MAC_RADIO_SET_RX_THRESHOLD(x) /* aliased because FIFO writes are fast */
#define MAC_RADIO_RX_AT_THRESHOLD()                   (RFSTATUS & FIFOP)
#define MAC_RADIO_ENABLE_RX_THRESHOLD_INTERRUPT()     MAC_MCU_FIFOP_ENABLE_INTERRUPT()
#define MAC_RADIO_DISABLE_RX_THRESHOLD_INTERRUPT()    MAC_MCU_FIFOP_DISABLE_INTERRUPT()
#define MAC_RADIO_CLEAR_RX_THRESHOLD_INTERRUPT_FLAG() st( RFIF = ~IRQ_FIFOP; )

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  FIX_ON_REV_C : work around for chip bug #267, replace these lines with following.
#ifndef _NEXTREV
#define MAC_RADIO_TX_ACK()                            st( FSMTC1 &= ~PENDING_OR; )
#define MAC_RADIO_TX_ACK_PEND()                       st( FSMTC1 |=  PENDING_OR; )
#else
//vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
//  keep this code, delete the rest
#define MAC_RADIO_TX_ACK()                            st( RFST = ISACK; )
#define MAC_RADIO_TX_ACK_PEND()                       st( RFST = ISACKPEND; )
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
#endif
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define MAC_RADIO_TX()                                st( RFST = ISTXON;    )
#define MAC_RADIO_TX_AFTER_CCA()                      st( RFST = ISTXONCCA; )
#define MAC_RADIO_RX_ON()                             st( RFST = ISRXON;    )
#define MAC_RADIO_RXTX_OFF()                          st( RFST = ISRFOFF;   )
#define MAC_RADIO_FLUSH_RX_FIFO()                     st( RFST = ISFLUSHRX; RFST = ISFLUSHRX; )
#define MAC_RADIO_FLUSH_TX_FIFO()                     st( RFST = ISFLUSHTX; )
#define MAC_RADIO_TX_IS_ACTIVE()                      (RFSTATUS & TX_ACTIVE)

#define MAC_RADIO_READ_RX_FIFO(pData,len)             macMemReadRxFifo((pData),(uint8)(len))
#define MAC_RADIO_WRITE_TX_FIFO(pData,len)            macMemWriteTxFifo((pData),(uint8)(len))

#define MAC_RADIO_SET_PAN_COORDINATOR(b)              st( MDMCTRL0H = (MDMCTRL0H & ~PAN_COORDINATOR) | (PAN_COORDINATOR * (b!=0)); )
#define MAC_RADIO_SET_CHANNEL(x)                      st( FSCTRLL = FREQ_2405MHZ + 5 * ((x) - 11); )
#define MAC_RADIO_SET_TX_POWER(x)                     st( TXCTRLL = PA_CURRENT_RESET_VALUE | (x); )

#define MAC_RADIO_READ_PAN_ID()                       (PANIDL + (macMemReadRamByte(&PANIDH) << 8))
#define MAC_RADIO_SET_PAN_ID(x)                       st( PANIDL = (x) & 0xFF; PANIDH = (x) >> 8; )
#define MAC_RADIO_SET_SHORT_ADDR(x)                   st( SHORTADDRL = (x) & 0xFF; SHORTADDRH = (x) >> 8; )
#define MAC_RADIO_SET_IEEE_ADDR(p)                    macMemWriteRam((macRam_t *) &IEEE_ADDR0, p, 8)

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  FIX_ON_REV_C : The new chip should interrupt on ACK transmit complete too (currently only interrupts for regular TX complete)
//  Replace these macros with the macros below when available.
#ifndef _NEXTREV
#define MAC_RADIO_REQUEST_ACK_TX_DONE_CALLBACK()      st( MAC_MCU_SFD_CLEAR_INTERRUPT(); MAC_MCU_SFD_ENABLE_INTERRUPT(); )
#define MAC_RADIO_CANCEL_ACK_TX_DONE_CALLBACK()       MAC_MCU_SFD_DISABLE_INTERRUPT()
#else
//vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
//  keep this code, delete the rest
#define MAC_RADIO_REQUEST_ACK_TX_DONE_CALLBACK()      st( MAC_MCU_TXDONE_CLEAR_INTERRUPT(); MAC_MCU_TXDONE_ENABLE_INTERRUPT(); )
#define MAC_RADIO_CANCEL_ACK_TX_DONE_CALLBACK()       MAC_MCU_TXDONE_DISABLE_INTERRUPT()
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
#endif
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define MAC_RADIO_CONFIG_RX_INFINITE_RECEPTION()      st( MDMCTRL1L = MDMCTRL1L_RESET_VALUE | RX_MODE_INFINITE_RECEPTION; )
#define MAC_RADIO_CONFIG_RX_NORMAL_OPERATION()        st( MDMCTRL1L = MDMCTRL1L_RESET_VALUE | RX_MODE_NORMAL_OPERATION; )
#define MAC_RADIO_RANDOM_FSMSTATE_RX_INF()            (FSMSTATE == FSM_FFCTRL_STATE_RX_INF)
#define MAC_RADIO_RANDOM_ADC_BIT()                    (ADCTSTH & 0x01)
#define MAC_RADIO_RANDOM_SET_SEED(x)                  macMcuSetRandomSeed(x)
#define MAC_RADIO_RANDOM_BYTE()                       macMcuRandomByte()

#define MAC_RADIO_TX_RESET()                          macCspTxReset()
#define MAC_RADIO_TX_PREP_CSMA_UNSLOTTED()            macCspTxPrepCsmaUnslotted()
#define MAC_RADIO_TX_PREP_CSMA_SLOTTED()              macCspTxPrepCsmaSlotted()
#define MAC_RADIO_TX_PREP_SLOTTED()                   macCspTxPrepSlotted()
#define MAC_RADIO_TX_GO_CSMA()                        macCspTxGoCsma()
#define MAC_RADIO_TX_GO_SLOTTED()                     macCspTxGoSlotted()
#define MAC_RADIO_TX_REQUEST_ACK_TIMEOUT_CALLBACK()   macCspTxRequestAckTimeoutCallback()
#define MAC_RADIO_TX_CANCEL_ACK_TIMEOUT_CALLBACK()    macCspTxCancelAckTimeoutCallback()

#define MAC_RADIO_TIMER_TICKS_PER_USEC()              HAL_CPU_CLOCK_MHZ /* never fractional */
#define MAC_RADIO_TIMER_TICKS_PER_BACKOFF()           (HAL_CPU_CLOCK_MHZ * MAC_SPEC_USECS_PER_BACKOFF)
#define MAC_RADIO_TIMER_TICKS_PER_SYMBOL()            (HAL_CPU_CLOCK_MHZ * MAC_SPEC_USECS_PER_SYMBOL)

#define MAC_RADIO_TIMER_CAPTURE()                     macMcuTimerCapture()
#define MAC_RADIO_TIMER_FORCE_DELAY(x)                st( T2TLD = (x) & 0xFF;  T2THD = (x) >> 8; )  /* timer must be running */

#define MAC_RADIO_TIMER_SLEEP()                       st( T2CNF &= ~RUN; while (T2CNF & RUN); )
#define MAC_RADIO_TIMER_WAKE_UP()                     st( T2CNF |=  RUN; while (!(T2CNF & RUN)); )

#define MAC_RADIO_BACKOFF_COUNT()                     macMcuOverflowCount()
#define MAC_RADIO_BACKOFF_CAPTURE()                   macMcuOverflowCapture()
#define MAC_RADIO_BACKOFF_SET_COUNT(x)                macMcuOverflowSetCount(x)
#define MAC_RADIO_BACKOFF_SET_COMPARE(x)              macMcuOverflowSetCompare(x)

#define MAC_RADIO_BACKOFF_COMPARE_CLEAR_INTERRUPT()   st( T2CNF = RUN | SYNC | (~OFCMPIF & T2CNF_IF_BITS); )
#define MAC_RADIO_BACKOFF_COMPARE_ENABLE_INTERRUPT()  macMcuOrT2PEROF2(OFCMPIM)
#define MAC_RADIO_BACKOFF_COMPARE_DISABLE_INTERRUPT() macMcuAndT2PEROF2(~OFCMPIM)

#define MAC_RADIO_ENERGY_DETECT_START()               macMcuEnergyDetectStart()
#define MAC_RADIO_ENERGY_DETECT_STOP()                macMcuEnergyDetectStop()

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  FIX_ON_REV_C : work around for chip bug #267, autoack is used as work around, need to disble for promiscuous mode
//  easiest to do here as part of address recognition since bug is exclusive to 2430.  Replace these lines with following
//  when REV_C arrives.
#ifndef _NEXTREV
#define MAC_RADIO_TURN_ON_RX_FRAME_FILTERING()        st( MDMCTRL0H |=  ADDR_DECODE; MDMCTRL0L |=  AUTOACK; )
#define MAC_RADIO_TURN_OFF_RX_FRAME_FILTERING()       st( MDMCTRL0H &= ~ADDR_DECODE; MDMCTRL0L &= ~AUTOACK; )
#else
//vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
//  keep this code, delete the rest
#define MAC_RADIO_TURN_ON_RX_FRAME_FILTERING()        st( MDMCTRL0H |=  ADDR_DECODE; )
#define MAC_RADIO_TURN_OFF_RX_FRAME_FILTERING()       st( MDMCTRL0H &= ~ADDR_DECODE; )
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
#endif
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/* ------------------------------------------------------------------------------------------------
 *                                    Common Radio Externs
 * ------------------------------------------------------------------------------------------------
 */
extern const uint8 CODE macRadioDefsTxPowerTable[];


/**************************************************************************************************
 */
#endif
