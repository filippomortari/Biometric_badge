/**************************************************************************************************
    Filename:
    Revised:        $Date: 2006-11-16 13:51:15 -0800 (Thu, 16 Nov 2006) $
    Revision:       $Revision: 12756 $

    Description:

    Describe the purpose and contents of the file.

    Copyright (c) 2006 by Texas Instruments, Inc.
    All Rights Reserved.  Permission to use, reproduce, copy, prepare
    derivative works, modify, distribute, perform, display or sell this
    software and/or its documentation for any purpose is prohibited
    without the express written consent of Texas Instruments, Inc.
**************************************************************************************************/



/* ------------------------------------------------------------------------------------------------
 *                                           Includes
 * ------------------------------------------------------------------------------------------------
 */

/* hal */
#include "hal_types.h"

/* low-level specific */
#include "mac_mem.h"

/* target specific */
#include "hal_mcu.h"

/* debug */
#include "mac_assert.h"


/**************************************************************************************************
 * @fn          macMemReadRamByte
 *
 * @brief       Read a byte from RAM.
 *
 * @param       pRam - pointer to byte RAM byte to read
 *
 * @return      byte read from RAM
 **************************************************************************************************
 */
uint8 macMemReadRamByte(macRam_t * pRam)
{
  return(*pRam);
}


/**************************************************************************************************
 * @fn          macMemWriteRam
 *
 * @brief       Write multiple bytes to RAM.
 *
 * @param       pRam  - pointer to RAM to be written to
 * @param       pData - pointer to data to write
 * @param       len   - number of bytes to write
 *
 * @return      none
 **************************************************************************************************
 */
void macMemWriteRam(macRam_t * pRam, uint8 * pData, uint8 len)
{
  while (len)
  {
    len--;
    *pRam = *pData;
    pRam++;
    pData++;
  }
}


/**************************************************************************************************
 * @fn          macMemReadRam
 *
 * @brief       Read multiple bytes from RAM.
 *
 * @param       pRam  - pointer to RAM to be read from
 * @param       pData - pointer to location to store read data
 * @param       len   - number of bytes to read
 *
 * @return      none
 **************************************************************************************************
 */
void macMemReadRam(macRam_t * pRam, uint8 * pData, uint8 len)
{
  while (len)
  {
    len--;
    *pData = *pRam;
    pRam++;
    pData++;
  }
}


/**************************************************************************************************
 * @fn          macMemWriteTxFifo
 *
 * @brief       Write multiple bytes to the transmit FIFO.
 *
 * @param       pData - pointer to bytes to be written to TX FIFO
 * @param       len   - number of bytes to write
 *
 * @return      none
 **************************************************************************************************
 */
void macMemWriteTxFifo(uint8 * pData, uint8 len)
{
  MAC_ASSERT(len != 0); /* pointless to write zero bytes */

  do
  {
    RFD = *pData;
    pData++;
    len--;
  }
  while (len);
}


/**************************************************************************************************
 * @fn          macMemReadRxFifo
 *
 * @brief       Read multiple bytes from receive FIFO.
 *
 * @param       pData - pointer to location to store read data
 * @param       len   - number of bytes to read from RX FIFO
 *
 * @return      none
 **************************************************************************************************
 */
void macMemReadRxFifo(uint8 * pData, uint8 len)
{
  MAC_ASSERT(len != 0); /* pointless to read zero bytes */

  do
  {
    *pData = RFD;
    pData++;
    len--;
  }
  while (len);
}


/**************************************************************************************************
*/
