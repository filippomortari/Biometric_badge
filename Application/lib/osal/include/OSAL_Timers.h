#ifndef OSAL_TIMERS_H
#define OSAL_TIMERS_H

/*********************************************************************
    Filename:       OSAL_Timers.h
    Revised:        $Date: 2006-08-03 11:44:57 -0700 (Thu, 03 Aug 2006) $
    Revision:       $Revision: 11593 $

    Description:

       This file contains the OSAL Timer definition and manipulation
       functions.

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
 * INCLUDES
 */

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */
#define OSAL_TIMERS_MAX_TIMEOUT 0xFFFF

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */

/*********************************************************************
 * FUNCTIONS
 */

  /*
   * Initialization for the OSAL Timer System.
   */
  extern void osalTimerInit( void );

  /*
   * Set a Timer
   */
  extern byte osal_start_timer( UINT16 event_id, UINT16 timeout_value );
  extern byte osal_start_timerEx( byte task_id, UINT16 event_id, UINT16 timeout_value );

  /*
   * Stop a Timer
   */
  extern byte osal_stop_timer( UINT16 event_id );
  extern byte osal_stop_timerEx( byte task_id, UINT16 event_id );

  /*
   * Get the tick count of a Timer.
   */
  extern UINT16 osal_get_timeoutEx( byte task_id, UINT16 event_id );

  /*
   * Simulated Timer Interrupt Service Routine
   */

  extern void osal_timer_ISR( void );

  /*
   * Adjust timer tables
   */
  extern void osal_adjust_timers( void );

  /*
   * Update timer tables
   */
  extern void osal_update_timers( void );

  /*
   * Count active timers
   */
  extern byte osal_timer_num_active( void );

  /*
   * Set the hardware timer interrupts for sleep mode.
   * These functions should only be called in OSAL_PwrMgr.c
   */
  extern void osal_sleep_timers( void );
  extern void osal_unsleep_timers( void );

 /*
  * Read the system clock - returns milliseconds
  */
  extern uint32 osal_GetSystemClock( void );

  /*
   * Get the next OSAL timer expiration.
   * This function should only be called in OSAL_PwrMgr.c
   */
  extern uint16 osal_next_timeout( void );

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* OSAL_TIMERS_H */
