#ifndef MAC_PIB_H
#define MAC_PIB_H
/**************************************************************************************************
    Filename:       mac_pib.h
    Revised:        $Date: 2006-11-09 13:34:02 -0800 (Thu, 09 Nov 2006) $
    Revision:       $Revision: 12651 $

    Description:

    Internal interface file for the MAC PIB module.

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
 *                                           Typedefs
 * ------------------------------------------------------------------------------------------------
 */

/* MAC PIB type */
typedef struct
{
  uint8             ackWaitDuration;
  bool              associationPermit;
  bool              autoRequest;
  bool              battLifeExt;
  uint8             battLifeExtPeriods;

  uint8             *pBeaconPayload;
  uint8             beaconPayloadLength;
  uint8             beaconOrder;
  uint32            beaconTxTime;
  uint8             bsn;

  sAddr_t           coordExtendedAddress;
  uint16            coordShortAddress;
  uint8             dsn;
  bool              gtsPermit;
  uint8             maxCsmaBackoffs;

  uint8             minBe;
  uint16            panId;
  bool              promiscuousMode;
  bool              rxOnWhenIdle;
  uint16            shortAddress;

  uint8             superframeOrder;
  uint16            transactionPersistenceTime;
  bool              associatedPanCoord;
  uint8             maxBe;
  uint16            maxFrameTotalWaitTime;

  uint8             maxFrameRetries;
  uint8             responseWaitTime;
  uint8             syncSymbolOffset;
  bool              timeStampSupported;
  bool              securityEnabled;

  /* Proprietary */
  uint8             phyTransmitPower;
  uint8             logicalChannel;
  sAddr_t           extendedAddress;
  uint8             altBe;

} macPib_t;

/* ------------------------------------------------------------------------------------------------
 *                                           Global Variables
 * ------------------------------------------------------------------------------------------------
 */

/* MAC PIB */
extern macPib_t macPib;

/* ------------------------------------------------------------------------------------------------
 *                                           Function Prototypes
 * ------------------------------------------------------------------------------------------------
 */

extern void macPibReset(void);

/**************************************************************************************************
*/

#endif /* MAC_PIB_H */

