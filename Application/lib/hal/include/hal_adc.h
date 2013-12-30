/**************************************************************************************************
  Filename:       hal_adc.h
  Revised:        $Date: 2005/04/29 01:36:04 $
  Revision:       $Revision$

  Description:

  This file contains the interface to the ADC Service.
  This also contains the Task functions.

  Notes:

  Copyright (c) 2006 by Texas Instruments, Inc.
  All Rights Reserved.  Permission to use, reproduce, copy, prepare
  derivative works, modify, distribute, perform, display or sell this
  software and/or its documentation for any purpose is prohibited
  without the express written consent of Texas Instruments, Inc.
**************************************************************************************************/

#ifndef HAL_ADC_H
#define HAL_ADC_H

#ifdef __cplusplus
extern "C"
{
#endif

/**************************************************************************************************
 * INCLUDES
 **************************************************************************************************/
#include "hal_board.h"

/**************************************************************************************************
 * MACROS
 **************************************************************************************************/



/**************************************************************************************************
 * CONSTANTS
 **************************************************************************************************/

/* Resolution */
#define HAL_ADC_RESOLUTION_8       0x01
#define HAL_ADC_RESOLUTION_10      0x02
#define HAL_ADC_RESOLUTION_12      0x03
#define HAL_ADC_RESOLUTION_14      0x04

/* Channels */
#define HAL_ADC_CHANNEL_0          0x00
#define HAL_ADC_CHANNEL_1          0x01
#define HAL_ADC_CHANNEL_2          0x02
#define HAL_ADC_CHANNEL_3          0x03
#define HAL_ADC_CHANNEL_4          0x04
#define HAL_ADC_CHANNEL_5          0x05
#define HAL_ADC_CHANNEL_6          0x06
#define HAL_ADC_CHANNEL_7          0x07

/**************************************************************************************************
 *                                            TYPEDEFS
 **************************************************************************************************/


/**************************************************************************************************
 *                                        GLOBAL VARIABLES
 **************************************************************************************************/


/**************************************************************************************************
 *                                        FUNCTIONS - API
 **************************************************************************************************/

/*
 * Initialize ADC Service
 */
extern void HalAdcInit ( void );

/*
 * Read value from a specified ADC Channel at the given resolution
 */
extern uint16 HalAdcRead ( uint8 channel, uint8 resolution );


/**************************************************************************************************
**************************************************************************************************/

#ifdef __cplusplus
}
#endif

#endif
