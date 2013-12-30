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
#include "hal_defs.h"
#include "hal_types.h"

/* high-level */
#include "mac_high_level.h"
#include "mac_spec.h"

/* exported low-level */
#include "mac_low_level.h"

/* low-level specific */
#include "mac_rx.h"
#include "mac_tx.h"
#include "mac_rx_onoff.h"
#include "mac_radio.h"

/* target specific */
#include "mac_radio_defs.h"

/* debug */
#include "mac_assert.h"


/* ------------------------------------------------------------------------------------------------
 *                                            Defines
 * ------------------------------------------------------------------------------------------------
 */
#define MAX_PAYLOAD_BYTES_READ_PER_INTERRUPT   16   /* adjustable to tune performance */

/* receive FIFO bytes needed to start a valid receive (see function rxStartIsr for details) */
#define RX_THRESHOLD_START_LEN    (MAC_PHY_PHR_LEN        +  \
                                   MAC_FCF_FIELD_LEN      +  \
                                   MAC_SEQ_NUM_FIELD_LEN  +  \
                                   MAC_FCS_FIELD_LEN)

/* maximum size of addressing fields (note: command frame identifier processed as part of address) */
#define MAX_ADDR_FIELDS_LEN  ((MAC_EXT_ADDR_FIELD_LEN + MAC_PAN_ID_FIELD_LEN) * 2)

/* addressing mode reserved value */
#define ADDR_MODE_RESERVERED  1

/* length of command frame identifier */
#define CMD_FRAME_ID_LEN      1

/* packet size mask is equal to the maximum value */
#define PHY_PACKET_SIZE_MASK  0x7F

/* value for promiscuous off, must not conflict with other mode variants from separate include files */
#define PROMISCUOUS_MODE_OFF  0x00

/* bit of proprietary FCS format that indicates if the CRC is OK */
#define PROPRIETARY_FCS_CRC_OK_BIT  0x80


/* ------------------------------------------------------------------------------------------------
 *                                             Macros
 * ------------------------------------------------------------------------------------------------
 */
#define MEM_ALLOC(x)   macDataRxMemAlloc(x)
#define MEM_FREE(x)    macDataRxMemFree((uint8 *)x)

/*
 *  Macro for encoding frame control information into internal flags format.
 *  Parameter is pointer to the frame.  NOTE!  If either the internal frame
 *  format *or* the specification changes, this macro will need to be modified.
 */
#define INTERNAL_FCF_FLAGS(p)  ((((p)[1] >> 4) & 0x03) | ((p)[0] & 0x78))

/*
 *  The radio replaces the actual FCS with different information.  This proprietary FCS is
 *  the same length as the original and includes:
 *    1) the RSSI value
 *    2) the average correlation value (used for LQI)
 *    3) a CRC passed bit
 *
 *  These macros decode the proprietary FCS.  The macro parameter is a pointer to the two byte FCS.
 */
#define PROPRIETARY_FCS_RSSI(p)                 ((int8)((p)[0]))
#define PROPRIETARY_FCS_CRC_OK(p)               ((p)[1] & PROPRIETARY_FCS_CRC_OK_BIT)
#define PROPRIETARY_FCS_CORRELATION_VALUE(p)    ((p)[1] & ~PROPRIETARY_FCS_CRC_OK_BIT)


/* ------------------------------------------------------------------------------------------------
 *                                       Global Variables
 * ------------------------------------------------------------------------------------------------
 */
uint8 macRxActive;
uint8 macRxFilter;


/* ------------------------------------------------------------------------------------------------
 *                                       Local Constants
 * ------------------------------------------------------------------------------------------------
 */
static const uint8 CODE macRxAddrLen[] =
{
  0,                                                /* no address */
  0,                                                /* reserved */
  MAC_PAN_ID_FIELD_LEN + MAC_SHORT_ADDR_FIELD_LEN,  /* short address + pan id */
  MAC_PAN_ID_FIELD_LEN + MAC_EXT_ADDR_FIELD_LEN     /* extended address + pan id */
};


