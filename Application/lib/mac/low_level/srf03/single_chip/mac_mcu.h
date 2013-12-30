/**************************************************************************************************
    Filename:
    Revised:        $Date: 2006-11-14 11:20:09 -0800 (Tue, 14 Nov 2006) $
    Revision:       $Revision: 12700 $

    Description:

    Describe the purpose and contents of the file.

    Copyright (c) 2006 by Texas Instruments, Inc.
    All Rights Reserved.  Permission to use, reproduce, copy, prepare
    derivative works, modify, distribute, perform, display or sell this
    software and/or its documentation for any purpose is prohibited
    without the express written consent of Texas Instruments, Inc.
**************************************************************************************************/

#ifndef MAC_MCU_H
#define MAC_MCU_H


/* ------------------------------------------------------------------------------------------------
 *                                     Compiler Includes
 * ------------------------------------------------------------------------------------------------
 */
#include "hal_mcu.h"
#include "hal_types.h"
#include "hal_defs.h"
#include "hal_board.h"


/* ------------------------------------------------------------------------------------------------
 *                                    Target Specific Defines
 * ------------------------------------------------------------------------------------------------
 */
/* IP0, IP1 */
#define IP_0                  BV(0)
#define IP_1                  BV(0)
#define IP_2                  BV(2)
#define IP_RFERR_RF_DMA_BV    IP_0
#define IP_RXTX0_T2_BV        IP_2

/* T2CNF */
#define CMPIF           BV(7)
#define PERIF           BV(6)
#define OFCMPIF         BV(5)
#define SYNC            BV(1)
#define RUN             BV(0)
#define T2CNF_IF_BITS   (CMPIF | PERIF | OFCMPIF)

/* T2PEROF2 */
#define CMPIM           BV(7)
#define PERIM           BV(6)
#define OFCMPIM         BV(5)
#define PEROF2_BITS     (BV(3) | BV(2) | BV(1) | BV(0))

/* RFIF */
#define IRQ_TXDONE      BV(6)
#define IRQ_FIFOP       BV(5)
#define IRQ_SFD         BV(4)
#define IRQ_CSP_STOP    BV(1)
#define IRQ_CSP_INT     BV(0)

/* RFIM */
#define IM_TXDONE       BV(6)
#define IM_FIFOP        BV(5)
#define IM_SFD          BV(4)
#define IM_CSP_STOP     BV(1)
#define IM_CSP_INT      BV(0)

/* SLEEP */
#define XOSC_STB        BV(6)
#define OSC_PD          BV(2)

/* CLKCON */
#define OSC32K          BV(7)
#define OSC             BV(6)

/* IRQSRC */
#define TXACK           BV(0)


/* ------------------------------------------------------------------------------------------------
 *                                       Interrupt Macros
 * ------------------------------------------------------------------------------------------------
 */
////////////////////////////////////////////////////////////////////////////////////////////////////
//  FIX_ON_REV_C : workaround for chip bug #297, replace with macros below when fixed
#ifndef _NEXTREV
void macMcuOrRFIM(uint8 value);
void macMcuAndRFIM(uint8 value);
void macMcuWriteRFIF(uint8 value);

#define MAC_MCU_FIFOP_CLEAR_INTERRUPT()               macMcuWriteRFIF(~IRQ_FIFOP)
#define MAC_MCU_FIFOP_ENABLE_INTERRUPT()              macMcuOrRFIM(IM_FIFOP)
#define MAC_MCU_FIFOP_DISABLE_INTERRUPT()             macMcuAndRFIM(~IM_FIFOP)

#define MAC_MCU_SFD_CLEAR_INTERRUPT()                 macMcuWriteRFIF(~IRQ_SFD)
#define MAC_MCU_SFD_ENABLE_INTERRUPT()                macMcuOrRFIM(IM_SFD)
#define MAC_MCU_SFD_DISABLE_INTERRUPT()               macMcuAndRFIM(~IM_SFD)

#define MAC_MCU_CSP_STOP_CLEAR_INTERRUPT()            macMcuWriteRFIF(~IRQ_CSP_STOP)
#define MAC_MCU_CSP_STOP_ENABLE_INTERRUPT()           macMcuOrRFIM(IM_CSP_STOP)
#define MAC_MCU_CSP_STOP_DISABLE_INTERRUPT()          macMcuAndRFIM(~IM_CSP_STOP)
#else
//vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
//  keep this code, delete the rest
#define MAC_MCU_RFIF_CLEAR_FLAGS(x)   HAL_CRITICAL_STATEMENT( RFIF = ~(x); S1CON = 0x00; RFIF = 0xFF; )
#define MAC_MCU_RFIM_CLEAR_BITS(x)    st( RFIM &= ~(x); ) /* compiler must use atomic ANL instruction */
#define MAC_MCU_RFIM_SET_BITS(x)      st( RFIM |= x; )    /* compiler must use atomic ORL instruction */

#define MAC_MCU_FIFOP_CLEAR_INTERRUPT()               MAC_MCU_RFIF_CLEAR_FLAGS(IRQ_FIFOP)
#define MAC_MCU_FIFOP_ENABLE_INTERRUPT()              MAC_MCU_RFIM_SET_BITS(IM_FIFOP)
#define MAC_MCU_FIFOP_DISABLE_INTERRUPT()             MAC_MCU_RFIM_CLEAR_BITS(IM_FIFOP)

#define MAC_MCU_TXDONE_CLEAR_INTERRUPT()              MAC_MCU_RFIF_CLEAR_FLAGS(IRQ_TXDONE)
#define MAC_MCU_TXDONE_ENABLE_INTERRUPT()             MAC_MCU_RFIM_SET_BITS(IM_TXDONE)
#define MAC_MCU_TXDONE_DISABLE_INTERRUPT()            MAC_MCU_RFIM_CLEAR_BITS(IM_TXDONE)

#define MAC_MCU_CSP_STOP_CLEAR_INTERRUPT()            MAC_MCU_RFIF_CLEAR_FLAGS(IRQ_CSP_STOP)
#define MAC_MCU_CSP_STOP_ENABLE_INTERRUPT()           MAC_MCU_RFIM_SET_BITS(IM_CSP_STOP)
#define MAC_MCU_CSP_STOP_DISABLE_INTERRUPT()          MAC_MCU_RFIM_CLEAR_BITS(IM_CSP_STOP)
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
#endif
////////////////////////////////////////////////////////////////////////////////////////////////////


/* ------------------------------------------------------------------------------------------------
 *                                       Prototypes
 * ------------------------------------------------------------------------------------------------
 */
void macMcuInit(void);
void macMcuSetRandomSeed(uint16 rndSeed);
uint8 macMcuRandomByte(void);
uint8 macMcuTimerCount(void);
uint16 macMcuTimerCapture(void);
uint32 macMcuOverflowCount(void);
uint32 macMcuOverflowCapture(void);
void macMcuOverflowSetCount(uint32 count);
void macMcuOverflowSetCompare(uint32 count);
void macMcuOrT2PEROF2(uint8 value);
void macMcuAndT2PEROF2(uint8 value);
void macMcuEnergyDetectStart(void);
int8 macMcuEnergyDetectStop(void);
void macMcuEnergyDetectIsr(void);


/**************************************************************************************************
 */
#endif
