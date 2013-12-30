/**************************************************************************************************
    Filename:       mac_cfg.c
    Revised:        $Date: 2006-10-27 16:48:58 -0700 (Fri, 27 Oct 2006) $
    Revision:       $Revision: 12443 $

    Description:

    Compile-time parameters which are configurable by the user.

  Copyright (c) 2006 by Texas Instruments, Inc.
  All Rights Reserved.  Permission to use, reproduce, copy, prepare
  derivative works, modify, distribute, perform, display or sell this
  software and/or its documentation for any purpose is prohibited
  without the express written consent of Texas Instruments, Inc.
**************************************************************************************************/

/* ------------------------------------------------------------------------------------------------
 *                                          Includes
 * ------------------------------------------------------------------------------------------------
 */
#include "mac_api.h"

/* ------------------------------------------------------------------------------------------------
 *                                           Constants
 * ------------------------------------------------------------------------------------------------
 */

/* maximum number of data frames in transmit queue */
#ifndef MAC_CFG_TX_DATA_MAX
#define MAC_CFG_TX_DATA_MAX         2
#endif

/* maximum number of frames of all types in transmit queue */
#ifndef MAC_CFG_TX_MAX
#define MAC_CFG_TX_MAX              5
#endif

/* maximum number of frames in receive queue */
#ifndef MAC_CFG_RX_MAX
#define MAC_CFG_RX_MAX              2
#endif

/* allocate additional bytes in the data indication for application-defined headers */
#ifndef MAC_CFG_DATA_IND_OFFSET
#define MAC_CFG_DATA_IND_OFFSET     0
#endif

/* determine whether MAC_MLME_POLL_IND will be sent to the application */
#ifndef MAC_CFG_APP_PENDING_QUEUE
#define MAC_CFG_APP_PENDING_QUEUE   FALSE
#endif

/* ------------------------------------------------------------------------------------------------
 *                                           Global Variables
 * ------------------------------------------------------------------------------------------------
 */

/* configurable parameters */
const macCfg_t macCfg =
{
  MAC_CFG_TX_DATA_MAX,
  MAC_CFG_TX_MAX,
  MAC_CFG_RX_MAX,
  MAC_CFG_DATA_IND_OFFSET,
  MAC_CFG_APP_PENDING_QUEUE
};