/* ------------------------------------------------------------------------------------------------
 *                                       Local Prototypes
 * ------------------------------------------------------------------------------------------------
 */
static void rxStartIsr(void);
static void rxAddrIsr(void);
static void rxPayloadIsr(void);
static void rxDiscardIsr(void);
static void rxFcsIsr(void);

static void rxPrepPayload(void);
static void rxDiscardFrame(void);
static void rxDone(void);


/* ------------------------------------------------------------------------------------------------
 *                                         Local Variables
 * ------------------------------------------------------------------------------------------------
 */
static void    (* pFuncRxState)(void);
static macRx_t  * pRxBuf;

static uint8  rxBuf[MAC_PHY_PHR_LEN + MAC_FCF_FIELD_LEN + MAC_SEQ_NUM_FIELD_LEN];
static uint8  rxUnreadLen;
static uint8  rxNextLen;
static uint8  rxPayloadLen;
static uint8  rxAckReq;
static uint8  rxFilter;
static uint8  rxPromiscuousMode;


/**************************************************************************************************
 * @fn          macRxInit
 *
 * @brief       Initialize receive variables and set radio threshold.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
void macRxInit(void)
{
  macRxFilter       = RX_FILTER_OFF;
  rxPromiscuousMode = PROMISCUOUS_MODE_OFF;
  pRxBuf            = NULL; /* required for macRxReset() to function correctly */
  macRxActive       = FALSE;
  pFuncRxState      = &rxStartIsr;
  MAC_RADIO_SET_RX_THRESHOLD(RX_THRESHOLD_START_LEN);
}


/**************************************************************************************************
 * @fn          macRxTxReset
 *
 * @brief       Reset the receive state.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
void macRxTxReset(void)
{
  /* forces receiver off, cleans up by calling macRxHaltCleanup() and macTxHaltCleanup() */
  macRxHardDisable();

  /*
   *   Note : transmit does not require any reset logic
   *          beyond what macRxHardDisable() provides.
   */

  /* restore deault filter mode to off */
  macRxFilter = RX_FILTER_OFF;

  /* return promiscous mode to default off state */
  macRxPromiscuousMode(MAC_PROMISCUOUS_MODE_OFF);
}


/**************************************************************************************************
 * @fn          macRxHaltCleanup
 *
 * @brief       Cleanup up the receiver logic after a forced off.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
void macRxHaltCleanup(void)
{
  /////////////////////////////////////////////////////////////////////////////////////////
  // TBD : must be made callable at any time, including if in middle of rx interrupt
  /////////////////////////////////////////////////////////////////////////////////////////

  /* first thing, disable further receive interrupts */
  MAC_RADIO_DISABLE_RX_THRESHOLD_INTERRUPT();

  /* cancel any upcoming ACK transmit complete callback */
  MAC_RADIO_CANCEL_ACK_TX_DONE_CALLBACK();

  /* flush the receive FIFO */
  MAC_RADIO_FLUSH_RX_FIFO();

  /* if data buffer has been allocated, free it */
  if (pRxBuf != NULL)
  {
    MEM_FREE((uint8 *) pRxBuf);
  }
  pRxBuf = NULL; /* needed to indicate buffer is no longer allocated */

  /* reset receive logic to be ready for next receive */
  macRxActive = FALSE;
  pFuncRxState = &rxStartIsr;
  MAC_RADIO_SET_RX_THRESHOLD(RX_THRESHOLD_START_LEN);

  /* clear any receive interrupt that happened to get through */
  MAC_RADIO_CLEAR_RX_THRESHOLD_INTERRUPT_FLAG();

  /* re-enable interrupts */
  MAC_RADIO_ENABLE_RX_THRESHOLD_INTERRUPT();
}


/**************************************************************************************************
 * @fn          macRxThresholdIsr
 *
 * @brief       Interrupt service routine called when bytes in FIFO reach threshold value.
 *              It implements a state machine for receiving a packet.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
void macRxThresholdIsr(void)
{
  /* handle current state of receive */
  (*pFuncRxState)();
}


