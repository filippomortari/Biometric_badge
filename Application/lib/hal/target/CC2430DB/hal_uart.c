/**************************************************************************************************
    Filename:       hal_uart.c
    Revised:        $Date: 2006-11-16 11:03:41 -0800 (Thu, 16 Nov 2006) $
    Revision:       $Revision: 12751 $

    Description:

    This file contains the interface to the UART.
    This also contains the Task functions.

    Copyright (c) 2006 by Texas Instruments, Inc.
    All Rights Reserved.  Permission to use, reproduce, copy, prepare
    derivative works, modify, distribute, perform, display or sell this
    software and/or its documentation for any purpose is prohibited
    without the express written consent of Texas Instruments, Inc.
**************************************************************************************************/

/*********************************************************************
 * INCLUDES
 */
#include "hal_mcu.h"
#include "hal_types.h"
#include "hal_defs.h"
#include "hal_uart.h"
#include "osal.h"


/*********************************************************************
 * MACROS
 */

#define HAL_UART_HW_DISABLE 0x00    /* Macro used to indicate UART is present */
#define HAL_UART_HW_ENABLE  0x01    /* Macro used to indicate UART is present */

/* Enable UART depends on board type */
#if defined (HAL_BOARD_CC2430EB)    /* UART0, ALT1 enabled */
  #define HAL_UART_0_HW_STATUS  HAL_UART_HW_ENABLE
  #define HAL_UART_1_HW_STATUS  HAL_UART_HW_DISABLE
#endif

#if defined (HAL_BOARD_CC2430DB)    /* UART1, ALT1 enabled */
  #define HAL_UART_0_HW_STATUS  HAL_UART_HW_DISABLE
  #define HAL_UART_1_HW_STATUS  HAL_UART_HW_ENABLE
#endif

#if HAL_UART_0_HW_STATUS == HAL_UART_HW_ENABLE

/* UART0 Pin Configuration Constants */
#define HAL_UART_0_PERCFG_BIT       0x01  /* Alt 2 location is Port 1 */
#define HAL_UART_0_P0SEL_SET        0x0c  /* Enable Tx,Rx */
#define HAL_UART_0_P0SEL_RTCT_SET   0x30  /* Enable RT,CT */
#define HAL_UART_0_P1SEL_SET        0x30  /* Enable Tx,Rx */
#define HAL_UART_0_P1SEL_RTCT_SET   0x0c  /* Enable RT,CT */
#define HAL_UART_0_P2DIR_BITS       0xc0  /* HAL_UART_0 has priority on Port 0 */
#define HAL_UART_0_ALT1_RTS         0x20  /* UART0-ALT1 RTS on P0.5 */
#define HAL_UART_0_ALT2_RTS         0x08  /* UART0-ALT1 RTS on P1.3 */

#endif // UART 0

#if HAL_UART_1_HW_STATUS == HAL_UART_HW_ENABLE

/* UART1 Pin Configuration Constants */
#define HAL_UART_1_PERCFG_BIT       0x02  /* Alt 2 location is Port 1 */
#define HAL_UART_1_P0SEL_SET        0x30  /* Enable Rx,Tx */
#define HAL_UART_1_P0SEL_RTCT_SET   0x0c  /* Enable RT,CT */
#define HAL_UART_1_P1SEL_SET        0xc0  /* Enable Rx,Tx */
#define HAL_UART_1_P1SEL_RTCT_SET   0x30  /* Enable RT,CT */
#define HAL_UART_1_P1DIR_MASK       0x3f  /* PRIP0[1:0] */
#define HAL_UART_1_P1DIR_SET        0x40  /* HAL_UART_1 has priority on Port 0 */
#define HAL_UART_1_ALT1_RTS         0x08  /* UART0-ALT1 RTS on P0.3 */
#define HAL_UART_1_ALT2_RTS         0x20  /* UART0-ALT1 RTS on P1.5 */

#endif // UART 1

/* This macro clears RX_BYTE */
#define HAL_UART_CLEAR_RX_FLAG(p) (p == HAL_UART_PORT_0)?(U0CSR &= ~(HAL_UART_RX_BYTE)):(U1CSR &= ~(HAL_UART_RX_BYTE))

/* This macro clears TX_BYTE */
#define HAL_UART_CLEAR_TX_FLAG(p) (p == HAL_UART_PORT_0)?(U0CSR &= ~(HAL_UART_TX_BYTE)):( U1CSR &= ~(HAL_UART_TX_BYTE))

/* This gets a byte from BUF */
uint8 HAL_UART_GET_BYTE(uint8 port);

/* This puts a byte on BUF */
void HAL_UART_PUT_BYTE(uint8 port, uint8 ch);

/* This gets the status from particular port */
uint8 HAL_UART_GET_STATUS(uint8 port);

/* TRUE - indicates that DBUFF is empty and Tx buff is also empty */
static bool txIdleFlag;

/*********************************************************************
 * CONSTANTS
 */

/* Frame format bit positions */
#define UCR_STOP_BIT                 0x04
#define UCR_PARITY_ENABLE_BIT        0x08
#define UCR_PARITY_BIT               0x20
#define UCR_CHARACTER_SIZE_BIT       0x10
#define UCR_FLUSH_BIT                0x80

/* Stop Bits Table */
uint8 stopBitsTable [] = {0x00,   // One Stop Bit
                          0x04};  // Two Stop Bits

