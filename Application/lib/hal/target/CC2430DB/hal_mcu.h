/**************************************************************************************************
    Filename:
    Revised:        $Date$
    Revision:       $Revision$

    Description:

    Describe the purpose and contents of the file.

    Copyright (c) 2006 by Texas Instruments, Inc.
    All Rights Reserved.  Permission to use, reproduce, copy, prepare
    derivative works, modify, distribute, perform, display or sell this
    software and/or its documentation for any purpose is prohibited
    without the express written consent of Texas Instruments, Inc.
**************************************************************************************************/

#ifndef HAL_MCU_H
#define HAL_MCU_H


/*
 *  Target : Chipcon CC2430  (core is 8051)
 *
 */


/* ------------------------------------------------------------------------------------------------
 *                                           Includes
 * ------------------------------------------------------------------------------------------------
 */
#include "hal_defs.h"


/* ------------------------------------------------------------------------------------------------
 *                                        Target Defines
 * ------------------------------------------------------------------------------------------------
 */
#define HAL_MCU_CC2430


/* ------------------------------------------------------------------------------------------------
 *                                     Compiler Abstraction
 * ------------------------------------------------------------------------------------------------
 */

/* ---------------------- IAR Compiler ---------------------- */
#ifdef __IAR_SYSTEMS_ICC__
#include <ioCC2430.h>
#define HAL_COMPILER_IAR
#define HAL_MCU_LITTLE_ENDIAN()   __LITTLE_ENDIAN__
#define _PRAGMA(x) _Pragma(#x)
#define HAL_ISR_FUNC_DECLARATION(f,v)   _PRAGMA(vector=v) __near_func __interrupt void f(void)
#define HAL_ISR_FUNC_PROTOTYPE(f,v)     _PRAGMA(vector=v) __near_func __interrupt void f(void)
#define HAL_ISR_FUNCTION(f,v)           HAL_ISR_FUNC_PROTOTYPE(f,v); HAL_ISR_FUNC_DECLARATION(f,v)

/* ---------------------- Keil Compiler ---------------------- */
#elif defined __KEIL__
#include <CC2430.h>
#define HAL_COMPILER_KEIL
#define HAL_MCU_LITTLE_ENDIAN()   0
#define HAL_ISR_FUNC_DECLARATION(f,v)   void f(void) interrupt v
#define HAL_ISR_FUNC_PROTOTYPE(f,v)     void f(void)
#define HAL_ISR_FUNCTION(f,v)           HAL_ISR_FUNC_PROTOTYPE(f,v); HAL_ISR_FUNC_DECLARATION(f,v)

/* ------------------ Unrecognized Compiler ------------------ */
#else
#error "ERROR: Unknown compiler."
#endif


/* ------------------------------------------------------------------------------------------------
 *                                        Interrupt Macros
 * ------------------------------------------------------------------------------------------------
 */
#define HAL_ENABLE_INTERRUPTS()         st( EA = 1; )
#define HAL_DISABLE_INTERRUPTS()        st( EA = 0; )
#define HAL_INTERRUPTS_ARE_ENABLED()    (EA)

typedef unsigned char halIntState_t;
#define HAL_ENTER_CRITICAL_SECTION(x)   st( x = EA;  HAL_DISABLE_INTERRUPTS(); )
#define HAL_EXIT_CRITICAL_SECTION(x)    st( EA = x; )
#define HAL_CRITICAL_STATEMENT(x)       st( halIntState_t s; HAL_ENTER_CRITICAL_SECTION(s); x; HAL_EXIT_CRITICAL_SECTION(s); )



/**************************************************************************************************
 */
#endif