/*=================================================================================================
 * @fn          rxStartIsr
 *
 * @brief       First ISR state for receiving a packet - compute packet length, allocate
 *              buffer, initialize buffer.  Acknowledgements are handled immediately without
 *              allocating a buffer.
 *
 * @param       none
 *
 * @return      none
 *=================================================================================================
 */
static void rxStartIsr(void)
{
  uint8  addrLen;
  uint8  ackWithPending;
  uint8  dstAddrMode;
  uint8  srcAddrMode;

  MAC_ASSERT(macRxActive == 0); /* receive on top of receive */

  /* indicate rx is active */
  macRxActive = TRUE;

  /*
   *  Make a module-local copy of macRxFilter.  This prevents the selected
   *  filter from changing in the middle of a receive.
   */
  rxFilter = macRxFilter;

  /*-------------------------------------------------------------------------------
   *  Read initial frame information from FIFO.
   *
   *   This code is not triggered until the following are in the RX FIFO:
   *     frame length          - one byte containing length of MAC frame (excludes this field)
   *     frame control field   - two bytes defining frame type, addressing fields, control flags
   *     sequence number       - one byte unique sequence identifier
   *     additional two bytes  - these bytes are available in case the received frame is an ACK,
   *                             if so, the frame can be verified and responded to immediately,
   *                             if not an ACK, these bytes will be processed normally
   */

  /* read frame length, frame control field, and sequence number from FIFO */
  MAC_RADIO_READ_RX_FIFO(rxBuf, MAC_PHY_PHR_LEN + MAC_FCF_FIELD_LEN + MAC_SEQ_NUM_FIELD_LEN);

  /* bytes to read from FIFO equals frame length minus length of MHR fields just read from FIFO */
  rxUnreadLen = (rxBuf[0] & PHY_PACKET_SIZE_MASK) - MAC_FCF_FIELD_LEN - MAC_SEQ_NUM_FIELD_LEN;

  /*
   *  Workaround for chip bug.  The receive buffer can sometimes be corrupted by hardware.
   *  This usually occurs under heavy traffic.  If a corrupted receive buffer is detected
   *  the entire receive buffer is flushed.
   */
#ifdef MAC_RADIO_RXBUFF_CHIP_BUG
  if ((rxUnreadLen > (MAC_A_MAX_PHY_PACKET_SIZE - MAC_FCF_FIELD_LEN - MAC_SEQ_NUM_FIELD_LEN)) ||
      (MAC_FRAME_TYPE(&rxBuf[1]) > MAC_FRAME_TYPE_MAX_VALID))
  {
    MAC_RADIO_FLUSH_RX_FIFO();
    rxDone();
    return;
  }
#endif

  /*
   *  Workaround for probable software logic hole that causes receive-on-top-of-receive assert.
   *  This workaround only addresses 2420 targets.  The root cause does need to be understood
   *  and then fixed properly.
   */
#ifdef MAC_RADIO_CC2420
    /* cleanup any transmit pending on SFD interrupt */
    if (HAL_MAC_SFD_INT_IS_ENABLED() && !macTxListenForAck)
    {
      macSfdCancelTxDoneCallback();
      macDualchipTxDoneIsr();
    }
#endif

  
  /*-------------------------------------------------------------------------------
   *  If receiving an ACK, handle immediately and exit.  There are guaranteed
   *  to be two unread bytes in the FIFO.  For an ACK frame type these bytes
   *  will be the FCS.
   */
  if (MAC_FRAME_TYPE(&rxBuf[1]) == MAC_FRAME_TYPE_ACK)
  {
    /*
     *  In promiscuous mode is 'off', go ahead process the ACK here and exit.
     *  If promiscuous mode is 'on' though, skip this ACK processing and let the
     *  ACK be treated like any other frame.
     */
    if (rxPromiscuousMode == PROMISCUOUS_MODE_OFF)
    {
      uint8 fcsBuf[MAC_FCF_FIELD_LEN];

      /* read FCS from FIFO (threshold set so bytes are guaranteed to be there) */
      MAC_RADIO_READ_RX_FIFO(fcsBuf, MAC_FCS_FIELD_LEN);

      /* if CRC passed, call ACK handling function */
      if (PROPRIETARY_FCS_CRC_OK(fcsBuf))
      {
        /* call transmit logic to indicate ACK was received */
        macTxAckReceived(MAC_SEQ_NUMBER(&rxBuf[1]), MAC_FRAME_PENDING(&rxBuf[1]));
      }

      /* receive is done, exit from here */
      rxDone();
      return;
    }
  }
  else if (macTxListenForAck)
  {
    /*
     *  If transmit is active and listening for an ACK and something else is
     *  received, the transmit logic must complete before continuing with receive.
     */
     MAC_RADIO_TX_CANCEL_ACK_TIMEOUT_CALLBACK();
     macTxAckTimeoutCallback();
  }

  /*-------------------------------------------------------------------------------
   *  Set the ACK request flag.
   */
  rxAckReq = MAC_ACK_REQUEST(&rxBuf[1]);

  /*-------------------------------------------------------------------------------
   *  Apply filtering and promiscuous mode.
   *
   *  In promiscuous mode, frames are never ACK'ed.  Clear the ACK flag so an
   *  ACK will not be sent.  Also, ignore all filtering.  Promiscuous mode allows
   *  all frames to be received.
   *
   *  For efficient filtering, the first test is to see if filtering is applied at all.
   *  If it's not, no further action is needed.  If filtering is active, each type of
   *  filter is tested.  If the frame does not meet the filter's criteria it is discarded.
   */
  if (rxPromiscuousMode != PROMISCUOUS_MODE_OFF)
  {
    /* for promiscuous mode ACKs are not sent, clear flag here */
    rxAckReq = FALSE;
  }
  else if (rxFilter == RX_FILTER_OFF)
  {
    /* filtering is off, do nothing and just continue */
  }
  else if (/* filter all frames */
           (rxFilter == RX_FILTER_ALL) ||

           /* filter non-beacon frames */
           ((rxFilter == RX_FILTER_NON_BEACON_FRAMES) &&
            (MAC_FRAME_TYPE(&rxBuf[1]) != MAC_FRAME_TYPE_BEACON)) ||

           /* filter non-command frames */
           ((rxFilter == RX_FILTER_NON_COMMAND_FRAMES) &&
            ((MAC_FRAME_TYPE(&rxBuf[1]) != MAC_FRAME_TYPE_COMMAND))))
  {
    /* discard rest of frame */
    rxDiscardFrame();
    return;
  }

  /*-------------------------------------------------------------------------------
   *  Compute length of addressing fields.  Compute payload length.
   */

  /* decode addressing modes */
  dstAddrMode = MAC_DEST_ADDR_MODE(&rxBuf[1]);
  srcAddrMode = MAC_SRC_ADDR_MODE(&rxBuf[1]);

  /*
   *  Workaround for chip bug.  The receive buffer can sometimes be corrupted by hardware.
   *  This usually occurs under heavy traffic.  If a corrupted receive buffer is detected
   *  the entire receive buffer is flushed.
   */
#ifdef MAC_RADIO_RXBUFF_CHIP_BUG
  if ((srcAddrMode == ADDR_MODE_RESERVERED) || (dstAddrMode == ADDR_MODE_RESERVERED))
  {
    MAC_RADIO_FLUSH_RX_FIFO();
    rxDone();
    return;
  }
#endif

  /*
   *  Compute the addressing field length.  A lookup table based on addressing
   *  mode is used for efficiency.  If the source address is present and the
   *  frame is intra-PAN, the PAN Id is not repeated.  In this case, the address
   *  length is adjusted to match the smaller length.
   */
  addrLen = macRxAddrLen[dstAddrMode] + macRxAddrLen[srcAddrMode];
  if ((srcAddrMode != SADDR_MODE_NONE) && MAC_INTRA_PAN(&rxBuf[1]))
  {
    addrLen -= MAC_PAN_ID_FIELD_LEN;
  }

  /*
   *  If there are not enough unread bytes to include the computed address
   *  plus FCS field, the frame is corrupted and must be discarded.
   */
  if ((addrLen + MAC_FCS_FIELD_LEN) > rxUnreadLen)
  {
    /* discard frame and exit */
    rxDiscardFrame();
    return;
  }

  /* payload length is equal to unread bytes minus address length, minus the FCS */
  rxPayloadLen = rxUnreadLen - addrLen - MAC_FCS_FIELD_LEN;

  /*-------------------------------------------------------------------------------
   *  Allocate memory for the incoming frame.
   */
  pRxBuf = (macRx_t *) MEM_ALLOC(sizeof(macRx_t) + rxPayloadLen);
  if (pRxBuf == NULL)
  {
    /* buffer allocation failed, discard the frame and exit*/
    rxDiscardFrame();
    return;
  }

  /*-------------------------------------------------------------------------------
   *  Send ACK request.
   */
  ackWithPending = 0;
  if (rxAckReq)
  {
    if ((MAC_FRAME_TYPE(&rxBuf[1]) == MAC_FRAME_TYPE_COMMAND) && macRxCheckPendingCallback())
    {
      MAC_RADIO_TX_ACK_PEND();
      ackWithPending = MAC_RX_FLAG_ACK_PENDING;
    }
    else
    {
      /* send ACK */
      MAC_RADIO_TX_ACK();
    }

    /* request a callback to macRxAckTxDoneCallback() when the ACK transmit has finished */
    MAC_RADIO_REQUEST_ACK_TX_DONE_CALLBACK();
  }

 /*-------------------------------------------------------------------------------
  *  Populate the receive buffer going up to high-level.
  */

  /* configure the payload buffer */
  pRxBuf->msdu.p = (uint8 *) (pRxBuf + 1);
  pRxBuf->msdu.len = rxPayloadLen;

  /* set internal values */
  pRxBuf->mac.srcAddr.addrMode  = srcAddrMode;
  pRxBuf->mac.dstAddr.addrMode  = dstAddrMode;
  pRxBuf->mac.timestamp         = MAC_RADIO_BACKOFF_CAPTURE();
  pRxBuf->mac.timestamp2        = MAC_RADIO_TIMER_CAPTURE();
  pRxBuf->internal.frameType    = MAC_FRAME_TYPE(&rxBuf[1]);
  pRxBuf->mac.dsn               = MAC_SEQ_NUMBER(&rxBuf[1]);
  pRxBuf->internal.flags        = INTERNAL_FCF_FLAGS(&rxBuf[1]) | ackWithPending;
  pRxBuf->sec.securityLevel     = MAC_SEC_LEVEL_NONE;

  /*-------------------------------------------------------------------------------
   *  If the processing the addressing fields does not require more bytes from
   *  the FIFO go directly address processing function.  Otherwise, configure
   *  interrupt to jump there once bytes are received.
   */
  if (addrLen == 0)
  {
    /* no addressing fields to read, prepare for payload interrupts */
    pFuncRxState = &rxPayloadIsr;
    rxPrepPayload();
  }
  else
  {
    /* need to read and process addressing fields, prepare for address interrupt */
    rxNextLen = addrLen;
    MAC_RADIO_SET_RX_THRESHOLD(rxNextLen);
    pFuncRxState = &rxAddrIsr;
  }
}


