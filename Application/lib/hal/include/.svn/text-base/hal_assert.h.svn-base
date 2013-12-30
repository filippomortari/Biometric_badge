/**************************************************************************************************
    Filename:
    Revised:        $Date: 2006-07-27 16:53:51 -0700 (Thu, 27 Jul 2006) $
    Revision:       $Revision: 11485 $

    Description:

    Describe the purpose and contents of the file.

  Copyright (c) 2006 by Texas Instruments, Inc.
  All Rights Reserved.  Permission to use, reproduce, copy, prepare
  derivative works, modify, distribute, perform, display or sell this
  software and/or its documentation for any purpose is prohibited
  without the express written consent of Texas Instruments, Inc.
**************************************************************************************************/

#ifndef HAL_ASSERT_H
#define HAL_ASSERT_H

/* ------------------------------------------------------------------------------------------------
 *                                           Macros
 * ------------------------------------------------------------------------------------------------
 */

/*
 *  This macro is for use during debugging.  The given expression must evaluate as "true"
 *  or else a fatal error occurs.  From here, the call stack feature of the debugger can
 *  pinpoint where the problem occurred.
 *
 *  This feature can be disabled for optimum performance and minimum code size (ideal for
 *  finalized, debugged production code).  To disable, define the preprocessor symbol HALNODEBUG
 *  at the project level.
 */
#ifdef HALNODEBUG
#define HAL_ASSERT(expr)
#else
#define HAL_ASSERT(expr)    { if (!(expr)) halAssertFatalError(); }
#endif


/*
 *  This macro compares the size of the first parameter to the integer value
 *  of the second parameter.  If they do not match, a compile time error for
 *  negative array size occurs (even gnu chokes on negative array size).
 *
 *  This compare is done by creating a typedef for an array.  No variables are
 *  created and no memory is consumed with this check.  The created type is
 *  used for checking only and is not for use by any other code.
 */
#define HAL_ASSERT_SIZE(x,y) typedef char x ## _assert_size_t[-1+10*(sizeof(x) == (y))]


/* ------------------------------------------------------------------------------------------------
 *                                          Prototypes
 * ------------------------------------------------------------------------------------------------
 */
void halAssertFatalError(void);


/**************************************************************************************************
 */
#endif
