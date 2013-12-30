/****************************************************************************
    Filename:       saddr.c
    Revised:        $Date: 2006-04-06 08:19:08 -0700 (Thu, 06 Apr 2006) $
    Revision:       $Revision: 10396 $

    Description:

    Zigbee and 802.15.4 device address utility functions.


    Copyright (c) 2006 by Texas Instruments, Inc.
    All Rights Reserved.  Permission to use, reproduce, copy, prepare
    derivative works, modify, distribute, perform, display or sell this
    software and/or its documentation for any purpose is prohibited
    without the express written consent of Texas Instruments, Inc.
****************************************************************************/

/****************************************************************************
 * INCLUDES
 */
#include "hal_types.h"
#include "OSAL.h"
#include "saddr.h"

/****************************************************************************
 * @fn          sAddrCmp
 *
 * @brief       Compare two device addresses.
 *
 * input parameters
 *
 * @param       pAddr1        - Pointer to first address.
 * @param       pAddr2        - Pointer to second address.
 *
 * output parameters
 *
 * @return      TRUE if addresses are equal, FALSE otherwise
 */
bool sAddrCmp(const sAddr_t *pAddr1, const sAddr_t *pAddr2)
{
  if (pAddr1->addrMode != pAddr2->addrMode)
  {
    return FALSE;
  }
  else if (pAddr1->addrMode == SADDR_MODE_NONE)
  {
    return FALSE;
  }
  else if (pAddr1->addrMode == SADDR_MODE_SHORT)
  {
    return (bool) (pAddr1->addr.shortAddr == pAddr2->addr.shortAddr);
  }
  else if (pAddr1->addrMode == SADDR_MODE_EXT)
  {
    return (sAddrExtCmp(pAddr1->addr.extAddr, pAddr2->addr.extAddr));
  }
  else
  {
    return FALSE;
  }
}

/****************************************************************************
 * @fn          sAddrCpy
 *
 * @brief       Copy a device address.
 *
 * input parameters
 *
 * @param       pSrc         - Pointer to address to copy.
 *
 * output parameters
 *
 * @param       pDest        - Pointer to address of copy.
 *
 * @return      None.
 */
void sAddrCpy(sAddr_t *pDest, const sAddr_t *pSrc)
{
  pDest->addrMode = pSrc->addrMode;

  if (pDest->addrMode == SADDR_MODE_SHORT)
  {
    pDest->addr.shortAddr = pSrc->addr.shortAddr;
  }
  else if (pDest->addrMode == SADDR_MODE_EXT)
  {
    sAddrExtCpy(pDest->addr.extAddr, pSrc->addr.extAddr);
  }
}

/****************************************************************************
 * @fn          sAddrExtCmp
 *
 * @brief       Compare two extended addresses.
 *
 * input parameters
 *
 * @param       pAddr1        - Pointer to first address.
 * @param       pAddr2        - Pointer to second address.
 *
 * output parameters
 *
 * @return      TRUE if addresses are equal, FALSE otherwise
 */
bool sAddrExtCmp(const uint8 * pAddr1, const uint8 * pAddr2)
{
  byte i;

  for (i = SADDR_EXT_LEN; i != 0; i--)
  {
    if (*pAddr1++ != *pAddr2++)
    {
      return FALSE;
    }
  }
  return TRUE;
}

/****************************************************************************
 * @fn          sAddrExtCpy
 *
 * @brief       Copy an extended address.
 *
 * input parameters
 *
 * @param       pSrc         - Pointer to address to copy.
 *
 * output parameters
 *
 * @param       pDest        - Pointer to address of copy.
 *
 * @return      pDest + SADDR_EXT_LEN.
 */
void *sAddrExtCpy(uint8 * pDest, const uint8 * pSrc)
{
  return osal_memcpy(pDest, pSrc, SADDR_EXT_LEN);
}




