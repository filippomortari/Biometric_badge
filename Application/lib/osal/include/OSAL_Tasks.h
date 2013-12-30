#ifndef OSAL_TASKS_H
#define OSAL_TASKS_H

/*********************************************************************
    Filename:       OSAL_Tasks.h
    Revised:        $Date: 2006-04-06 08:19:08 -0700 (Thu, 06 Apr 2006) $
    Revision:       $Revision: 10396 $

    Description:

       This file contains the OSAL Task definition and manipulation
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
#define TASK_NO_TASK      0xFF

/* Task priority level */
#define OSAL_TASK_PRIORITY_LOW		50
#define OSAL_TASK_PRIORITY_MED		130
#define OSAL_TASK_PRIORITY_HIGH		230

/*********************************************************************
 * TYPEDEFS
 */
/*
 * Task Initialization function prototype
 */
typedef void (*pTaskInitFn)( unsigned char task_id );

/*
 * Event handler function prototype
 */
typedef unsigned short (*pTaskEventHandlerFn)( unsigned char task_id, unsigned short event );

typedef struct osalTaskRec
{
  struct osalTaskRec  *next;
  pTaskInitFn          pfnInit;
  pTaskEventHandlerFn  pfnEventProcessor;
  byte                 taskID;
  byte                 taskPriority;
  uint16               events;

} osalTaskRec_t;


/*********************************************************************
 * GLOBAL VARIABLES
 */
extern osalTaskRec_t *activeTask;

/*********************************************************************
 * FUNCTIONS
 */

/*
 * Initialization for the Tasking System.
 */
extern void osalTaskInit( void );

/*
 *  Add a task to the task list
 */
extern void osalTaskAdd( pTaskInitFn pfnInit,
                         pTaskEventHandlerFn pfnEventProcessor,
                         byte taskPriority);

/*
 * Call each of the tasks initailization functions.
 */
extern void osalInitTasks( void );

/*
 * This function will return the next active task.
 */
extern osalTaskRec_t *osalNextActiveTask( void );

/*
 * This function will return a task pointer to the task
 *       found with the passed in task ID.
 */
extern osalTaskRec_t *osalFindTask( byte taskID );

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* OSAL_TASKS_H */
