/**************************************************************************************************
    Filename:       msa_Osal.c
    Revised:        $Date: 2006-03-10 15:05:25 -0800 (Fri, 10 Mar 2006) $
    Revision:       $Revision: 10030 $

    Description: This file contains function that allows user setup tasks

    Notes:

    Copyright (c) 2004 by Figure 8 Wireless, Inc., All Rights Reserved.
    Permission to use, reproduce, copy, prepare derivative works,
    modify, distribute, perform, display or sell this software and/or
    its documentation for any purpose is prohibited without the express
    written consent of Figure 8 Wireless, Inc.
**************************************************************************************************/


/**************************************************************************************************
 *                                            INCLUDES
 **************************************************************************************************/
#include "hal_types.h"
#include "OSAL.h"
#include "OSAL_Tasks.h"
#include "OSAL_Custom.h"
#include "OnBoard.h"
#include "mac_api.h"

/* HAL */
#include "hal_drivers.h"

/* Application */
#include "msa.h"



/**************************************************************************************************
 *
 * @fn      osalAddTasks
 *
 * @brief   This function adds all the tasks to the task list.
 *          This is where to add new tasks.
 *
 * @param   void
 *
 * @return  none
 *
 **************************************************************************************************/
void osalAddTasks( void )
{

  /* HAL Drivers Task */
  osalTaskAdd (Hal_Init, Hal_ProcessEvent, OSAL_TASK_PRIORITY_LOW);

  /* MAC Task */
  osalTaskAdd( macTaskInit, macEventLoop, OSAL_TASK_PRIORITY_HIGH );

  /* Application Task */
  osalTaskAdd( MSA_Init, MSA_ProcessEvent, OSAL_TASK_PRIORITY_MED );

}

/**************************************************************************************************
**************************************************************************************************/
