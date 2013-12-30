/**************************************************************************************************
    Filename:       hal_adc.c
    Revised:        $Date: 2006-11-01 14:05:21 -0800 (Wed, 01 Nov 2006) $
    Revision:       $Revision: 12475 $

    Description:

    This file contains the interface to the HAL ADC.

    Copyright (c) 2006 by Texas Instruments, Inc.
    All Rights Reserved.  Permission to use, reproduce, copy, prepare
    derivative works, modify, distribute, perform, display or sell this
    software and/or its documentation for any purpose is prohibited
    without the express written consent of Texas Instruments, Inc.
**************************************************************************************************/

/**************************************************************************************************
 *                                           INCLUDES
 **************************************************************************************************/
#include  "hal_mcu.h"
#include  "hal_defs.h"
#include  "hal_types.h"
#include  "hal_adc.h"

/**************************************************************************************************
 *                                            CONSTANTS
 **************************************************************************************************/
#define HAL_ADC_EOC         0x80    /* End of Conversion bit */
#define HAL_ADC_START       0x40    /* Starts Conversion */

#define HAL_ADC_STSEL_EXT   0x00    /* External Trigger */
#define HAL_ADC_STSEL_FULL  0x10    /* Full Speed, No Trigger */
#define HAL_ADC_STSEL_T1C0  0x20    /* Timer1, Channel 0 Compare Event Trigger */
#define HAL_ADC_STSEL_ST    0x30    /* ADCCON1.ST =1 Trigger */

#define HAL_ADC_RAND_NORM   0x00    /* Normal Operation */
#define HAL_ADC_RAND_LFSR   0x04    /* Clock LFSR */
#define HAL_ADC_RAND_SEED   0x08    /* Seed Modulator */
#define HAL_ADC_RAND_STOP   0x0c    /* Stop Random Generator */
#define HAL_ADC_RAND_BITS   0x0c    /* Bits [3:2] */

#define HAL_ADC_REF_125V    0x00    /* Internal 1.25V Reference */
#define HAL_ADC_REF_AIN7    0x40    /* AIN7 Reference */
#define HAL_ADC_REF_AVDD    0x80    /* AVDD_SOC Pin Reference */
#define HAL_ADC_REF_DIFF    0xc0    /* AIN7,AIN6 Differential Reference */
#define HAL_ADC_REF_BITS    0xc0    /* Bits [7:6] */

#define HAL_ADC_DEC_064     0x00    /* Decimate by 64 : 8-bit resolution */
#define HAL_ADC_DEC_128     0x10    /* Decimate by 128 : 10-bit resolution */
#define HAL_ADC_DEC_256     0x20    /* Decimate by 256 : 12-bit resolution */
#define HAL_ADC_DEC_512     0x30    /* Decimate by 512 : 14-bit resolution */
#define HAL_ADC_DEC_BITS    0x30    /* Bits [5:4] */

#define HAL_ADC_CHN_AIN0    0x00    /* AIN0 */
#define HAL_ADC_CHN_AIN1    0x01    /* AIN1 */
#define HAL_ADC_CHN_AIN2    0x02    /* AIN2 */
#define HAL_ADC_CHN_AIN3    0x03    /* AIN3 */
#define HAL_ADC_CHN_AIN4    0x04    /* AIN4 */
#define HAL_ADC_CHN_AIN5    0x05    /* AIN5 */
#define HAL_ADC_CHN_AIN6    0x06    /* AIN6 */
#define HAL_ADC_CHN_AIN7    0x07    /* AIN7 */
#define HAL_ADC_CHN_A0A1    0x08    /* AIN0,AIN1 */
#define HAL_ADC_CHN_A2A3    0x09    /* AIN2,AIN3 */
#define HAL_ADC_CHN_A4A5    0x0a    /* AIN4,AIN5 */
#define HAL_ADC_CHN_A6A7    0x0b    /* AIN6,AIN7 */
#define HAL_ADC_CHN_GND     0x0c    /* GND */
#define HAL_ADC_CHN_VREF    0x0d    /* Positive voltage reference */
#define HAL_ADC_CHN_TEMP    0x0e    /* Temperature sensor */
#define HAL_ADC_CHN_VDD3    0x0f    /* VDD/3 */
#define HAL_ADC_CHN_BITS    0x0f    /* Bits [3:0] */

#define HAL_ADC_STSEL       HAL_ADC_STSEL_ST
#define HAL_ADC_RAND_GEN    HAL_ADC_RAND_STOP
#define HAL_ADC_REF_VOLT    HAL_ADC_REF_AVDD
#define HAL_ADC_DEC_RATE    HAL_ADC_DEC_064
#define HAL_ADC_SCHN        HAL_ADC_CHN_VDD3
#define HAL_ADC_ECHN        HAL_ADC_CHN_GND


/**************************************************************************************************
 *                                              MACROS
 **************************************************************************************************/
#define HAL_ADC_CLR_EOC()   asm("PUSH A"); asm("MOV A,ADCL"); asm("MOV A,ADCH"); asm("POP A");

