#ifndef OSAL_H
#define OSAL_H

/*********************************************************************
    Filename:       OSAL.h
    Revised:        $Date: 2006-09-29 18:09:20 -0700 (Fri, 29 Sep 2006) $
    Revision:       $Revision: 12162 $

    Description:

       This API allows the software components in the Z-stack to be
       written independently of the specifics of the operating system,
       kernel or tasking environment (including control loops or
       connect-to-interrupt systems).

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
#include "ZComDef.h"
#include "OSAL_Memory.h"
#include "OSAL_Timers.h"

/*********************************************************************
 * MACROS
 */

#define osal_offsetof(type, member) ((uint16) &(((type *) 0)->member))

#define OSAL_MSG_NEXT(msg_ptr)      ((osal_msg_hdr_t *) (msg_ptr) - 1)->next

#define OSAL_MSG_Q_INIT(q_ptr)      *(q_ptr) = NULL

#define OSAL_MSG_Q_EMPTY(q_ptr)     (*(q_ptr) == NULL)

#define OSAL_MSG_Q_HEAD(q_ptr)      (*(q_ptr))

/*********************************************************************
 * CONSTANTS
 */

/*** Interrupts ***/
#define INTS_ALL    0xFF


/*********************************************************************
 * TYPEDEFS
 */
typedef struct
{
  void   *next;
  uint16 len;
  byte   dest_id;
} osal_msg_hdr_t;

typedef struct
{
  uint8  event;
  uint8  status;
} osal_event_hdr_t;

typedef void * osal_msg_q_t;

/*********************************************************************
 * GLOBAL VARIABLES
 */

/*********************************************************************
 * FUNCTIONS
 */

/*** Message Management ***/

  /*
   * Task Message Allocation
   */
  extern byte * osal_msg_allocate(uint16 len );

  /*
   * Task Message Deallocation
   */
  extern byte osal_msg_deallocate( byte *msg_ptr );

  /*
   * Task Messages Count
   */
  extern UINT16 osal_num_msgs( void );

  /*
   * Send a Task Message
   */
  extern byte osal_msg_send( byte destination_task, byte *msg_ptr );

  /*
   * Receive a Task Message
   */
  extern byte *osal_msg_receive( byte task_id );


  /*
   * Enqueue a Task Message
   */
  extern void osal_msg_enqueue( osal_msg_q_t *q_ptr, void *msg_ptr );

  /*
   * Enqueue a Task Message Up to Max
   */
  extern byte osal_msg_enqueue_max( osal_msg_q_t *q_ptr, void *msg_ptr, byte max );

  /*
   * Dequeue a Task Message
   */
  extern void *osal_msg_dequeue( osal_msg_q_t *q_ptr );

  /*
   * Push a Task Message to head of queue
   */
  extern void osal_msg_push( osal_msg_q_t *q_ptr, void *msg_ptr );

  /*
   * Extract and remove a Task Message from queue
   */
  extern void osal_msg_extract( osal_msg_q_t *q_ptr, void *msg_ptr, void *prev_ptr );


/*** Task Synchronization  ***/

  /*
   * Set a Task Event
   */
  extern byte osal_set_event( byte task_id, UINT16 event_flag );


/*** Interrupt Management  ***/

  /*
   * Register Interrupt Service Routine (ISR)
   */
  extern byte osal_isr_register( byte interrupt_id, void (*isr_ptr)( byte* ) );

  /*
   * Enable Interrupt
   */
  extern byte osal_int_enable( byte interrupt_id );

  /*
   * Disable Interrupt
   */
  extern byte osal_int_disable( byte interrupt_id );


/*** Task Management  ***/

  /*
   * Initialize the Task System
   */
  extern byte osal_init_system( void );

  /*
   * System Processing Loop
   */
#if defined (ZBIT)
  extern __declspec(dllexport)  void osal_start_system( void );
#else
  extern void osal_start_system( void );
#endif
  /*
   * Get the active task ID
   */
  extern byte osal_self( void );


/*** Helper Functions ***/

  /*
   * String Length
   */
  extern int osal_strlen( char *pString );

  /*
   * Memory copy
   */
  extern void* osal_memcpy( void*, const void GENERIC *, unsigned int );

  /*
   * Copy Extended Addresses
   */
  extern void* osal_cpyExtAddr( void *dest, void *src );

  /*
   * Memory set
   */
  extern void *osal_memset( void *dest, byte value, int len );

  /*
   * Build a uint16 out of 2 bytes (0 then 1).
   */
  extern uint16 osal_build_uint16( byte *swapped );

  /*
   * Build a uint32 out of sequential bytes.
   */
  extern uint32 osal_build_uint32( byte *swapped, byte len );

  /*
   * Convert long to ascii string
   */
  #if !defined ( ZBIT ) && !defined ( ZBIT2 )
  extern byte *_ltoa( uint32 l, byte * buf, byte radix );
  #endif

  /*
   * Compares two address_t structures. true if equal
   */
  extern byte osal_AddrEqual( zAddrType_t *a1, zAddrType_t *a2 );

  /*
   * Verify that the extended addresses are equal. true if equal
   */
  extern byte osal_ExtAddrEqual( byte *extAddr1, byte *extAddr2 );

  /*
   * Random number generator
   */
  extern uint16 osal_rand( void );


/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* OSAL_H */