/*=================================================================================================
 * @fn          rxAddrIsr
 *
 * @brief       Receive ISR state for decoding address.  Reads and stores the address information
 *              from the incoming packet.
 *
 * @param       none
 *
 * @return      none
 *=================================================================================================
 */
static void rxAddrIsr(void)
{
  uint8 buf[MAX_ADDR_FIELDS_LEN];
  uint8 dstAddrMode;
  uint8 srcAddrMode;
  uint8  * p;

  MAC_ASSERT(rxNextLen != 0); /* logic assumes at least one address byte in buffer */

  /*  read out address fields into local buffer in one shot */
  MAC_RADIO_READ_RX_FIFO(buf, rxNextLen);

  /* set pointer to buffer with addressing fields */
  p = buf;

  /* destination address */
  dstAddrMode = MAC_DEST_ADDR_MODE(&rxBuf[1]);
  if (dstAddrMode != SADDR_MODE_NONE)
  {
    pRxBuf->mac.srcPanId = pRxBuf->mac.dstPanId = BUILD_UINT16(p[0], p[1]);
    p += MAC_PAN_ID_FIELD_LEN;
    if (dstAddrMode == SADDR_MODE_EXT)
    {
      sAddrExtCpy(pRxBuf->mac.dstAddr.addr.extAddr, p);
      p += MAC_EXT_ADDR_FIELD_LEN;
    }
    else
    {
      pRxBuf->mac.dstAddr.addr.shortAddr = BUILD_UINT16(p[0], p[1]);
      p += MAC_SHORT_ADDR_FIELD_LEN;
    }
  }

  /* sources address */
  srcAddrMode = MAC_SRC_ADDR_MODE(&rxBuf[1]);
  if (srcAddrMode != SADDR_MODE_NONE)
  {
    if (!(pRxBuf->internal.flags & MAC_RX_FLAG_INTRA_PAN))
    {
      pRxBuf->mac.srcPanId = BUILD_UINT16(p[0], p[1]);
      p += MAC_PAN_ID_FIELD_LEN;
    }
    if (srcAddrMode == SADDR_MODE_EXT)
    {
      sAddrExtCpy(pRxBuf->mac.srcAddr.addr.extAddr, p);
    }
    else
    {
      pRxBuf->mac.srcAddr.addr.shortAddr = BUILD_UINT16(p[0], p[1]);
    }
  }

  /*-------------------------------------------------------------------------------
   *  Prepare for payload interrupts.
   */
  pFuncRxState = &rxPayloadIsr;
  rxPrepPayload();
}


