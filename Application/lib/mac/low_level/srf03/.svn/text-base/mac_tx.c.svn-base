/**************************************************************************************************
    Filename:
    Revised:        $Date: 2006-12-05 14:28:04 -0800 (Tue, 05 Dec 2006) $
    Revision:       $Revision: 12971 $

    Description:

    Describe the purpose and contents of the file.

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

/* hal */
#include "hal_types.h"
#include "hal_defs.h"

/* high-level */
#include "mac_spec.h"
#include "mac_pib.h"

/* exported low-level */
#include "mac_low_level.h"

/* low-level specific */
#include "mac_tx.h"
#include "mac_backoff_timer.h"
#include "mac_rx.h"
#include "mac_rx_onoff.h"
#include "mac_radio.h"
#include "mac_sleep.h"

/* target specific */
#include "mac_radio_defs.h"

/* debug */
#include "mac_assert.h"
#include "hal_board.h"


/* ------------------------------------------------------------------------------------------------
 *                                            Defines
 * ------------------------------------------------------------------------------------------------
 */
#define MFR_LEN                   MAC_FCS_FIELD_LEN
#define PREPENDED_BYTE_LEN        1


/* ------------------------------------------------------------------------------------------------
 *                                         Global Variables
 * ------------------------------------------------------------------------------------------------
 */
uint8 macTxSlottedDelay;
uint8 macTxActive;
uint8 macTxFrameInQueueFlag;
uint8 macTxType;
uint8 macTxBe;
uint8 macTxCsmaBackoffDelay;
uint8 macTxListenForAck;


/* ------------------------------------------------------------------------------------------------
 *                                         Local Variables
 * ------------------------------------------------------------------------------------------------
 */
static uint8 nb;
static uint8 txSeqn;
static uint8 txAckReq;
static uint8 txRetransmitFlag;


/* ------------------------------------------------------------------------------------------------
 *                                         Local Prototypes
 * ------------------------------------------------------------------------------------------------
 */
static void txCsmaPrep(void);
static void txCsmaGo(void);
static void txComplete(uint8 status);


/**************************************************************************************************
 * @fn          macTxInit
 *
 * @brief       Initialize variables for tx module.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
void macTxInit(void)
{
  macTxActive           = FALSE;
  macTxFrameInQueueFlag = FALSE;
  txRetransmitFlag      = FALSE;
  macTxListenForAck     = FALSE;

  /////////////////////////////////////////////////////////////////////////////////////////
  // TBD : This hard coded value needs to be abstracted cleanly based on hardware used.
  macTxSlottedDelay = 3;
  /////////////////////////////////////////////////////////////////////////////////////////
}


/**************************************************************************************************
 * @fn          macTxHaltCleanup
 *
 * @brief       -
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
void macTxHaltCleanup(void)
{
  MAC_RADIO_TX_RESET();
  macTxInit();
}


/**************************************************************************************************
 * @fn          macTxFrame
 *
 * @brief       Transmit the frame pointed to by pMacDataTx with unslotted CSMA.
 *              NOTE! It is not legal to call this function from interrupt context.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
void macTxFrame(uint8 txType)
{
  MAC_ASSERT(!macTxActive);                                 /* transmit on top of transmit */
  MAC_ASSERT(!macTxFrameInQueueFlag);                       /* already a queued transmit */
  MAC_ASSERT(macSleepState == MAC_SLEEP_STATE_AWAKE);       /* radio must be awake */

  /* initialize */
  macTxActive = TRUE;
  macTxType   = txType;

  /*-------------------------------------------------------------------------------
   *  Prepare for transmit.
   */
  if (macTxType == MAC_TX_TYPE_SLOTTED)
  {
    MAC_RADIO_TX_PREP_SLOTTED();
  }
  else
  {
    MAC_ASSERT((macTxType == MAC_TX_TYPE_SLOTTED_CSMA) || (macTxType == MAC_TX_TYPE_UNSLOTTED_CSMA));

    nb = 0;
    macTxBe = (pMacDataTx->internal.txOptions & MAC_TXOPTION_ALT_BE) ? macPib.altBe : macPib.minBe;

    if ((macTxType == MAC_TX_TYPE_SLOTTED_CSMA) && (macPib.battLifeExt))
    {
      macTxBe = MIN(2, macTxBe);
    }

    txCsmaPrep();
  }


  /*-------------------------------------------------------------------------------
   *  Load transmit FIFO unless this is a retransmit.  No need to write
   *  the FIFO again in that case.
   */
  if (!txRetransmitFlag)
  {
    uint8 * p;
    uint8   lenMhrMsdu;

    MAC_ASSERT(pMacDataTx != NULL); /* must have data to transmit */

    /* save needed parameters */
    txAckReq = MAC_ACK_REQUEST(pMacDataTx->msdu.p);
    txSeqn   = MAC_SEQ_NUMBER(pMacDataTx->msdu.p);

    /* set length of frame (note: use of term msdu is a misnomer, here it's actually mhr + msdu) */
    lenMhrMsdu = pMacDataTx->msdu.len;

    /* calling code guarantees an unused prepended byte  */
    p = pMacDataTx->msdu.p - PREPENDED_BYTE_LEN;

    /* first byte of buffer is length of MPDU */
    *p = lenMhrMsdu + MFR_LEN;

    /*
     *  Flush the TX FIFO.  This is necessary in case the previous transmit was never
     *  actually sent (e.g. CSMA failed without strobing TXON).  If bytes are written to
     *  the FIFO but not transmitted, they remain in the FIFO to be transmitted whenever
     *  a strobe of TXON does happen.
     */
    MAC_RADIO_FLUSH_TX_FIFO();

    /* write bytes to FIFO, prepended byte is included, MFR is not (it's generated by hardware) */
    MAC_RADIO_WRITE_TX_FIFO(p, PREPENDED_BYTE_LEN + lenMhrMsdu);
  }

  /*-------------------------------------------------------------------------------
   *  If not receiving, start the transmit.  If receive is active
   *  queue up the transmit.
   */
  {
    halIntState_t  s;

    HAL_ENTER_CRITICAL_SECTION(s);
    if (!macRxActive)
    {
      HAL_EXIT_CRITICAL_SECTION(s);
      if (macTxType == MAC_TX_TYPE_SLOTTED)
      {
        MAC_RADIO_TX_GO_SLOTTED();
      }
      else
      {
        txCsmaGo();
      }
    }
    else
    {
      macTxFrameInQueueFlag = TRUE;
      HAL_EXIT_CRITICAL_SECTION(s);
    }
  }
}


