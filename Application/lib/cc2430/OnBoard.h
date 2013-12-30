#ifndef ONBOARD_H
#define ONBOARD_H

#include "hal_mcu.h"
#include "hal_sleep.h"

/*********************************************************************
 */
// Internal (MCU) RAM addresses
#define MCU_RAM_BEG 0x0100
#define MCU_RAM_END RAMEND
#define MCU_RAM_LEN (MCU_RAM_END - MCU_RAM_BEG + 1)

// Internal (MCU) heap size
#if !defined( INT_HEAP_LEN )
  #define INT_HEAP_LEN  1024  // 1.00K
#endif

// Memory Allocation Heap
#if defined( EXTERNAL_RAM )
  #define MAXMEMHEAP EXT_RAM_LEN   // Typically, 32K
#else
  #define MAXMEMHEAP INT_HEAP_LEN  // Typically, 0.70-1.50K
#endif

// Timer clock and power-saving definitions
#define TIMER_DECR_TIME    1  // 1ms - has to be matched with TC_OCC
#define RETUNE_THRESHOLD   1  // Threshold for power saving algorithm

/* OSAL timer defines */
#define TICK_TIME   1000   /* Timer per tick - in micro-sec */
#define TICK_COUNT  1
#define OSAL_TIMER  HAL_TIMER_3

#ifndef _WIN32
extern void _itoa(uint16 num, byte *buf, byte radix);
#endif

#ifndef RAMEND
#define RAMEND 0x1000
#endif

/* Tx and Rx buffer size defines used by SPIMgr.c */
#define SPI_THRESHOLD    5
#define SPI_TX_BUFF_MAX  170
#define SPI_RX_BUFF_MAX  32
#define SPI_IDLE_TIMEOUT 5

/* system restart and boot loader used from MTEL.c */
#define SystemReset()
#define BootLoader()



#ifdef LCD_SUPPORTED
  #define BUZZER_OFF  0
  #define BUZZER_ON   1
  #define BUZZER_BLIP 2
#endif

/* SERIAL PORT DEFINITIONS */
// Serial Ports ID Codes
#if defined (ZAPP_P1) || defined (ZTOOL_P1)
  #if defined (CC2430EB)
    #define SERIAL_PORT1 HAL_UART_PORT_0
  #elif defined (CC2430DB)
    #define SERIAL_PORT1 HAL_UART_PORT_1
  #endif
#else
  #undef SERIAL_PORT1
#endif

#if defined (ZAPP_P2) || defined (ZTOOL_P2)
  #if defined (CC2430EB)
    #define SERIAL_PORT2 HAL_UART_PORT_0
  #elif defined (CC2430DB)
    #define SERIAL_PORT2 HAL_UART_PORT_1
  #endif
#else
  #undef SERIAL_PORT2
#endif

// Application Serial Port Assignments
#if defined (ZAPP_P1)
  #define ZAPP_PORT SERIAL_PORT1
#elif defined (ZAPP_P2)
  #define ZAPP_PORT SERIAL_PORT2
#else
  #undef ZAPP_PORT
#endif
#if defined (ZTOOL_P1)
  #define ZTOOL_PORT SERIAL_PORT1
#elif defined (ZTOOL_P2)
  #define ZTOOL_PORT SERIAL_PORT2
#else
  #undef ZTOOL_PORT
#endif

/* sleep macros required by OSAL_PwrMgr.c */
#define SLEEP_DEEP                  0             /* value not used */
#define SLEEP_LITE                  0             /* value not used */
#define MIN_SLEEP_TIME              14            /* minimum time to sleep */
#define OSAL_SET_CPU_INTO_SLEEP(m)  halSleep(m)   /* interface to HAL sleep */




/* used by MTEL.c */
uint8 OnBoard_SendKeys( uint8 keys, uint8 state );

/*
 * Board specific random number generator
 */
extern uint16 Onboard_rand( void );

/*
 * Get elapsed timer clock counts
 *   reset: reset count register if TRUE
 */
extern uint32 TimerElapsed( void );


/*********************************************************************
 */

#endif
