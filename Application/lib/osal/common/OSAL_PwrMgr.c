/*********************************************************************
    Filename:       $RCSfile$
    Revised:        $Date: 2006-10-27 16:25:55 -0700 (Fri, 27 Oct 2006) $
    Revision:       $Revision: 12442 $

    Description:

      This file contains the OSAL Power Management API.


    Notes:

    Copyright (c) 2006 by Texas Instruments, Inc.
    All Rights Reserved.  Permission to use, reproduce, copy, prepare
    derivative works, modify, distribute, perform, display or sell this
    software and/or its documentation for any purpose is prohibited
    without the express written consent of Texas Instruments, Inc.
*********************************************************************/


/*********************************************************************
 * INCLUDES
 */

#include "ZComDef.h"
#include "OnBoard.h"
#include "OSAL.h"
#include "OSAL_Tasks.h"
#include "OSAL_Timers.h"
#include "OSAL_PwrMgr.h"

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */

/*********************************************************************
 * TYPEDEFS
 */

/*********************************************************************
 * GLOBAL VARIABLES
 */

/* This global variable stores the power management attributes.
 */
pwrmgr_attribute_t pwrmgr_attribute;

/*********************************************************************
 * EXTERNAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL FUNCTIONS
 */

/*********************************************************************
 * LOCAL VARIABLES
 */

/*********************************************************************
 * LOCAL FUNCTION PROTOTYPES
 */

/*********************************************************************
 * FUNCTIONS
 *********************************************************************/

/*********************************************************************
 * @fn      osal_pwrmgr_init
 *
 * @brief   Initialize the power management system.
 *
 * @param   none.
 *
 * @return  none.
 */
void osal_pwrmgr_init( void )
{
  pwrmgr_attribute.pwrmgr_device = PWRMGR_ALWAYS_ON; // Default to no power conservation.
  pwrmgr_attribute.pwrmgr_task_state = 0;            // Cleared.  All set to conserve
}

/*********************************************************************
 * @fn      osal_pwrmgr_device
 *
 * @brief   Sets the device power characteristic.
 *
 * @param   pwrmgr_device - type of power devices. With PWRMGR_ALWAYS_ON
 *          selection, there is no power savings and the device is most
 *          likely on mains power. The PWRMGR_BATTERY selection allows the
 *          HAL sleep manager to enter sleep.
 *
 * @return  none
 */
void osal_pwrmgr_device( uint8 pwrmgr_device )
{
  pwrmgr_attribute.pwrmgr_device = pwrmgr_device;
}

/*********************************************************************
 * @fn      osal_pwrmgr_task_state
 *
 * @brief   This function is called by each task to state whether or
 *          not this task wants to conserve power.
 *
 * @param   task_id - calling task ID.
 *          state - whether the calling task wants to
 *          conserve power or not.
 *
 * @return  ZSUCCESS if task complete
 */
uint8 osal_pwrmgr_task_state( uint8 task_id, uint8 state )
{
  if ( osalFindTask( task_id ) == NULL )
    return ( INVALID_TASK );

  if ( state == PWRMGR_CONSERVE )
  {
    // Clear the task state flag
    pwrmgr_attribute.pwrmgr_task_state &= ~(1 << task_id );
  }
  else
  {
    // Set the task state flag
    pwrmgr_attribute.pwrmgr_task_state |= (1 << task_id);
  }

  return ( ZSUCCESS );
}

#if defined( POWER_SAVING )
/*********************************************************************
 * @fn      osal_pwrmgr_powerconserve
 *
 * @brief   This function is called from the main OSAL loop when there are
 *          no events scheduled and shouldn't be called from anywhere else.
 *
 * @param   none.
 *
 * @return  none.
 */
void osal_pwrmgr_powerconserve( void )
{
  uint16 next;

  // Should we even look into power conservation
  if ( pwrmgr_attribute.pwrmgr_device != PWRMGR_ALWAYS_ON )
  {
    // Are all tasks in agreement to conserve
    if ( pwrmgr_attribute.pwrmgr_task_state == 0 )
    {
      // Get next time-out
      next = osal_next_timeout();

      // Put the processor into sleep mode
      OSAL_SET_CPU_INTO_SLEEP( next );
    }
  }
}
#endif /* POWER_SAVING */

/*********************************************************************
*********************************************************************/