/**************************************************************************************************
 *                                            TYPEDEFS
 **************************************************************************************************/

/**************************************************************************************************
 *                                         GLOBAL VARIABLES
 **************************************************************************************************/

/**************************************************************************************************
 *                                          FUNCTIONS - API
 **************************************************************************************************/

/**************************************************************************************************
 * @fn      HalAdcInit
 *
 * @brief   Initialize ADC Service
 *
 * @param   None
 *
 * @return  None
 **************************************************************************************************/
void HalAdcInit (void)
{
  volatile uint8  tmp;

  ADCCON1 = HAL_ADC_STSEL | HAL_ADC_RAND_GEN | 0x03;
  ADCCON2 = HAL_ADC_REF_VOLT | HAL_ADC_DEC_RATE | HAL_ADC_SCHN;
  /*
  *  After reset, the first ADC reading of the extra conversion always reads GND level.
  *  We will do a few dummy conversions to bypass this bug.
  */
  tmp = ADCL;     /* read ADCL,ADCH to clear EOC */
  tmp = ADCH;
  ADCCON3 = HAL_ADC_REF_VOLT | HAL_ADC_DEC_RATE | HAL_ADC_ECHN;
  while ((ADCCON1 & HAL_ADC_EOC) != HAL_ADC_EOC);   /* Wait for conversion */
  tmp = ADCL;     /* read ADCL,ADCH to clear EOC */
  tmp = ADCH;
  ADCCON3 = HAL_ADC_REF_VOLT | HAL_ADC_DEC_RATE | HAL_ADC_ECHN;
  while ((ADCCON1 & HAL_ADC_EOC) != HAL_ADC_EOC);   /* Wait for conversion */
  tmp = ADCL;     /* read ADCL,ADCH to clear EOC */
  tmp = ADCH;
}

/**************************************************************************************************
 * @fn      HalAdcRead
 *
 * @brief   Read the ADC based on given channel and resolution
 *
 * @param   channel - channel where ADC will be read
 * @param   resolution - the resolution of the value
 *
 * @return  16 bit value of the ADC in offset binary format.
 *          Note that the ADC is "bipolar", which means the GND (0V) level is mid-scale.
 **************************************************************************************************/
uint16 HalAdcRead (uint8 channel, uint8 resolution)
{
  uint8   i, resbits;
  uint8   adctemp;
  int16  reading;
  volatile  uint8 tmp;
  uint8  adcChannel = 1;

  /*
  * If Analog input channel is AIN0..AIN7, make sure corresponing P0 I/O pin is enabled.  The code
  * does NOT disable the pin at the end of this function.  I think it is better to leave the pin
  * enabled because the results will be more accurate.  Because of the inherent capacitance on the
  * pin, it takes time for the voltage on the pin to charge up to its steady-state level.  If
  * HalAdcRead() has to turn on the pin for every conversion, the results may show a lower voltage
  * than actuality because the pin did not have time to fully charge.
  */
  if (channel < 8)
  {
    for (i=0; i < channel; i++)
    {
      adcChannel <<= 1;
    }
  }

  /* Enable channel */
  ADCCFG |= adcChannel;

  /* Convert resolution to decimation rate */
  switch (resolution)
  {
    case HAL_ADC_RESOLUTION_8:
      resbits = HAL_ADC_DEC_064;
      break;
    case HAL_ADC_RESOLUTION_10:
      resbits = HAL_ADC_DEC_128;
      break;
    case HAL_ADC_RESOLUTION_12:
      resbits = HAL_ADC_DEC_256;
      break;
    case HAL_ADC_RESOLUTION_14:
    default:
      resbits = HAL_ADC_DEC_512;
      break;
  }

  /* read ADCL,ADCH to clear EOC */
  tmp = ADCL;
  tmp = ADCH;

  /* Setup Sample */
  adctemp = ADCCON3;
  adctemp &= ~(HAL_ADC_CHN_BITS | HAL_ADC_DEC_BITS);
  adctemp |= channel | resbits;

  /* writing to this register starts the extra conversion */
  ADCCON3 = adctemp;

  /* Wait for the conversion to be done */
  while (!(ADCCON1 & HAL_ADC_EOC));

  /* Disable channel after done conversion */
  ADCCFG &= ~adcChannel;

  /* Read the result */
  reading = (int16) (ADCL);
  reading |= (int16) (ADCH << 8);

  /* Treat small negative as 0 */
  if (reading < 0)
    reading = 0;

  switch (resolution)
  {
    case HAL_ADC_RESOLUTION_8:
      reading >>= 8;
      break;
    case HAL_ADC_RESOLUTION_10:
      reading >>= 6;
      break;
    case HAL_ADC_RESOLUTION_12:
      reading >>= 4;
      break;
    case HAL_ADC_RESOLUTION_14:
    default:
    break;
  }

  return ((uint16)reading);
}

/**************************************************************************************************
**************************************************************************************************/