/*=================================================================================================
 * @fn          txCsmaPrep
 *
 * @brief       -
 *
 * @param       none
 *
 * @return      none
 *=================================================================================================
 */
static void txCsmaPrep(void)
{
  macTxCsmaBackoffDelay = macRandomByte() & ((1 << macTxBe) - 1);

  if (macTxType == MAC_TX_TYPE_SLOTTED_CSMA)
  {
    MAC_RADIO_TX_PREP_CSMA_SLOTTED();
  }
  else
  {
    MAC_RADIO_TX_PREP_CSMA_UNSLOTTED();
  }
}


/*=================================================================================================
 * @fn          txCsmaGo
 *
 * @brief       -
 *
 * @param       none
 *
 * @return      none
 *=================================================================================================
 */
static void txCsmaGo(void)
{
  if (macTxType == MAC_TX_TYPE_SLOTTED_CSMA)
  {
    if (macTxCsmaBackoffDelay >= macDataTxTimeAvailable())
    {
      txComplete(MAC_NO_TIME);
      return;
    }
  }

  MAC_RADIO_TX_GO_CSMA();
}


/**************************************************************************************************
 * @fn          macTxFrameRetransmit
 *
 * @brief       -
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
void macTxFrameRetransmit(void)
{
  txRetransmitFlag = TRUE;
  macTxFrame(macTxType);
}


/**************************************************************************************************
 * @fn          macTxStartQueuedFrame
 *
 * @brief       -
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
void macTxStartQueuedFrame(void)
{
  halIntState_t  s;

  HAL_ENTER_CRITICAL_SECTION(s);
  if (macTxFrameInQueueFlag)
  {
    macTxFrameInQueueFlag = FALSE;
    HAL_EXIT_CRITICAL_SECTION(s);
    if (macTxType == MAC_TX_TYPE_SLOTTED)
    {
      MAC_RADIO_TX_GO_SLOTTED();
    }
    else
    {
      txCsmaGo();
    }
  }
  HAL_EXIT_CRITICAL_SECTION(s);
}


/**************************************************************************************************
 * @fn          macTxDoneCallback
 *
 * @brief       -
 *
 * @param       -
 *
 * @return      none
 **************************************************************************************************
 */
