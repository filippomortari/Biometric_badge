/**************************************************************************************************
    Filename:
    Revised:        $Date: 2006-10-31 14:16:24 -0800 (Tue, 31 Oct 2006) $
    Revision:       $Revision: 12457 $

    Description:

    Describe the purpose and contents of the file.

    Copyright (c) 2006 by Texas Instruments, Inc.
    All Rights Reserved.  Permission to use, reproduce, copy, prepare
    derivative works, modify, distribute, perform, display or sell this
    software and/or its documentation for any purpose is prohibited
    without the express written consent of Texas Instruments, Inc.
**************************************************************************************************/

#ifndef MAC_ASSERT_H
#define MAC_ASSERT_H


/* ------------------------------------------------------------------------------------------------
 *                                          Includes
 * ------------------------------------------------------------------------------------------------
 */
#include "hal_assert.h"


/* ------------------------------------------------------------------------------------------------
 *                                           Macros
 * ------------------------------------------------------------------------------------------------
 */

/*
 *  The MAC_ASSERT macro is for use during debugging.  The given expression must
 *  evaluate as "true" or else fatal error occurs.  At that point, the call stack
 *  feature of the debugger can pinpoint where the problem occurred.
 *
 *  To disable this feature and save code size, the project should define MACNODEBUG.
 */
#ifdef MACNODEBUG
#define MAC_ASSERT(expr)
#define MAC_DEBUG(statement)
#else
#define MAC_ASSERT(expr)        HAL_ASSERT(expr)
#define MAC_DEBUG(statement)    st( statement )
#endif


/**************************************************************************************************
 */
#endif