/*=================================================================================================
 * @fn          rxPrepPayload
 *
 * @brief       Common code to prepare for the payload ISR.
 *
 * @param       none
 *
 * @return      none
 *=================================================================================================
 */
static void rxPrepPayload(void)
{
  if (rxPayloadLen == 0)
  {
    MAC_RADIO_SET_RX_THRESHOLD(MAC_FCS_FIELD_LEN);
    pFuncRxState = &rxFcsIsr;
  }
  else
  {
    rxNextLen = MIN(rxPayloadLen, MAX_PAYLOAD_BYTES_READ_PER_INTERRUPT);
    MAC_RADIO_SET_DYNAMIC_RX_THRESHOLD(rxNextLen);
  }
}


/*=================================================================================================
 * @fn          rxPayloadIsr
 *
 * @brief       Receive ISR state for reading out and storing the packet payload.
 *
 * @param       none
 *
 * @return      none
 *=================================================================================================
 */
static void rxPayloadIsr(void)
{
  MAC_RADIO_READ_RX_FIFO(pRxBuf->msdu.p, rxNextLen);
  pRxBuf->msdu.p += rxNextLen;
  rxPayloadLen -= rxNextLen;

  rxPrepPayload();
}


/*=================================================================================================
 * @fn          rxFcsIsr
 *
 * @brief       Receive ISR state for handling the FCS.
 *
 * @param       none
 *
 * @return      none
 *=================================================================================================
 */
