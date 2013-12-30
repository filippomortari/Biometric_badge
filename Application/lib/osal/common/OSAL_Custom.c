/*********************************************************************
    Filename:       $RCSfile$
    Author:         $Author: rlynch $
    Revised:        $Date: 2006-08-03 11:44:57 -0700 (Thu, 03 Aug 2006) $
    Revision:       $Revision: 11593 $
    
    Description:    
    
    This file contains all the settings and other functions that
    the user should set and change.
            
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
#include "OSAL.h"
#include "OSAL_Tasks.h"
#include "OSAL_Custom.h"

#if defined ( MT_TASK )
  #include "MTEL.h"
#endif

#if !defined( NONWK )
  #include "nwk.h"
  #include "APS.h"
  #include "ZDApp.h"
#endif

#if defined( APP_GENERIC )  
  #include "GenericApp.h"
#endif
#if defined ( APP_DEBUG )
  #include "DebugApp.h"
#endif
#if defined( APP_SERIAL )  
  #include "SerialApp.h"
#endif
#if defined( APP_DRC )  
  #include "DRC03392_App.h"
#endif
#if defined( APP_DLC )  
  #include "DLC03395_App.h"
#endif
#if defined( APP_LSM )  
  #include "LSM02080_App.h"
#endif
#if defined( APP_OS )  
  #include "OS03393_App.h"
#endif
#if defined( APP_SRC )  
  #include "SRC03391_App.h"
#endif
#if defined( APP_MULTISRC )
  #include "Multi_SRC03391_App.h"
 #endif
#if defined( APP_SLC )  
  #include "SLC03394_App.h"
#endif

#if defined( APP_TGEN)
  #include "TrafficGenApp.h"
#endif

#include "OnBoard.h"
 


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

/*********************************************************************
 * USER DEFINED TASK TABLE 
 */

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
 * @fn      osalAddTasks 
 *
 * @brief   This function adds all the tasks to the task list.
 *          This is where to add new tasks.
 *
 * @param   void
 *
 * @return  none
 */
void osalAddTasks( void )
{

#if defined( MT_TASK )
  osalTaskAdd( MT_TaskInit, MT_ProcessEvent );
#endif

#if !defined( KB_INT )
  osalTaskAdd( Keypad_TaskInit, Keypad_ProcessEvent );
#endif  

#if !defined( NONWK )    
  osalTaskAdd( nwk_init, nwk_event_loop );
  osalTaskAdd( APS_Init, APS_event_loop );
  osalTaskAdd( ZDApp_Init, ZDApp_event_loop );
#endif

#if defined( APP_GENERIC )  
  osalTaskAdd( GenericApp_Init, GenericApp_ProcessEvent );
#endif

#if defined( APP_DEBUG )  
  osalTaskAdd( DebugApp_Init, DebugApp_ProcessEvent );
#endif

#if defined( APP_SERIAL )  
  osalTaskAdd( SerialApp_Init, SerialApp_ProcessEvent );
#endif

#if defined( APP_DRC )  
  osalTaskAdd( DRC03392_Init, DRC03392_ProcessEvent );    // ep 1
#endif

#if defined( APP_DLC )  
  osalTaskAdd( DLC03395_Init, DLC03395_ProcessEvent );    // ep 2
#endif

#if defined( APP_LSM )  
  osalTaskAdd( LSM02080_Init, LSM02080_ProcessEvent );    // ep 3
#endif

#if defined( APP_OS )  
  osalTaskAdd( OS03393_Init, OS03393_ProcessEvent );      // ep 4
#endif

#if defined( APP_SRC )
  osalTaskAdd( SRC03391_Init, SRC03391_ProcessEvent );    // ep 5
#endif

#if defined( APP_MULTISRC )  
  osalTaskAdd( SRC03391_Init, SRC03391_ProcessEvent );    // ep 1 - 4
#endif

#if defined( APP_SLC )  
  osalTaskAdd( SLC03394_Init, SLC03394_ProcessEvent );    // ep 6
#endif

#if defined( APP_TGEN )					
	osalTaskAdd( TrafficGenApp_Init, TrafficGenApp_ProcessEvent);	// ep 21
#endif
}


/*********************************************************************
$Log$
Revision 1.18  2004/10/13 17:12:54  rjessup
Added the Multi SRC application

Revision 1.17  2004/05/11 17:46:49  rjessup
Added the debug_app

Revision 1.16  2004/05/11 01:26:41  joseph
Added traffic generator app

Revision 1.15  2004/05/10 18:37:26  rjessup
Added ability to remove MT_TASK and still keep the key polling

Revision 1.14  2004/03/22 23:23:24  rjessup
Added the Serial transfer functions and sample app

Revision 1.13  2004/03/05 02:31:23  rjessup
Changes for new ZDO

Revision 1.12  2003/12/17 19:49:44  rjessup
Added power manager and memory manager as a seperate module

Revision 1.11  2003/11/18 19:33:36  grayapureddi
include MTEL.h with MT_TASK

Revision 1.10  2003/11/07 18:42:32  rjessup
reduced code size by 9%

Revision 1.9  2003/10/24 00:37:06  rjessup
Added MT_TASK compile flag

Revision 1.8  2003/10/07 23:43:38  rjessup
Updates for port to board

Revision 1.7  2003/10/01 23:09:20  rjessup
Added the APS task

Revision 1.6  2003/09/26 17:01:24  rjessup
Added SRC03391 and SLC00394 device descriptions and apps

Revision 1.5  2003/09/25 23:53:17  rjessup
Added OS03393 Device Description and App

Revision 1.4  2003/09/25 22:35:37  rjessup
Added LSM02080 Device Description and App

Revision 1.3  2003/09/25 01:08:08  rjessup
Added DLC03395 device description/app

Revision 1.2  2003/09/19 23:16:05  rjessup
Moved timer functions to OSAL_Timers and made them dynamic

Revision 1.1  2003/09/19 16:57:14  rjessup
Made the task definitions dynamic
Also, moved user changable items into OSAL_Custom

*********************************************************************/