/* Parity Table */
uint8 parityTable [] =   {0x00,   // No Parity
                          0x20,   // Even Parity
                          0x00};  // Odd Parity

uint8 charSizeTable [] = {0x00,   // 8bits char
                          0x10};  // 9bits char - set manually because the bits are spreaded in 2 registers

/* Hardware Flow Control */
#define HAL_UART_FLOW_ENABLE        0x40

/* UART Port Status Constants */
#define HAL_UART_HW_ACTIVE          0x01  /* Indicates UART is physically present */

/* UART Control/Status Register UxCSR Constants */
#define HAL_UART_ACTIVE_BIT   0x01
#define HAL_UART_TX_BYTE      0x02
#define HAL_UART_RX_BYTE      0x04
#define HAL_UART_ERR_BIT      0x08
#define HAL_UART_FE_BIT       0x10
#define HAL_UART_SLAVE_BIT    0x20
#define HAL_UART_RE_BIT       0x40
#define HAL_UART_MODE_BIT     0x80

/* UART Transmit Interrupt Enable Bits */
#define UTX0IE  0x04
#define UTX1IE  0x08

/* UART Tx Flag bits */
#define UTX0IF  0x02
#define UTX1IF  0x04

/* UART Tx flag array bit */
#define UTXxIF(x) UTXxIF_Bit[x]
uint8 UTXxIF_Bit[] = {UTX0IF, UTX1IF};

/*********************************************************************
 * GLOBAL VARIABLES
 */
halUARTCfg_t  halUartRecord[HAL_UART_PORT_MAX];

const uint8 CODE halUartBaudMTable[9] =
{
  59,   /* 1200 */
  59,   /* 2400 */
  59,   /* 4800 */
  59,   /* 9600 */
  59,   /* 19200 */
  0,    /* 31250 */
  59,   /* 38400 */
  216,  /* 57600 */
  216   /* 115200 */
};

const uint8 CODE halUartBaudETable[9] =
{
  5,    /* 1200 */
  6,    /* 2400 */
  7,    /* 4800 */
  8,    /* 9600 */
  9,    /* 19200 */
  10,   /* 31250 */
  10,   /* 38400 */
  10,   /* 57600 */
  11    /* 115200 */
};

/*********************************************************************
 * LOCAL FUNCTIONS
 */
/* UART Init Functions */
void halUartBufferStructureInit (uint8 port);
uint8 halUartAllocBuffers       (uint8 port);
void halUartSetBaudRate         (uint8 port, uint16 baudRate);
void halUartSetFrameFormat      (uint8 port, uint8 parityIndex, uint8 stopBitsIndex, uint8 charSizeIndex);
void Hal_UART_FlowControlInit   (uint8 port, bool enable );

/* UART Receive Functions */
void halUartRxEnable (uint8 port, bool enable);
void halUartRxIntEnable (uint8 port, bool enable);
bool halUartRxBufferIsFull (uint8 port);
void halUartRxInsertBuffer (uint8 port, uint8 ch);
void Hal_UART_RxProcessEvent (uint8 port, uint8 status);

/* UART Transmit Functions */
void halUartTxIntEnable (uint8 port, bool enable);
bool halUartTxBufferIsFull (uint8 port, uint16 length);
bool halUartTxBufferIsEmpty (uint8 port);
void halUartTxInsertBuffer (uint8 port, uint8 *pBuffer, uint16 length);
void halUartTxSendChar (uint8 port);
void Hal_UART_TxProcessEvent (uint8 port, uint8 status);

/* UART Other Functions */
void halUartSendCallBack (uint8 port, uint8 event);

/**************************************************************************************************
*
* UART API Functions
*
***************************************************************************************************/
/**************************************************************************************************
 * @fn      HalUARTInit()
 *
 * @brief   Initialize the UART
 *
 * @param   none
 *
 * @return  none
 **************************************************************************************************/
void HalUARTInit (void)
{
/*
*  HAL_UART_0 Initialization
*/
#if HAL_UART_0_HW_STATUS == HAL_UART_HW_ENABLE

  /* Set UART0 I/O location */
  PERCFG &= ~(HAL_UART_0_PERCFG_BIT);

  /* Enable Tx and Rx on P0 */
  P0SEL |= HAL_UART_0_P0SEL_SET;

  /* Make sure ADC doesnt use this */
  ADCCFG &= ~(HAL_UART_0_P0SEL_SET);

  /* Set direction - Rx as output, Tx as input */
  P2DIR &= ~(HAL_UART_0_P2DIR_BITS);

  /* Mode is UART Mode */
  U0CSR = 0x80;
  /*Flush it */
  U0UCR |= UCR_FLUSH_BIT;

#endif  /* HAL_UART_0_ENABLE */

/*
*  HAL_UART_1 Initialization
*/
#if HAL_UART_1_HW_STATUS == HAL_UART_HW_ENABLE

  /* Set UART1 I/O location */
  PERCFG |= HAL_UART_1_PERCFG_BIT;

  /* Enable Tx and Rx on P1 */
  P1SEL  |= HAL_UART_1_P1SEL_SET;

  /* Make sure ADC doesnt use this */
  ADCCFG &= ~(HAL_UART_1_P1SEL_SET);

  /* Set direction - Rx as output, Tx as input */
  P1DIR &= HAL_UART_1_P1DIR_MASK;
  P1DIR |= HAL_UART_1_P1DIR_SET;

  /* Mode is UART Mode */
  U1CSR = 0x80;
  /* Flush it */
  U1UCR |= UCR_FLUSH_BIT;

#endif  /* HAL_UART_1_ENABLE */

  halUartBufferStructureInit (0);
  halUartBufferStructureInit (1);
}