static void rxFcsIsr(void)
{
  uint8 crcOK;

  /* read FCS, rxBuf is not available */
  MAC_RADIO_READ_RX_FIFO(rxBuf, MAC_FCS_FIELD_LEN);

  /*
   *  The FCS has actually been replaced within the radio by a proprietary version of the FCS.
   *  This proprietary FCS is two bytes (same length as the real FCS) and contains:
   *    1) the RSSI value
   *    2) the average correlation value (used for LQI)
   *    3) a CRC passed bit
   */

  /* save the CRC-is-OK status */
  crcOK = PROPRIETARY_FCS_CRC_OK(rxBuf);

  /*
   *  See if the frame should be passed up to high-level MAC.  If the CRC is OK, the
   *  the frame is always passed up.  Frames with a bad CRC are also passed up *if*
   *  a special variant of promiscuous mode is active.
   */
  if (crcOK || (rxPromiscuousMode == MAC_PROMISCUOUS_MODE_WITH_BAD_CRC))
  {
    /*
     *  The frame will be passed up to high-level.  Record the link quality.
     *  Set the MSDU pointer to point at start of data.  Finally, execute
     *  the callback function.
     */
    pRxBuf->internal.flags |= crcOK;
    pRxBuf->mac.mpduLinkQuality = MAC_RADIO_CONVERT_RSSI_TO_LQI(PROPRIETARY_FCS_RSSI(rxBuf));
    pRxBuf->mac.rssi = PROPRIETARY_FCS_RSSI(rxBuf);
    pRxBuf->mac.lqi = PROPRIETARY_FCS_CORRELATION_VALUE(rxBuf);
    pRxBuf->msdu.p = (uint8 *) (pRxBuf + 1);
    macRxCompleteCallback(pRxBuf);
    pRxBuf = NULL; /* needed to indicate buffer is no longer allocated */

    /*
     *  If an ACK has been requested, must wait for it to complete.
     *  If not, go directly to final receive-is-done function.
     */
    {
      halIntState_t  s;

      HAL_ENTER_CRITICAL_SECTION(s);
      if (rxAckReq)
      {
        rxAckReq = 0;
        HAL_EXIT_CRITICAL_SECTION(s);

        /* disable any further receive interrupts while waiting for ACK */
        MAC_RADIO_SET_RX_THRESHOLD(126);
        //MAC_RADIO_DISABLE_RX_THRESHOLD_INTERRUPT();
      }
      else
      {
        HAL_EXIT_CRITICAL_SECTION(s);
        /* if an ACK was not requested, transmit is done at this point */
        rxDone();
      }

    }
  }
  else
  {
    /*
     *  The CRC failed so the packet must be discarded.
     *  Cancel any pending interrupt and free up memory.
     */

    /* cancel any upcoming ACK callback */
    MAC_RADIO_CANCEL_ACK_TX_DONE_CALLBACK();

    /* free up the memory */
    MEM_FREE((uint8 *) pRxBuf);
    pRxBuf = NULL;  /* needed to indicate buffer is no longer allocated */

    rxDone();
  }
}


