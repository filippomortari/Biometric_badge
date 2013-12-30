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

#ifndef HAL_TYPES_H
#define HAL_TYPES_H

/* ChipCon CC2430 */

/* ------------------------------------------------------------------------------------------------
 *                                               Types
 * ------------------------------------------------------------------------------------------------
 */
typedef signed   char   int8;
typedef unsigned char   uint8;

typedef signed   short  int16;
typedef unsigned short  uint16;

typedef signed   long   int32;
typedef unsigned long   uint32;

typedef unsigned char   bool;

typedef uint8           halDataAlign_t;


/* ------------------------------------------------------------------------------------------------
 *                                       Memory Attributes
 * ------------------------------------------------------------------------------------------------
 */

/* ----------- IAR Compiler ----------- */
#ifdef __IAR_SYSTEMS_ICC__
#define  CODE   __code
#define  XDATA  __xdata

/* ----------- GNU Compiler ----------- */
#elif defined __KEIL__
#define  CODE   code
#define  XDATA  xdata

/* ----------- Unrecognized Compiler ----------- */
#else
#error "ERROR: Unknown compiler."
#endif


/* ------------------------------------------------------------------------------------------------
 *                                        Standard Defines
 * ------------------------------------------------------------------------------------------------
 */
#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef NULL
#define NULL 0
#endif


/**************************************************************************************************
 */
#endif