void macTxDoneCallback(uint8 status)
{
  if (status == MAC_TXDONE_SUCCESS)
  {
    /* see if ACK was requested */
    if (!txAckReq)
    {
      /* ACK was not requested, transmit is complete */
      txComplete(MAC_SUCCESS);
    }
    else
    {
      /*
       *  ACK was requested - must wait to receive it.  A timer is set
       *  to expire after the timeout duration for waiting for an ACK.
       *  If an ACK is received, the function macTxAckReceived() is called.
       *  If an ACK is not received within the timeout period,
       *  the function macTxAckTimeoutCallback() is called.
       */
      macTxListenForAck = TRUE;
      MAC_RADIO_TX_REQUEST_ACK_TIMEOUT_CALLBACK();
    }
  }
  else if (status == MAC_TXDONE_CHANNEL_BUSY)
  {
    MAC_ASSERT((macTxType == MAC_TX_TYPE_SLOTTED_CSMA) || (macTxType == MAC_TX_TYPE_UNSLOTTED_CSMA));

    /*  clear channel assement failed, follow through with CSMA algorithm */
    nb++;
    if (nb > macPib.maxCsmaBackoffs)
    {
      txComplete(MAC_CHANNEL_ACCESS_FAILURE);
    }
    else
    {
      macTxBe = MIN(macTxBe+1, macPib.maxBe);
      txCsmaPrep();
      txCsmaGo();
    }
  }
  else
  {
    MAC_ASSERT(status == MAC_TXDONE_INSUFFICIENT_TIME);

    txComplete(MAC_NO_TIME);
  }
}


/**************************************************************************************************
 * @fn          macTxAckReceived
 *
 * @brief       Function called by interrupt routine when an ACK is received.
 *
 * @param       seqn        - sequence number of received ACK
 * @param       pendingFlag - set to TRUE if pending flag of ACK is set, FALSE otherwise
 *
 * @return      none
 **************************************************************************************************
 */
void macTxAckReceived(uint8 seqn, uint8 pendingFlag)
{
  halIntState_t  s;

  /* only process the received ACK if the macTxListenForAck flag is set */
  HAL_ENTER_CRITICAL_SECTION(s);
  if (macTxListenForAck)
  {
    macTxListenForAck = FALSE;
    HAL_EXIT_CRITICAL_SECTION(s);

    /* cancel the ACK callback */
    MAC_RADIO_TX_CANCEL_ACK_TIMEOUT_CALLBACK();

    /* see if the sequence number of received ACK matches sequence number of packet just sent */
    if (seqn == txSeqn)
    {
      /*
       *  Sequence numbers match so transmit is successful.  Return appropriate
       *  status based on the pending flag of the received ACK.
       */
      if (pendingFlag)
      {
        txComplete(MAC_ACK_PENDING);
      }
      else
      {
        txComplete(MAC_SUCCESS);
      }
    }
    else
    {
      /* sequence number did not match; per spec, transmit failed at this point */
      txComplete(MAC_NO_ACK);
    }
  }

  HAL_EXIT_CRITICAL_SECTION(s);
}


/**************************************************************************************************
 * @fn          macTxAckTimeoutCallback
 *
 * @brief       This function is called if ACK is not received within timeout period.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
void macTxAckTimeoutCallback(void)
{
  halIntState_t  s;

  HAL_ENTER_CRITICAL_SECTION(s);
  if (macTxListenForAck)
  {
    macTxListenForAck = FALSE;
    HAL_EXIT_CRITICAL_SECTION(s);
    txComplete(MAC_NO_ACK);
  }
  HAL_EXIT_CRITICAL_SECTION(s);
}


/*=================================================================================================
 * @fn          txComplete
 *
 * @brief       Transmit has completed.  Perform needed maintenance and return status of
 *              the transmit via callback function.
 *
 * @param       status - status of the transmit that just went out
 *
 * @return      none
 *=================================================================================================
 */
static void txComplete(uint8 status)
{
  /*
   *  Unless receive is active, update power here.  Power is *always* updated at end of receive.
   */
  if (!macRxActive)
  {
    macRadioUpdateTxPower();
  }

  /*
   *  Channel cannot change during transmit so update it here.  (Channel *can* change during
   *  a receive.  The update function resets receive logic and any partially received
   *  frame is purged.)
   */
  macRadioUpdateChannel();

  /* reset the retransmit flag */
  txRetransmitFlag = FALSE;

  /* update tx state; turn off receiver if nothing is keeping it on */
  macTxActive = FALSE;

  /* turn off receive if allowed */
  macRxOffRequest();

  /* return status of transmit via callback function */
  macTxCompleteCallback(status);
}


/**************************************************************************************************
 * @fn          macTxTimestampCallback
 *
 * @brief       -
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
void macTxTimestampCallback(void)
{
  MAC_ASSERT(pMacDataTx != NULL); /* must have data to transmit */

  pMacDataTx->internal.timestamp = MAC_RADIO_BACKOFF_CAPTURE();
  pMacDataTx->internal.timestamp2 = MAC_RADIO_TIMER_CAPTURE();
}


/**************************************************************************************************
 * @fn          macTxCollisionWithRxCallback
 *
 * @brief       Function called if transmit strobed on top of a receive.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
void macTxCollisionWithRxCallback(void)
{
  macRxHaltCleanup();
}


/**************************************************************************************************
*/