/**************************************************************************************************
 * @fn      HalUARTOpen()
 *
 * @brief   Open a port based on the configuration
 *
 * @param   port   - UART port
 *          config - contains configuration information
 *          cBack  - Call back function where events will be reported back
 *
 * @return  Status of the function call
 ***************************************************************************************************/
uint8 HalUARTOpen (uint8 port, halUARTCfg_t *config)
{
  /* Save important information */
  halUartRecord[port].configured           = config->configured;
  halUartRecord[port].baudRate             = config->baudRate;
  halUartRecord[port].flowControl          = config->flowControl;
  halUartRecord[port].flowControlThreshold = config->flowControlThreshold;
  halUartRecord[port].tx.maxBufSize        = config->tx.maxBufSize;
  halUartRecord[port].rx.maxBufSize        = config->rx.maxBufSize;
  halUartRecord[port].idleTimeout          = config->idleTimeout;
  halUartRecord[port].intEnable            = config->intEnable;
  halUartRecord[port].callBackFunc         = config->callBackFunc;

  /* allocate Tx and Rx buffers */
  if (!halUartAllocBuffers (port))
  {
    /* allocation failed */
    halUartRecord[port].configured = FALSE;
    return HAL_UART_MEM_FAIL;
  }

  /* Setup baudrate  */
  if (config->baudRate > HAL_UART_BR_115200)
    return HAL_UART_BAUDRATE_ERROR;
  else
    halUartSetBaudRate (port, config->baudRate);

  /* Init flow control */
  Hal_UART_FlowControlInit (port, config->flowControl);

  /* software flow control */
  if (config->flowControlThreshold > config->rx.maxBufSize)
  {
    halUartRecord[port].flowControlThreshold = 0;
  }
  else
  {
    halUartRecord[port].flowControlThreshold = config->flowControlThreshold;
  }

  /* Set frame format */
  halUartSetFrameFormat (port, HAL_UART_NO_PARITY, HAL_UART_ONE_STOP_BIT, HAL_UART_8_BITS_PER_CHAR);

  /* Enable/Disable interrupts */
  halUartRxIntEnable (port, config->intEnable);
  halUartTxIntEnable (port, FALSE);

  /* Enable receiver and initialize status */
  halUartRxEnable (port, TRUE);

  HAL_UART_CLEAR_RX_FLAG(port);
  HAL_UART_CLEAR_TX_FLAG(port);

  return HAL_UART_SUCCESS;
}

/**************************************************************************************************
 * @fn      HalUARTClose()
 *
 * @brief   Close the UART
 *
 * @param   port - UART port
 *
 * @return  none
 ***************************************************************************************************/
void HalUARTClose ( uint8 port )
{
  /* Disable Tx and Rx */
  halUartRxEnable (port, FALSE);
  halUartRxIntEnable (port, FALSE);
  halUartTxIntEnable (port, FALSE);

  /* clear Rx,Tx complete status bit */
  HAL_UART_CLEAR_RX_FLAG(port);
  HAL_UART_CLEAR_TX_FLAG(port);

  /* Free Rx and Tx buffers */
  if (halUartRecord[port].configured)
  {
    osal_mem_free (halUartRecord[port].rx.pBuffer);
    osal_mem_free (halUartRecord[port].tx.pBuffer);

    halUartBufferStructureInit (port);   /* re-init buffers */
  }
}

/**************************************************************************************************
 * @fn      HalUARTRead()
 *
 * @brief   Read a buffer from the UART
 *
 * @param   port - UART port
 *          ppBuffer - pointer to a pointer that points to the data that will be read
 *          length - pointer to length of the buffer that returns
 *
 * @return  length of buffer that was read
 ***************************************************************************************************/
uint16 HalUARTRead (uint8 port, uint8 *pBuffer, uint16 length)
{
  uint16  bufLength = Hal_UART_RxBufLen(port);
  uint16  x=0;

  /* If port is not configured, do nothing */
  if (halUartRecord[port].configured)
  {

    /* limit length to what's available in buffer */
    if (length > bufLength)
    {
      length = bufLength;
    }

    if (pBuffer)
    {
      for (x=0; x < length; x++)
      {
        pBuffer[x] = halUartRecord[port].rx.pBuffer[halUartRecord[port].rx.bufferHead++];

        if ((halUartRecord[port].rx.bufferHead) == halUartRecord[port].rx.maxBufSize)
        {
          halUartRecord[port].rx.bufferHead = 0;
        }
      }
      return length;
    }
  }

  /* Read nothing if buffer is invalid or not configured */
  return 0;
}

/**************************************************************************************************
 * @fn      HalUARTWrite()
 *
 * @brief   Write a buffer to the UART
 *
 * @param   port    - UART port
 *          pBuffer - pointer to the buffer that will be written
 *          length  - length of
 *
 * @return  length of the buffer that was sent
 **************************************************************************************************/
uint16 HalUARTWrite (uint8 port, uint8 *pBuffer, uint16 length)
{
  /* Do nothing if not configured */
  if (halUartRecord[port].configured)
  {
    /* Check if there is room in the Tx buffer for all of the bytes */
    if (halUartTxBufferIsFull (port, length))
    {
      halUartSendCallBack (port, HAL_UART_TX_FULL) ;
    }
    else
    {
      if (halUartRecord[port].tx.pBuffer)
      {
        /* Put the new bytes in the buffer */
        halUartTxInsertBuffer (port, pBuffer, length);

        /* txFlag is clear because nothing has been transfered or it's the first time */
        if ((txIdleFlag == FALSE ) && (halUartRecord[port].intEnable))
        {
          /* Set txIdleFlag up */
          txIdleFlag = TRUE;

          /* Process the buffer since we just put something in there */
          Hal_UART_TxProcessEvent (port, HAL_UART_GET_STATUS(port));
        }

        return length;
      }
    }
  }

  /* Nothing is sent. Buffer is fulled or not configured */
  return 0;

}

/**************************************************************************************************
 * @fn      Hal_UARTPoll
 *
 * @brief   This routine simulate polling and has to be called by the main loop
 *
 * @param   void
 *
 * @return  void
 ***************************************************************************************************/
void HalUARTPoll( void )
{
  uint8 stat, ch;
  uint8 port = HAL_UART_PORT_MAX;


  /* cycle through ports */
  while (port--)
  {
    /* Only process ports that exist and are configured */
    if (halUartRecord[port].configured)
    {
      /* Check for Rx Buffer is full */
      if (halUartRxBufferIsFull (port))
      {
        halUartSendCallBack (port, HAL_UART_RX_FULL) ;
      }

      /* Check for Rx Buffer reaching threshold */
      if (halUartRecord[port].flowControlThreshold)
      {
        if (Hal_UART_RxBufLen(port) >= halUartRecord[port].rx.maxBufSize - halUartRecord[port].flowControlThreshold)
        {
          halUartSendCallBack (port, HAL_UART_RX_ABOUT_FULL) ;
        }
      }

      /* Check if Rx Buffer is idled */
      if ((halUartRecord[port].rxChRvdTime != 0)  && ((osal_GetSystemClock() - halUartRecord[port].rxChRvdTime) > halUartRecord[port].idleTimeout ))
      {
        halUartSendCallBack (port, HAL_UART_RX_TIMEOUT);
        halUartRecord[port].rxChRvdTime = 0;
      }

      /* If FC is required */
      if (halUartRecord[port].flowControl)
      {
        if (Hal_UART_RxBufLen(port) > halUartRecord[port].rx.maxBufSize / 2)
        {
          Hal_UART_FlowControlSet (port, HAL_UART_FLOW_OFF);
        }
        else
        {
          Hal_UART_FlowControlSet (port, HAL_UART_FLOW_ON);
        }
      }

      /* This part for POLLING. Basically check for RX_BYTE for Rx and check for Tx buffer's capacity for Tx */
      if (!halUartRecord[port].intEnable)
      {
        /* Make sure to get fresh/latest status */
        stat = HAL_UART_GET_STATUS (port);

        if (stat & HAL_UART_RX_BYTE)
        {
          /* Get a byte out of BUF, auto clear RX_BYTE */
          ch = HAL_UART_GET_BYTE(port);

          /* If Rx buffer is not full, then insert the new byte */
          if ((Hal_UART_RxBufLen (port) + 1) < halUartRecord[port].rx.maxBufSize)
            halUartRxInsertBuffer (port, ch);

        }

        /* Make sure to get fresh/latest status */
        stat = HAL_UART_GET_STATUS (port);

        /* Only process TX if Tx buffer is not empty, UART is not active and no pending RX_BYTE */
        if (!halUartTxBufferIsEmpty (port))
        {
          if (!(stat & HAL_UART_ACTIVE_BIT) && !(stat & HAL_UART_RX_BYTE))
          {
            Hal_UART_TxProcessEvent (port, stat);
          }
        }
      } /* Interrupt not enabled */
    } /* Configured */
  } /* While */
}

/**************************************************************************************************
 * @fn      HalUARTIoctl()
 *
 * @brief   This function is used to get/set a control
 *
 * @param   port   - UART port
 *          cmd    - Command
 *          pIoctl - control
 *
 * @return  none
 ***************************************************************************************************/
uint8 HalUARTIoctl (uint8 port, uint8 cmd, halUARTIoctl_t *pIoctl)
{
  return (HAL_UART_SUCCESS);
}


/**************************************************************************************************
*
* UART Init Functions
*
***************************************************************************************************/
/**************************************************************************************************
 * @fn      HalBufferStructureInit()
 *
 * @brief   Initialize the UART buffer structure elements
 *
 * @param   port - UART port
 *
 * @return  none
 **************************************************************************************************/
void halUartBufferStructureInit (uint8 port)
{
  halUartRecord[port].configured        = FALSE;
  halUartRecord[port].rx.bufferHead     = 0;
  halUartRecord[port].rx.bufferTail     = 0;
  halUartRecord[port].rx.pBuffer        = (uint8 *) NULL;
  halUartRecord[port].tx.bufferHead     = 0;
  halUartRecord[port].tx.bufferTail     = 0;
  halUartRecord[port].tx.pBuffer        = (uint8 *) NULL;
  halUartRecord[port].rxChRvdTime       = 0;
}

/**************************************************************************************************
 * @fn      Hal_UART_InitBuffer()
 *
 * @brief   Initialize a Rx and Tx buffer of particular port
 *
 * @param   port  - the port where the buffer will be created
 *
 * @return  Status of the function
 **************************************************************************************************/
uint8 halUartAllocBuffers (uint8 port)
{
  /* Allocate memory for Rx buffer */
  halUartRecord[port].rx.pBuffer = osal_mem_alloc (halUartRecord[port].rx.maxBufSize);
  halUartRecord[port].rx.bufferHead = 0;
  halUartRecord[port].rx.bufferTail = 0;

  /* Allocate memory for Tx buffer */
  halUartRecord[port].tx.pBuffer = osal_mem_alloc (halUartRecord[port].tx.maxBufSize);
  halUartRecord[port].tx.bufferHead = 0;
  halUartRecord[port].tx.bufferTail = 0;

  /* Validate buffers */
  if ((halUartRecord[port].rx.pBuffer) && (halUartRecord[port].tx.pBuffer))
  {
    return TRUE;
  }
  else
  {
    return FALSE;
  }
}
/**************************************************************************************************
 * @fn      HalSetBaudrate()
 *
 * @brief   set the baudrate
 *
 * @param   port  - the port where the baudrate will be set
 *          baudRate - the new baudrate
 *
 * @return  none
 **************************************************************************************************/
void halUartSetBaudRate (uint8 port, uint16 baudRate)
{
#if HAL_UART_0_HW_STATUS == HAL_UART_HW_ENABLE
  if (port == HAL_UART_PORT_0)
  {
    U0GCR  = halUartBaudETable[baudRate];
    U0BAUD = halUartBaudMTable[baudRate];
  }
#endif

#if HAL_UART_1_HW_STATUS == HAL_UART_HW_ENABLE
  if (port == HAL_UART_PORT_1)
  {
    U1GCR  = halUartBaudETable[baudRate];
    U1BAUD = halUartBaudMTable[baudRate];
  }
#endif
}

/**************************************************************************************************
 * @fn      Hal_UART_FlowControlInit
 *
 * @brief   Setup UART Rx flow control
 *
 * @param   port: serial port bit(s)
 *          on:   0=OFF, !0=ON
 *
 * @return  none
 *
 **************************************************************************************************/
void Hal_UART_FlowControlInit (uint8 port, bool enable)
{

#if HAL_UART_0_HW_STATUS == HAL_UART_HW_ENABLE
  if (port == HAL_UART_PORT_0)
  {
    if (enable)
    {
      P0DIR |= HAL_UART_0_ALT1_RTS;
      P0 &= ~(HAL_UART_0_ALT1_RTS);
    }
    else
    {
      P0DIR &= ~(HAL_UART_0_ALT1_RTS);
      P0 |= HAL_UART_0_ALT1_RTS;
    }
  }
#endif

#if HAL_UART_1_HW_STATUS == HAL_UART_HW_ENABLE
  if (port == HAL_UART_PORT_1)
  {
    if (enable)
    {
      P1DIR |= HAL_UART_1_ALT2_RTS;
      P1 &= ~(HAL_UART_1_ALT2_RTS);
    }
    else
    {
      P0DIR &= ~(HAL_UART_1_ALT2_RTS);
      P0 |= HAL_UART_1_ALT2_RTS;
    }
  }
#endif

}

/**************************************************************************************************
 * @fn      Hal_UART_SetFlowControl
 *
 * @brief   Set UART RTS ON/OFF
 *
 * @param   port: serial port bit(s)
 *          on:   0=OFF, !0=ON
 *
 * @return  none
 *
 **************************************************************************************************/
void Hal_UART_FlowControlSet (uint8 port, bool status)
{

#if HAL_UART_0_HW_STATUS == HAL_UART_HW_ENABLE
    if (status == HAL_UART_FLOW_ON)
      P0 &= ~(HAL_UART_0_ALT1_RTS);
    else
      P0 |= HAL_UART_0_ALT1_RTS;
#endif


#if HAL_UART_1_HW_STATUS == HAL_UART_HW_ENABLE
  if (HAL_UART_PORT_1)
  {
    if (status == HAL_UART_FLOW_ON)
      P1 &= ~(HAL_UART_1_ALT2_RTS);
    else
      P1 |= HAL_UART_1_ALT2_RTS;
  }
#endif

}


/**************************************************************************************************
 * @fn      HalUARTSetFrameFormat()
 *
 * @brief   set Data Direction Register with the bit mask value
 *
 * @param   port  - the port where the direction will be set
 *          parity - parity
 *          stopBits - stop bits
 *          charSize - character size
 *
 * @return  none
 **************************************************************************************************/
void halUartSetFrameFormat ( uint8 port, uint8 parityIndex, uint8 stopBitsIndex, uint8 charSizeIndex )
{
#if HAL_UART_0_HW_STATUS == HAL_UART_HW_ENABLE
  if (port == HAL_UART_PORT_0)
  {
    /* Clear relevant bits */
    U0UCR &= ~(UCR_STOP_BIT | UCR_PARITY_ENABLE_BIT | UCR_PARITY_BIT | UCR_CHARACTER_SIZE_BIT);

    /* Enable parity */
    if (parityIndex != HAL_UART_NO_PARITY)
      U0UCR |= UCR_PARITY_ENABLE_BIT;

    /* Setup bits */
    U0UCR |= parityTable[parityIndex] | stopBitsTable[stopBitsIndex] | charSizeTable[charSizeIndex];
  }
#endif

#if HAL_UART_1_HW_STATUS == HAL_UART_HW_ENABLE
  if (port == HAL_UART_PORT_1)
  {
    /* Clear relevant bits */
    U1UCR &= ~(UCR_STOP_BIT | UCR_PARITY_ENABLE_BIT | UCR_PARITY_BIT | UCR_CHARACTER_SIZE_BIT);

    /* Enable parity */
    if (parityIndex != HAL_UART_NO_PARITY)
      U1UCR |= UCR_PARITY_ENABLE_BIT;

    /* Setup bits */
    U1UCR |= parityTable[parityIndex] | stopBitsTable[stopBitsIndex] | charSizeTable[charSizeIndex];
  }
#endif
}

/**************************************************************************************************
*
* UART Receive Functions
*
***************************************************************************************************/
/**************************************************************************************************
 * @fn      halUartRxEnable()
 *
 * @brief   Enables UART receiver
 *
 * @param   port   - UART port
 *          enable - TRUE or FALSE, Enable/Disable
 *
 * @return  none
 **************************************************************************************************/
void halUartRxEnable (uint8 port, bool enable)
{
#if HAL_UART_0_HW_STATUS == HAL_UART_HW_ENABLE
  if (port == HAL_UART_PORT_0)
  {
    if (enable)
      U0CSR |= HAL_UART_RE_BIT;
    else
      U0CSR &= ~(HAL_UART_RE_BIT);
  }
#endif

#if HAL_UART_1_HW_STATUS == HAL_UART_HW_ENABLE
  if (port == HAL_UART_PORT_1)
  {
    if (enable)
      U1CSR |= HAL_UART_RE_BIT;
    else
      U1CSR &= ~(HAL_UART_RE_BIT);
  }
#endif
}

/**************************************************************************************************
 * @fn      halUartRxIntEnable()
 *
 * @brief   Enable or Disable Rx Interrupt
 *
 * @param   port   - UART port
 *          enable - TRUE/FALSE, enable/disable
 *
 * @return  none
 **************************************************************************************************/
void halUartRxIntEnable (uint8 port, bool enable)
{
#if HAL_UART_0_HW_STATUS == HAL_UART_HW_ENABLE
  if (port == HAL_UART_PORT_0)
  {
    URX0IE = enable;
  }
#endif

#if HAL_UART_1_HW_STATUS == HAL_UART_HW_ENABLE
  if (port == HAL_UART_PORT_1)
  {
    URX1IE = enable;
  }
#endif
}

/**************************************************************************************************
 * @fn      Hal_UART_RxBufLen()
 *
 * @brief   Calculate Rx Buffer length of a port (ie. the number of bytes in buffer).
 *
 * @param   port - UART port
 *
 * @return  length of current Rx Buffer
 **************************************************************************************************/
uint16 Hal_UART_RxBufLen (uint8 port)
{
  int16 length=0;

  length = halUartRecord[port].rx.bufferTail - halUartRecord[port].rx.bufferHead;
  if  (length < 0)
  {
    length += halUartRecord[port].rx.maxBufSize;
  }
  return ((uint16) length);
}

/**************************************************************************************************
 * @fn      halUartRxBufferIsFull
 *
 * @brief   Determines if Rx buffer is full.
 *
 * @param   port - UART port
 *
 * @return  TRUE or FALSE
 **************************************************************************************************/
bool halUartRxBufferIsFull (uint8 port)
{
  if ((Hal_UART_RxBufLen (port) + 1) >= halUartRecord[port].rx.maxBufSize)
  {
    return TRUE;
  }
  else
  {
    return FALSE;
  }
}

/**************************************************************************************************
 * @fn      halUartRxInsertBuffer
 *
 * @brief   Adds a byte to the Rx buffer
 *
 * @param   port - UART port
 * @param   ch   - byte to be inserted
 *
 * @return  void
 **************************************************************************************************/
void halUartRxInsertBuffer (uint8 port, uint8 ch)
{
  halUartRecord[port].rx.pBuffer[halUartRecord[port].rx.bufferTail++] = ch;

  if (halUartRecord[port].rx.bufferTail >= halUartRecord[port].rx.maxBufSize)
  {
    halUartRecord[port].rx.bufferTail = 0;
  }

  halUartRecord[port].rxChRvdTime = osal_GetSystemClock();
}

/**************************************************************************************************
 * @fn      Hal_UART_RxProcessEvent
 *
 * @brief   UART Rx interrupt service routine
 *
 * @param   port - UART port
 *
 * @return  void
 **************************************************************************************************/
void Hal_UART_RxProcessEvent (uint8 port, uint8 status)
{
  uint8 ch ;

  if (halUartRecord[port].configured)
  {
     /* Rx buffer has room */
    if ((!halUartRxBufferIsFull (port)) && status & HAL_UART_RX_BYTE)
    {
      /* read Rx byte out of receive register, RX_BYTE is auto-cleared */
      ch = HAL_UART_GET_BYTE(port);

      if (!(status & (HAL_UART_ERR_BIT | HAL_UART_FE_BIT)))
      {
        halUartRxInsertBuffer (port, ch);
      }
    }
  }
}

/**************************************************************************************************
*
* UART Transmit Functions
*
***************************************************************************************************/
/**************************************************************************************************
 * @fn      halUartTxIntEnable()
 *
 * @brief   Enable or Disable Tx Interrupt
 *
 * @param   port   - UART port
 *          enable - TRUE/FALSE, enable/disable
 *
 * @return  none
 **************************************************************************************************/
void halUartTxIntEnable (uint8 port, bool enable)
{
#if HAL_UART_0_HW_STATUS == HAL_UART_HW_ENABLE
  if (port == HAL_UART_PORT_0)
  {
    if (enable)
     IEN2 |= UTX0IE;
    else
     IEN2 &= ~(UTX0IE);
  }
#endif

#if HAL_UART_1_HW_STATUS == HAL_UART_HW_ENABLE
  if (port == HAL_UART_PORT_1)
  {
    if (enable)
     IEN2 |= UTX1IE;
    else
     IEN2 &= ~(UTX1IE);
  }
#endif
}

/**************************************************************************************************
 * @fn      Hal_UART_TxBufLen()
 *
 * @brief   Calculate Tx Buffer length of a port (ie. the number of bytes in buffer)
 *
 * @param   port - UART port
 *
 * @return  length of current Tx buffer
 **************************************************************************************************/
uint16 Hal_UART_TxBufLen (uint8 port)
{
  int16 length = 0;

  length = halUartRecord[port].tx.bufferTail - halUartRecord[port].tx.bufferHead;
  if  (length < 0)
  {
    length += halUartRecord[port].tx.maxBufSize;
  }
  return (uint16)length;
}

/**************************************************************************************************
 * @fn      halUartTxBufferIsFull
 *
 * @brief  Check if particular port is full or not if accepting 'length'
 *
 * @param   port - UART port
 *          length - the length of the new buffer that will be inserted
 *
 * @return  void
 **************************************************************************************************/
bool halUartTxBufferIsFull (uint8 port, uint16 length)
{
  if ((Hal_UART_TxBufLen (port) + length) >= halUartRecord[port].tx.maxBufSize)
  {
    return TRUE;
  }
  else
  {
    return FALSE;
  }
}

/**************************************************************************************************
 * @fn      halUartTxBufferIsEmpty
 *
 * @brief   Check if the particular port is empty or not
 *
 * @param   port - UART port
 *
 * @return  TRUE or FALSE
 **************************************************************************************************/
bool halUartTxBufferIsEmpty (uint8 port)
{
  if (halUartRecord[port].tx.bufferHead == halUartRecord[port].tx.bufferTail)
  {
    return TRUE;
  }
  else
  {
    return FALSE;
  }
}

/**************************************************************************************************
 * @fn      halUartTxInsertBuffer
 *
 * @brief   Adds 'length' bytes to Tx buffer
 *
 * @param   port    - UART port
 * @param   pBuffer - Buffer that will be inserted
 * @param   length  - Length of the buffer
 *
 * @return  void
 **************************************************************************************************/
void halUartTxInsertBuffer (uint8 port, uint8 *pBuffer, uint16 length)
{
  uint16  x=0;

  for (x=0; x < length; x++)
  {
    halUartRecord[port].tx.pBuffer[halUartRecord[port].tx.bufferTail++] = pBuffer[x];

    if (halUartRecord[port].tx.bufferTail == halUartRecord[port].tx.maxBufSize)
    {
      halUartRecord[port].tx.bufferTail = 0;
    }
  }
}

/**************************************************************************************************
 * @fn      halUartTxSendChar
 *
 * @brief   Writes a byte to Tx register and updates buffer pointers
 *
 * @param   port - UART port
 *          ch   - byte that will be sent
 *
 * @param   void
 *
 * @return  void
 **************************************************************************************************/
void halUartTxSendChar (uint8 port)
{
  /* Put a char on the BUF */
  HAL_UART_PUT_BYTE(port, halUartRecord[port].tx.pBuffer[halUartRecord[port].tx.bufferHead]);

  /* Update position on the buffer */
  halUartRecord[port].tx.bufferHead++;
  if (halUartRecord[port].tx.bufferHead >= halUartRecord[port].tx.maxBufSize)
  {
    halUartRecord[port].tx.bufferHead = 0;
  }

  /* Turn ON interrupt if needed. Has to be done after Head and Tail updated */
  if (halUartRecord[port].intEnable)
    halUartTxIntEnable (port, TRUE);

}

/***************************************************************************************************
 * @fn      Hal_UART_TxProcessEvent
 *
 * @brief  Process TX event either from interrupt or polling
 *
 * @param  port - port of the active UART
 *         status - status of "port"
 *
 * @return  void
 ***************************************************************************************************/
void Hal_UART_TxProcessEvent (uint8 port, uint8 status)
{
  if (halUartRecord[port].configured)
  {
    /* Clear UART TX interrupt flag */
    if (halUartRecord[port].intEnable)
     IRCON2 &= ~(UTXxIF(port));

    /* There's more data to send  */
    if (!halUartTxBufferIsEmpty (port))
    {
      /* Clear TX_BYTE status bit*/
      if (status & HAL_UART_TX_BYTE)
        HAL_UART_CLEAR_TX_FLAG(port);

      /* No activity, send out next char */
      halUartTxSendChar(port);
    }
    else
    {
      /* Turn off Tx if buffer is empty  and it's interrupt driven */
      if (halUartRecord[port].intEnable)
      {
        /* Disable Tx interrupt */
        halUartTxIntEnable (port, FALSE);

        /*
          There is nothing in the tx buffer to send.
          Last byte got sent out, turn off txFlag so Write() can prime DBUF next time
        */
        txIdleFlag = FALSE;
      }
    }
  }
}

/**************************************************************************************************
*
*                                       UART Other Functions
*
***************************************************************************************************/

/***************************************************************************************************
 * @fn      HalUARTSendCallBack
 *
 * @brief   Send Callback back to the caller
 *
 * @param   port - UART port
 *          event - event that causes the call back
 *
 * @return  None
 ***************************************************************************************************/
void halUartSendCallBack (uint8 port, uint8 event)
{
  if (halUartRecord[port].callBackFunc)
  {
    (halUartRecord[port].callBackFunc) (port, event);
  }
}

/**************************************************************************************************
 * @fn      HAL_UART_GET_STATUS()
 *
 * @brief   Initialize the UART
 *
 * @param   port - particular UART port
 *
 * @return  none
 **************************************************************************************************/
uint8 HAL_UART_GET_STATUS(uint8 port)
{
#if HAL_UART_0_HW_STATUS == HAL_UART_HW_ENABLE
  if (port == HAL_UART_PORT_0)
    return U0CSR;
#endif

#if HAL_UART_1_HW_STATUS == HAL_UART_HW_ENABLE
  if (port == HAL_UART_PORT_1)
    return U1CSR;
#endif
  return 0;
}

/**************************************************************************************************
 * @fn      HAL_UART_GET_BYTE()
 *
 * @brief   Initialize the UART
 *
 * @param   port - particular UART port
 *
 * @return  none
 **************************************************************************************************/
uint8 HAL_UART_GET_BYTE(uint8 port)
{
#if HAL_UART_0_HW_STATUS == HAL_UART_HW_ENABLE
  if (port == HAL_UART_PORT_0)
    return U0DBUF;
#endif

#if HAL_UART_1_HW_STATUS == HAL_UART_HW_ENABLE
  if (port == HAL_UART_PORT_1)
    return U1DBUF;
#endif
  return 0;
}

/**************************************************************************************************
 * @fn      HAL_UART_PUT_BYTE()
 *
 * @brief   Initialize the UART
 *
 * @param   port - particular UART port
 *          ch - byte that will be sent out
 *
 * @return  none
 **************************************************************************************************/
void HAL_UART_PUT_BYTE(uint8 port, uint8 ch)
{
#if HAL_UART_0_HW_STATUS == HAL_UART_HW_ENABLE
  if (port == HAL_UART_PORT_0)
    U0DBUF = ch;
#endif

#if HAL_UART_1_HW_STATUS == HAL_UART_HW_ENABLE
  if (port == HAL_UART_PORT_1)
    U1DBUF = ch;
#endif
}

/**************************************************************************************************
*
* UART Interrupt Functions
*
***************************************************************************************************/
/***************************************************************************************************
 * @fn      halUart0RxIsr
 *
 * @brief   UART0 Receive Interrupt
 *
 * @param   None
 *
 * @return  None
 ***************************************************************************************************/
#if HAL_UART_0_HW_STATUS == HAL_UART_HW_ENABLE
HAL_ISR_FUNCTION( halUart0RxIsr, URX0_VECTOR )
{
  Hal_UART_RxProcessEvent (HAL_UART_PORT_0, HAL_UART_GET_STATUS (HAL_UART_PORT_0));
}
#endif

/***************************************************************************************************
 * @fn      halUart1RxIsr
 *
 * @brief   UART1 Receive Interrupt
 *
 * @param   None
 *
 * @return  None
 ***************************************************************************************************/
#if HAL_UART_1_HW_STATUS == HAL_UART_HW_ENABLE
HAL_ISR_FUNCTION( halUart1RxIsr, URX1_VECTOR )
{
  Hal_UART_RxProcessEvent (HAL_UART_PORT_1, HAL_UART_GET_STATUS (HAL_UART_PORT_1));
}
#endif

/***************************************************************************************************
 * @fn      halUart0TxIsr
 *
 * @brief   UART0 Transmit Interrupt
 *
 * @param   None
 *
 * @return  None
 ***************************************************************************************************/
#if HAL_UART_0_HW_STATUS == HAL_UART_HW_ENABLE
HAL_ISR_FUNCTION( halUart0TxIsr, UTX0_VECTOR )
{
  Hal_UART_TxProcessEvent (HAL_UART_PORT_0, HAL_UART_GET_STATUS(HAL_UART_PORT_0));
}
#endif

/***************************************************************************************************
 * @fn      halUart1TxIsr
 *
 * @brief   UART1 Transmit Interrupt
 *
 * @param   None
 *
 * @return  None
 ***************************************************************************************************/
#if HAL_UART_1_HW_STATUS == HAL_UART_HW_ENABLE
HAL_ISR_FUNCTION( halUart1TxIsr, UTX1_VECTOR )
{
  Hal_UART_TxProcessEvent (HAL_UART_PORT_1, HAL_UART_GET_STATUS(HAL_UART_PORT_1));
}
#endif


