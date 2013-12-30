#ifndef MAC_HIGH_LEVEL_H
#define MAC_HIGH_LEVEL_H
/**************************************************************************************************
    Filename:       mac_high_level.h
    Revised:        $Date: 2006-10-27 16:48:58 -0700 (Fri, 27 Oct 2006) $
    Revision:       $Revision: 12443 $

    Description:

    Contains interfaces shared between high and low level MAC.

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

/* RX flag masks */
#define MAC_RX_FLAG_VERSION           0x03    /* received frame's version */
#define MAC_RX_FLAG_ACK_PENDING       0x04    /* outgoing ACK has pending bit set */
#define MAC_RX_FLAG_SECURITY          0x08    /* received frame has security bit set */
#define MAC_RX_FLAG_PENDING           0x10    /* received frame has pending bit set */
#define MAC_RX_FLAG_ACK_REQUEST       0x20    /* received frame has ack request bit set */
#define MAC_RX_FLAG_INTRA_PAN         0x40    /* received frame has intra pan fcf bit set */
#define MAC_RX_FLAG_CRC_OK            0x80    /* received frame CRC OK bit */


/* ------------------------------------------------------------------------------------------------
 *                                           Typedefs
 * ------------------------------------------------------------------------------------------------
 */

/* Structure for internal data tx */
typedef struct
{
  macEventHdr_t     hdr;
  sData_t           msdu;
  macTxIntData_t    internal;
  macSec_t          sec;
} macTx_t;

/* Structure for internal data rx */
typedef struct
{
  macEventHdr_t     hdr;
  sData_t           msdu;
  macRxIntData_t    internal;
  macSec_t          sec;
  macDataInd_t      mac;
} macRx_t;

/* ------------------------------------------------------------------------------------------------
 *                                           Global Variables
 * ------------------------------------------------------------------------------------------------
 */

/* pointer to current tx frame */
extern macTx_t *pMacDataTx;

/* TRUE if operating as a PAN coordinator */
extern bool macPanCoordinator;

/* ------------------------------------------------------------------------------------------------
 *                                          Function Prototypes
 * ------------------------------------------------------------------------------------------------
 */

/* functions located in mac_data.c */
uint8 *macDataRxMemAlloc(uint16 len);
uint8 macDataRxMemFree(uint8 *pMsg);
uint8 macDataTxTimeAvailable(void);

/**************************************************************************************************
*/

#endif /* MAC_HIGH_LEVEL_H */
