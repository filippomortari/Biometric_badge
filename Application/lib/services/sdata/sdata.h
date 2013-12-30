#ifndef SDATA_H
#define SDATA_H
/**************************************************************************************************
    Filename:       sdata.h
    Revised:        $Date: 2006-04-06 08:19:08 -0700 (Thu, 06 Apr 2006) $
    Revision:       $Revision: 10396 $
    
    Description:    
    
    Data buffer service. 
                
    Copyright (c) 2006 by Texas Instruments, Inc.
    All Rights Reserved.  Permission to use, reproduce, copy, prepare
    derivative works, modify, distribute, perform, display or sell this
    software and/or its documentation for any purpose is prohibited
    without the express written consent of Texas Instruments, Inc.
**************************************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

/* ------------------------------------------------------------------------------------------------
 *                                           Typedefs
 * ------------------------------------------------------------------------------------------------
 */

typedef struct
{
  uint8     *p;
  uint8     len;
} sData_t;

#ifdef __cplusplus
}
#endif

#endif /* SDATA_H */
