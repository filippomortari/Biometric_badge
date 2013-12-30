/**************************************************************************************************
    Filename:
    Revised:        $Date: 2006-11-16 14:55:59 -0800 (Thu, 16 Nov 2006) $
    Revision:       $Revision: 12760 $

    Description:

    Describe the purpose and contents of the file.

       Copyright (c) 2006 by Texas Instruments, Inc.
    All Rights Reserved.  Permission to use, reproduce, copy, prepare
    derivative works, modify, distribute, perform, display or sell this
    software and/or its documentation for any purpose is prohibited
    without the express written consent of Texas Instruments, Inc.
**************************************************************************************************/



/* ------------------------------------------------------------------------------------------------
 *                                             Includes
 * ------------------------------------------------------------------------------------------------
 */
#include "mac_radio_defs.h"
#include "hal_types.h"
#include "hal_assert.h"


/* ------------------------------------------------------------------------------------------------
 *                                        Global Constants
 * ------------------------------------------------------------------------------------------------
 */

   ///////////////////////////////////////////////////////////////////////////////////
   //  FIX_ON_REV_C : These values are not particularly accurate.  We should
   //  receive newly characterized values when Rev C arrives.  Follow up with
   //  Oslo to make sure we get the new data.  A value for each setting would be nice.
   ///////////////////////////////////////////////////////////////////////////////////

const uint8 CODE macRadioDefsTxPowerTable[] =
{
  /*   0 dBm */   0x1F,   /* === data sheet value ===*/
  /*  -1 dBm */   0x1B,   /* === data sheet value ===*/
  /*  -2 dBm */   0x1B,
  /*  -3 dBm */   0x17,   /* === data sheet value ===*/
  /*  -4 dBm */   0x17,
  /*  -5 dBm */   0x13,   /* === data sheet value ===*/
  /*  -6 dBm */   0x13,
  /*  -7 dBm */   0x0F,   /* === data sheet value ===*/
  /*  -8 dBm */   0x0F,
  /*  -9 dBm */   0x0F,
  /* -10 dBm */   0x0B,   /* === data sheet value ===*/
  /* -11 dBm */   0x0B,
  /* -12 dBm */   0x0B,
  /* -13 dBm */   0x0B,
  /* -14 dBm */   0x0B,
  /* -15 dBm */   0x07,   /* === data sheet value ===*/
  /* -16 dBm */   0x07,
  /* -17 dBm */   0x07,
  /* -18 dBm */   0x07,
  /* -19 dBm */   0x07,
  /* -20 dBm */   0x07,
  /* -21 dBm */   0x07,
  /* -22 dBm */   0x07,
  /* -23 dBm */   0x07,
  /* -24 dBm */   0x07,
  /* -25 dBm */   0x03    /* === data sheet value ===*/
};



/**************************************************************************************************
 *                                  Compile Time Integrity Checks
 **************************************************************************************************
 */
HAL_ASSERT_SIZE(macRadioDefsTxPowerTable, MAC_RADIO_TX_POWER_MAX_MINUS_DBM+1);  /* array size mismatch */

#if (HAL_CPU_CLOCK_MHZ != 32)
#error "ERROR: The only tested/supported clock speed is 32 MHz."
#endif


/**************************************************************************************************
 */
