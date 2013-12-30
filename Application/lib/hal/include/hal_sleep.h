#ifndef HAL_SLEEP_H
#define HAL_SLEEP_H

/*********************************************************************
  Filename:       hal_sleep.h
  Revised:        $Date: 2006-12-12 16:15:19 -0800 (Tue, 12 Dec 2006) $
  Revision:       $Revision: 13089 $

  Description:

  This file contains the interface to the power management service.

  Notes:

  Copyright (c) 2006 by Texas Instruments, Inc.
  All Rights Reserved.  Permission to use, reproduce, copy, prepare
  derivative works, modify, distribute, perform, display or sell this
  software and/or its documentation for any purpose is prohibited
  without the express written consent of Texas Instruments, Inc.
*********************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * FUNCTIONS
 */

/*
 * Execute power management procedure
 */
extern void halSleep( uint16 osal_timer );

/*
 * Used in mac_mcu
 */
extern void halSleepWait(uint16 duration);

/*
 * Used in hal_drivers, AN044 - DELAY EXTERNAL INTERRUPTS
 */
extern void halRestoreSleepLevel( void );

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif
