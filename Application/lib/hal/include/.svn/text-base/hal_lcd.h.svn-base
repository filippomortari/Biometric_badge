#ifndef HAL_LCD_H
#define HAL_LCD_H

/**************************************************************************************************
  Filename:       hal_lcd.h
  Revised:        $Date: 2005/04/29 01:36:04 $
  Revision:       $Revision$

  Description:

  This file contains the interface to the LCD Service.
  This also contains the Task functions.

  Notes:

  Copyright (c) 2006 by Texas Instruments, Inc.
  All Rights Reserved.  Permission to use, reproduce, copy, prepare
  derivative works, modify, distribute, perform, display or sell this
  software and/or its documentation for any purpose is prohibited
  without the express written consent of Texas Instruments, Inc.
**************************************************************************************************/



#ifdef __cplusplus
extern "C"
{
#endif

/**************************************************************************************************
 *                                          INCLUDES
 **************************************************************************************************/
#include "hal_board.h"

/**************************************************************************************************
 *                                         CONSTANTS
 **************************************************************************************************/

/* These are used to specify which line the text will be printed */
#define HAL_LCD_LINE_1      0x01
#define HAL_LCD_LINE_2      0x02

/* Max number of chars on a single LCD line */
#define HAL_LCD_MAX_CHARS   16
#define HAL_LCD_MAX_BUFF    25

/**************************************************************************************************
 *                                          MACROS
 **************************************************************************************************/


/**************************************************************************************************
 *                                         TYPEDEFS
 **************************************************************************************************/


/**************************************************************************************************
 *                                     GLOBAL VARIABLES
 **************************************************************************************************/


/**************************************************************************************************
 *                                     FUNCTIONS - API
 **************************************************************************************************/

/*
 * Initialize LCD Service
 */
extern void HalLcdInit(void);

/*
 * Write a string to the LCD
 */
extern void HalLcdWriteString ( char *str, uint8 option);

/*
 * Write a value to the LCD
 */
extern void HalLcdWriteValue ( uint32 value, const uint8 radix, uint8 option);

/*
 * Write a value to the LCD
 */
extern void HalLcdWriteScreen( char *line1, char *line2 );

/*
 * Write a string followed by a value to the LCD
 */
extern void HalLcdWriteStringValue( char *title, uint16 value, uint8 format, uint8 line );

/*
 * Write a string followed by 2 values to the LCD
 */
extern void HalLcdWriteStringValueValue( char *title, uint16 value1, uint8 format1, uint16 value2, uint8 format2, uint8 line );

/*
 * Write a percentage bar to the LCD
 */
extern void HalLcdDisplayPercentBar( char *title, uint8 value );


/**************************************************************************************************
**************************************************************************************************/

#ifdef __cplusplus
}
#endif

#endif