/*=================================================================================================
 * @fn          rxDiscardFrame
 *
 * @brief       Initializes for discarding a packet.
 *
 * @param       none
 *
 * @return      none
 *=================================================================================================
 */
static void rxDiscardFrame(void)
{
  if (rxUnreadLen == 0)
  {
    rxDone();
  }
  else
  {
    rxNextLen = MIN(rxUnreadLen, MAX_PAYLOAD_BYTES_READ_PER_INTERRUPT);
    MAC_RADIO_SET_RX_THRESHOLD(rxNextLen);
    pFuncRxState = &rxDiscardIsr;
  }
}


/*=================================================================================================
 * @fn          rxDiscardIsr
 *
 * @brief       Receive ISR state for discarding a packet.
 *
 * @param       none
 *
 * @return      none
 *=================================================================================================
 */
static void rxDiscardIsr(void)
{
  uint8 buf[MAX_PAYLOAD_BYTES_READ_PER_INTERRUPT];

  MAC_RADIO_READ_RX_FIFO(buf, rxNextLen);
  rxUnreadLen -= rxNextLen;

  /* read out and discard bytes until all bytes of packet are disposed of */
  if (rxUnreadLen != 0)
  {
    if (rxUnreadLen < MAX_PAYLOAD_BYTES_READ_PER_INTERRUPT)
    {
      rxNextLen = rxUnreadLen;
      MAC_RADIO_SET_DYNAMIC_RX_THRESHOLD(rxNextLen);
    }
  }
  else
  {
    rxDone();
  }
}


/**************************************************************************************************
 * @fn          macRxAckTxDoneCallback
 *
 * @brief       Function called when the ACK has been completed transmitting.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
void macRxAckTxDoneCallback(void)
{
  halIntState_t  s;

  HAL_ENTER_CRITICAL_SECTION(s);
  if (rxAckReq)
  {
    rxAckReq = 0;
    HAL_EXIT_CRITICAL_SECTION(s);
  }
  else
  {
    HAL_EXIT_CRITICAL_SECTION(s);
    rxDone();
  }
}


/*=================================================================================================
 * @fn          rxDone
 *
 * @brief       Common exit point for receive.
 *
 * @param       none
 *
 * @return      none
 *=================================================================================================
 */
static void rxDone(void)
{
  /* reset the receive state and the threshold for receiving a new packet */
  pFuncRxState = &rxStartIsr;

  /* if the receive FIFO has overflowed, flush it here */
  if (MAC_RADIO_RX_FIFO_OVERFLOW())
  {
    MAC_RADIO_FLUSH_RX_FIFO();
  }

  /* update power after receive has completed, physical transmit guaranteed not to be active */
  macRadioUpdateTxPower();

  /* mark receive as inactive */
  macRxActive = FALSE;

  /* turn off receive if allowed */
  macRxOffRequest();

  /* activate any queued transmit */
  macTxStartQueuedFrame();

  /* reset threshold level */
  MAC_RADIO_SET_RX_THRESHOLD(RX_THRESHOLD_START_LEN);
}


/**************************************************************************************************
 * @fn          macRxPromiscuousMode
 *
 * @brief       -
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
void macRxPromiscuousMode(uint8 mode)
{
  rxPromiscuousMode = mode;

  if (rxPromiscuousMode == MAC_PROMISCUOUS_MODE_OFF)
  {
      MAC_RADIO_TURN_ON_RX_FRAME_FILTERING();
  }
  else
  {
    MAC_ASSERT((mode == MAC_PROMISCUOUS_MODE_WITH_BAD_CRC)   ||
               (mode == MAC_PROMISCUOUS_MODE_COMPLIANT));  /* invalid mode */

    MAC_RADIO_TURN_OFF_RX_FRAME_FILTERING();
  }
}



/**************************************************************************************************
 *                                  Compile Time Integrity Checks
 **************************************************************************************************
 */

/* check for changes to the spec that would affect the source code */
#if ((MAC_A_MAX_PHY_PACKET_SIZE   !=  0x7F )   ||  \
     (MAC_FCF_FIELD_LEN           !=  2    )   ||  \
     (MAC_FCF_FRAME_TYPE_POS      !=  0    )   ||  \
     (MAC_FCF_FRAME_PENDING_POS   !=  4    )   ||  \
     (MAC_FCF_ACK_REQUEST_POS     !=  5    )   ||  \
     (MAC_FCF_INTRA_PAN_POS       !=  6    )   ||  \
     (MAC_FCF_DST_ADDR_MODE_POS   !=  10   )   ||  \
     (MAC_FCF_FRAME_VERSION_POS   !=  12   )   ||  \
     (MAC_FCF_SRC_ADDR_MODE_POS   !=  14   ))
#error "ERROR!  Change to the spec that requires modification of source code."
#endif

/* check for changes to the internal flags format */
#if ((MAC_RX_FLAG_VERSION      !=  0x03)  ||  \
     (MAC_RX_FLAG_ACK_PENDING  !=  0x04)  ||  \
     (MAC_RX_FLAG_SECURITY     !=  0x08)  ||  \
     (MAC_RX_FLAG_PENDING      !=  0x10)  ||  \
     (MAC_RX_FLAG_ACK_REQUEST  !=  0x20)  ||  \
     (MAC_RX_FLAG_INTRA_PAN    !=  0x40))
#error "ERROR!  Change to the internal RX flags format.  Requires modification of source code."
#endif

/* validate CRC OK bit optimization */
#if (MAC_RX_FLAG_CRC_OK != PROPRIETARY_FCS_CRC_OK_BIT)
#error "ERROR!  Optimization relies on these bits have the same position."
#endif


/**************************************************************************************************
*/
