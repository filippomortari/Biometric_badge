/**************************************************************************************************
    Filename:   msa.c
    Revised:    $Date: 2006-11-14 15:00:46 -0800 (Tue, 14 Nov 2006) $
    Revision:   $Revision: 12706 $

    Description: This file contains the sample application that can be use to test the functionality
                 of the MAC, HAL and low level.

    Copyright (c) 2005 by Figure 8 Wireless, Inc., A Chipcon Company.
    All Rights Reserved.  Permission to use, reproduce, copy, prepare
    derivative works, modify, distribute, perform, display or sell this
    software and/or its documentation for any purpose is prohibited
    without the express written consent of Figure 8 Wireless, Inc.
**************************************************************************************************/


/**************************************************************************************************

    Description:

                                KEY UP
                  - Beacon Enabled starting network

                                 |
          KEY LEFT               |      KEY RIGHT
                            -----+-----
                                 |
                                 |
                                KEY DOWN



**************************************************************************************************/


/**************************************************************************************************
 *                                           Includes
 **************************************************************************************************/

/* Hal Driver includes */
#include "hal_types.h"
#include "hal_key.h"
#include "hal_timer.h"
#include "hal_drivers.h"
#include "hal_led.h"
#include "hal_lcd.h"
#include "hal_adc.h"		//serve solo per completare gli ultimi byte del device ext addr r78,r148
#include "hal_uart.h"

/* OS includes */
#include "OSAL.h"
#include "OSAL_Tasks.h"
#include "OSAL_PwrMgr.h"

/* Application Includes */
#include "OnBoard.h"

/* MAC Application Interface */
#include "mac_api.h"

/* Application */
#include "msa.h"

/**************************************************************************************************
 *                                           Constant
 **************************************************************************************************/

#define MSA_DEV_SHORT_ADDR        0x0000        /* Device initial short address - This will change
												after association */

#define MSA_MAC_MAX_RESULTS       5             /* Maximun number of scan result that will be accepted*/
#define MSA_MAX_DEVICE_NUM        16            /* Maximun number of devices can associate with the
												coordinator */

#if defined (HAL_BOARD_CC2420DB)
  #define MSA_HAL_ADC_CHANNEL     HAL_ADC_CHANNEL_0             /* AVR - Channel 0 and Resolution 10 */
  #define MSA_HAL_ADC_RESOLUTION  HAL_ADC_RESOLUTION_10
#elif defined (HAL_BOARD_DZ1611) || defined (HAL_BOARD_DZ1612)
  #define MSA_HAL_ADC_CHANNEL     HAL_ADC_CHANNEL_0             /* DZ1611 and DZ1612 - Channel 0 and
  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  Resolution 12 */
  #define MSA_HAL_ADC_RESOLUTION  HAL_ADC_RESOLUTION_12
#else
  #define MSA_HAL_ADC_CHANNEL     HAL_ADC_CHANNEL_7             /* CC2430 EB & DB - Channel 7 and
  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  	  Resolution 14 */
  #define MSA_HAL_ADC_RESOLUTION  HAL_ADC_RESOLUTION_14
#endif

/* Size table for MAC structures */
const CODE uint8 msa_cbackSizeTable [] =
{
  0,                                   /* unused */
  sizeof(macMlmeAssociateInd_t),       /* MAC_MLME_ASSOCIATE_IND */
  sizeof(macMlmeAssociateCnf_t),       /* MAC_MLME_ASSOCIATE_CNF */
  sizeof(macMlmeDisassociateInd_t),    /* MAC_MLME_DISASSOCIATE_IND */
  sizeof(macMlmeDisassociateCnf_t),    /* MAC_MLME_DISASSOCIATE_CNF */
  sizeof(macMlmeBeaconNotifyInd_t),    /* MAC_MLME_BEACON_NOTIFY_IND */
  sizeof(macMlmeOrphanInd_t),          /* MAC_MLME_ORPHAN_IND */
  sizeof(macMlmeScanCnf_t),            /* MAC_MLME_SCAN_CNF */
  sizeof(macMlmeStartCnf_t),           /* MAC_MLME_START_CNF */
  sizeof(macMlmeSyncLossInd_t),        /* MAC_MLME_SYNC_LOSS_IND */
  sizeof(macMlmePollCnf_t),            /* MAC_MLME_POLL_CNF */
  sizeof(macMlmeCommStatusInd_t),      /* MAC_MLME_COMM_STATUS_IND */
  sizeof(macMcpsDataCnf_t),            /* MAC_MCPS_DATA_CNF */
  sizeof(macMcpsDataInd_t),            /* MAC_MCPS_DATA_IND */
  sizeof(macMcpsPurgeCnf_t),           /* MAC_MCPS_PURGE_CNF */
  sizeof(macEventHdr_t)                /* MAC_PWR_ON_CNF */
};

/*************************************************************************************************
 * STATUS STRINGS
 *************************************************************************************************/
#if defined ( HAL_LCD )
  const char PingStr[]         = "Ping Rcvd from";
  const char AssocCnfStr[]     = "Assoc Cnf";
  const char SuccessStr[]      = "Success";
  const char EndDeviceStr[]    = "EndDevice:";
  const char StartingStr[]     = "Starting";
  const char CoordStr[]  	   = "Started Coord";
  const char NetworkIDStr[]    = "Network ID:";
  const char RouterStr[]       = "Router:";
  const char OrphanRspStr[]    = "Orphan Response";
  const char SentStr[]         = "Sent";
  const char FailedStr[]       = "Failed";
  const char AssocRspFailStr[] = "Assoc Rsp fail";
  const char AssocIndStr[]     = "Assoc Ind";
  const char AssocCnfFailStr[] = "Assoc Cnf fail";
  const char EnergyLevelStr[]  = "Energy Level";
  const char ScanFailedStr[]   = "Scan Failed";
  const char ValueStr[]   	   = "Value";
  const char StartStr[]		   = "Starting";
  const char OpED[]	   		   = "Energy detection";
  const char MsgMacReceived[]  = "Msg from Mac:";
  const char MsgUartReceived[] = "Msg from Uart:";
  const char MsgSend[]	   	   = "Msg sent.";

#endif

/**************************************************************************************************
 *                                        Local Variables
 **************************************************************************************************/

/*
  Extended address of the device, for coordinator, it will be msa_ExtAddr1
  For any device, it will be msa_ExtAddr2 with the last 2 bytes from a ADC read
*/
sAddrExt_t    msa_ExtAddr; // working copy of extended address
sAddrExt_t    msa_ExtAddr1 = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88}; //coordinator extended address
sAddrExt_t    msa_ExtAddr2 = {0xA0, 0xB0, 0xC0, 0xD0, 0xE0, 0xF0, 0x00, 0x00}; //devices extended address

/* Coordinator and Device information */
uint16        msa_PanId = MSA_PAN_ID;
uint16        msa_CoordShortAddr = MSA_COORD_SHORT_ADDR;
uint16        msa_DevShortAddr   = MSA_DEV_SHORT_ADDR;
uint8		  msa_ChannelExpect = MAC_CHAN_11;

/*
  List of short addresses that the coordinator will use
  to assign to each the device that associates to it
*/
uint16        msa_DevShortAddrList[] = {0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037, 0x0038,
										0x0039, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047};

/* Current number of devices associated to the coordinator */
uint8         msa_NumOfDevices = 0;


/* TRUE and FALSE value */
bool          msa_MACTrue = TRUE;
bool          msa_MACFalse = FALSE;

/* Beacon payload, this is used to determine if the device is not zigbee device */
uint8         msa_BeaconPayload[] = {0x62, 0x61, 0x64, 0x67, 0x65};  // ascii value = "badge"
uint8         msa_BeaconPayloadLen = 5;

/* Contains pan descriptor results from scan */
macPanDesc_t  msa_PanDesc[MSA_MAC_MAX_RESULTS];

/* Contains energy detection after channel scan 18 elements, one for each channel*/
uint8		  msa_EnergyDetect[MSA_MAC_CHANNEL_ALL];

/*timer lcd value*/
char inttochar[16];  //array di caratteri da stampare
uint16 time = 1000;  //tempo di permanenza stampe lcd

/*energy index printEnergy()*/
uint8 energyIndex = 0;
uint8 currentCh = 11;

/* radice decimale per lcd*/
const uint8 radice10 = 10;

/*calcolo minimo canale con energia nel coordinator*/
uint8 chMaxEnergy;
uint8 chMaxEnergyValue;

/* flags used in the application */
bool          msa_IsCoordinator  = FALSE;   /* True if the device is started as a Pan Coordinate */
bool          msa_IsStarted      = FALSE;   /* True if the device started, either as Pan Coordinator
											or device */
bool          msa_IsCorrectBeacon = FALSE;   /* True if the beacon payload match with the predefined */
bool          msa_IsDirectMsg    = TRUE;   /* True if the messages will be sent as direct messages */
uint8         msa_State = MSA_IDLE_STATE;   /* Either IDLE state or SEND state */

/* Structure that used for association request */
macMlmeAssociateReq_t msa_AssociateReq;

/* Structure that used for association response */
macMlmeAssociateRsp_t msa_AssociateRsp;

/* Structure that used for disassociation*/
macMlmeDisassociateReq_t disReq;
sAddr_t address;


uint8* mymessage;

/* Structure that contains information about the device that associates with the coordinator */
typedef struct
{
	sAddrExt_t  extAddr;
	uint16 devShortAddr;
	uint8  isDirectMsg;
}msa_DeviceInfo_t;

/* Array contains the information of the devices */
static msa_DeviceInfo_t msa_DeviceRecord[MSA_MAX_DEVICE_NUM];

uint8 msa_SuperFrameOrder;
uint8 msa_BeaconOrder;

/* Task ID */
uint8 MSA_TaskId;

halUARTBufControl_t RxUART;
halUARTBufControl_t TxUART;

uint8 *RxUARTCurrentMsg;
uint16 RxUARTCurrentMsglenght;
uint8 *TxUARTCurrentMsg;
uint16 TxUARTCurrentMsglenght;


static uint8 index = MSA_MAX_DEVICE_NUM;

/**************************************************************************************************
 *                                     Local Function Prototypes
 **************************************************************************************************/

/* Setup routines */
void MSA_CoordinatorStartup();
void MSA_DeviceStartup();

/* MAC related routines, generano le strutture dati idonee a chiamare le ruotine MAC di pari tipo*/
void MSA_AssociateReq(void);
void MSA_AssociateRsp(macCbackEvent_t* pMsg);
void MSA_McpsDataReq(uint8* data, uint8 dataLength, bool directMsg, uint16 dstShortAddr);
void MSA_McpsPollReq(void);
void MSA_ScanReq(uint8 scanType, uint8 scanDuration);
void MSA_SyncReq(void);

/* UART routines */
void HalUARTCBack(uint8 port,uint8 event); //routine di callback per la traduzione di timeout
										   //uart in eventi per il gestore eventi di msa
void Msa_Uart_Received_Msg();
void Msa_Uart_Send_Msg();

/* Debug lcd */
void printenergy();
void _itoa(uint16 num, byte *buf, byte radix);

/* Controllo coerenza frames */
bool MSA_BeaconPayLoadCheck(uint8* pSdu);
bool MSA_DataCheck(uint8* data, uint8 dataLength);

/* controllo messaggi di sistema da uart*/
bool sysMsgfromUart();


/*
 *
 *	----------------  Common functions for End-Device & Coordinator  -------------------
 *
 */

/**************************************************************************************************
 *
 * @fn          MSA_Init
 *
 * @brief       Initialize the application
 *
 * @param       taskId - taskId of the task after it was added in the OSAL task queue
 *
 * @return      none
 *
 **************************************************************************************************/
void MSA_Init(uint8 taskId)
{

  //uint8 i;

  /* Initialize the task id */
  MSA_TaskId = taskId;

  /* initialize MAC features
  MAC_InitDevice();
  MAC_InitCoord();

  // Initialize MAC beacon
  MAC_InitBeaconDevice();
  MAC_InitBeaconCoord();*/

  if (MSA_ROLE==MSA_COORDINATOR)
  {
	  MAC_InitCoord();
	  MAC_InitBeaconCoord();
	  HalLcdWriteScreen((char*)StartStr,"coordinator");
  }
  else
  {
	  MAC_InitDevice();
	  MAC_InitBeaconDevice();
	  HalLcdWriteScreen((char*)StartStr,"End-Device");
  }

  /* Reset the MAC */
  MAC_MlmeResetReq(TRUE);

  msa_BeaconOrder = MSA_MAC_BEACON_ORDER;
  msa_SuperFrameOrder = MSA_MAC_SUPERFRAME_ORDER;
  HalLedBlink (HAL_LED_4, 0, 40, 500);

  //HalLcdWriteStringValue("UART status",status,10, HAL_LCD_LINE_1);
}

/**************************************************************************************************
 *
 * @fn          MSA_ProcessEvent
 *
 * @brief       This routine handles events
 *
 * @param       taskId - ID of the application task when it registered with the OSAL
 *              events - Events for this task
 *
 * @return      16bit - Unprocessed events
 *
 **************************************************************************************************/
uint16 MSA_ProcessEvent(uint8 taskId, uint16 events)
{
  uint8* pMsg;
  macCbackEvent_t* pData;



  if (events & SYS_EVENT_MSG)
  {
    while ((pMsg = osal_msg_receive(MSA_TaskId)) != NULL)

    {
      switch ( *pMsg )
      {

      	  case MAC_MLME_SCAN_CNF:

          /* coordinator & end device*/

                pData = (macCbackEvent_t *) pMsg;

                if((pData->scanCnf.scanType == MAC_SCAN_ED)&&
                		(pData->scanCnf.hdr.status == MAC_SUCCESS))
                {
              	  /*coordinator retrieve expected channel with lowest noise*/
                	chMaxEnergyValue = msa_EnergyDetect[0];
                	chMaxEnergy = 0;

                	for(uint8 i = 1; i<MSA_MAC_CHANNEL_ALL ; i++)
                	{

                		if (chMaxEnergyValue > msa_EnergyDetect[i])
                		{

                			chMaxEnergyValue = msa_EnergyDetect[i];
                			chMaxEnergy = i;
                		}
                	}

					msa_ChannelExpect = chMaxEnergy + 11;
					MSA_ScanReq(MAC_SCAN_ACTIVE,3);
					printenergy();
                }

                /* If there is no other on the channel or no other with sampleBeacon start as
                 * coordinator */
                else if ((pData->scanCnf.scanType == MAC_SCAN_ACTIVE) &&
                		(pData->scanCnf.resultListSize == 0) &&
                		(pData->scanCnf.hdr.status == MAC_NO_BEACON))
                {
                	/* coordinator startup */
                    MSA_CoordinatorStartup();
                }
                /* If other coordinators found check PAN ID difference before
                 * starting as coordinator
                 *
                 */
                else if ((pData->scanCnf.scanType == MAC_SCAN_ACTIVE) &&
                                		(pData->scanCnf.resultListSize != 0))
                {
                	bool flag = false;
                	for (uint8 i=0;i<(pData->scanCnf.resultListSize);i++)
                	{
                		if (msa_PanId == pData->scanCnf.result.pPanDescriptor[i].coordPanId){
                			flag=true;
                		}
                	}
                	if (flag)
                	{
                		uint8 scActive[30]="$Can't start, PAN ID conflict ";
                		scActive[29]=0xA;
                		HalUARTWrite(HAL_UART_PORT,scActive,30);
                		osal_stop_timer(PRINT_NEXT_ENERGY);
                	}
                	else
                	{
                		uint8 scActive[25]="$Other coordinators found";
                		scActive[24]=0xA;
                		HalUARTWrite(HAL_UART_PORT,scActive,25);
                		MSA_CoordinatorStartup();
                	}
                }
                else if ( (pData->scanCnf.scanType == MAC_SCAN_PASSIVE) &&
                		(pData->scanCnf.hdr.status == MAC_SUCCESS))
                {
                  /* end device */

                  /* Start the device up as beacon enabled or not */
                  MSA_DeviceStartup();
                  /* Call Associate Req */
                  MSA_AssociateReq();

                }
          break;

      	  case MAC_MLME_START_CNF:
      	          /*
      	           * Coordinator,
      	           * Retrieve the message
      	           */
      	          pData = (macCbackEvent_t *) pMsg;

      	          /* Set some indicator for the Coordinator */
      	          if ((!msa_IsStarted) && (pData->startCnf.hdr.status == MAC_SUCCESS))
      	          {
      	        	//HalLedSet (HAL_LED_4, HAL_LED_MODE_ON);
      	            msa_IsStarted = TRUE;
      	            msa_IsCoordinator = TRUE;
      	          }
      	  break;

      	  case MAC_MLME_BEACON_NOTIFY_IND:
      	          /*
      	           * End device
      	           * Retrieve the message
      	           */
      	          pData = (macCbackEvent_t *) pMsg;
      	          /* Check for correct beacon payload */
      	          if (!msa_IsStarted && (pData->beaconNotifyInd.sduLength == msa_BeaconPayloadLen))
      	          {
      	        	msa_IsCorrectBeacon = MSA_BeaconPayLoadCheck(pData->beaconNotifyInd.pSdu);

      	            /* If it's the correct beacon payload, retrieve the data for association req */
      	            if (msa_IsCorrectBeacon)
      	            {

      	              msa_AssociateReq.logicalChannel = pData->beaconNotifyInd.pPanDesc->logicalChannel;
      	              msa_ChannelExpect = pData->beaconNotifyInd.pPanDesc->logicalChannel;
      	              msa_AssociateReq.coordAddress.addrMode = SADDR_MODE_SHORT;
      	              msa_AssociateReq.coordAddress.addr.shortAddr =
      	            		  pData->beaconNotifyInd.pPanDesc->coordAddress.addr.shortAddr;
      	              msa_AssociateReq.coordPanId = pData->beaconNotifyInd.pPanDesc->coordPanId;
      	              if (msa_IsDirectMsg)
      	                msa_AssociateReq.capabilityInformation = MAC_CAPABLE_ALLOC_ADDR |
      	                											MAC_CAPABLE_RX_ON_IDLE;
      	              else
      	              	  msa_AssociateReq.capabilityInformation = MAC_CAPABLE_ALLOC_ADDR;
      	              msa_AssociateReq.sec.securityLevel = MAC_SEC_LEVEL_NONE;

      	              /* Retrieve beacon order and superframe order from the beacon */
      	              msa_BeaconOrder =
      	            	MAC_SFS_BEACON_ORDER(pData->beaconNotifyInd.pPanDesc->superframeSpec);
      	              msa_SuperFrameOrder =
      	            	MAC_SFS_SUPERFRAME_ORDER(pData->beaconNotifyInd.pPanDesc->superframeSpec);

      	              HalLcdWriteStringValue("Beacon on ch:",msa_ChannelExpect,10,1);
      	            }
      	          }

          break;

      	  case MAC_MLME_ASSOCIATE_IND:
      		  /* coordinator */

      		  MSA_AssociateRsp((macCbackEvent_t *) pMsg);

          break;

        case MAC_MLME_ASSOCIATE_CNF:
          /*
           * End device
           * Retrieve the message
           *
           */
          pData = (macCbackEvent_t *) pMsg;

          if ((!msa_IsStarted) && (pData->associateCnf.hdr.status == MAC_SUCCESS))
          {

            msa_IsStarted = TRUE;

            /* Retrieve MAC_SHORT_ADDRESS */

            msa_DevShortAddr = pData->associateCnf.assocShortAddress;

            /* Setup MAC_SHORT_ADDRESS - obtained from Association */
            MAC_MlmeSetReq(MAC_SHORT_ADDRESS, &msa_DevShortAddr);

            HalLedBlink(HAL_LED_4, 0, 90, 1000);

            HalLcdWriteString("Short address :",1);

            HalLcdWriteValue(msa_DevShortAddr-48,10,2);

            HalLedSet(HAL_LED_4,HAL_LED_MODE_ON);

            uint8 devShAddr[18]="$Short address:   ";
            devShAddr[16]= (uint8) msa_DevShortAddr;
            devShAddr[17]= 0xA;
            HalUARTWrite  (HAL_UART_PORT,devShAddr,18);

          }
          break;

        case MAC_MLME_COMM_STATUS_IND:
          break;

        case MSA_UART_RX_TIMEOUT:
        	Msa_Uart_Received_Msg();
          break;

        case MSA_SEND_EVENT:

		  if (msa_State == MSA_SEND_STATE)
		  {
			MSA_McpsDataReq((uint8*)RxUARTCurrentMsg,
								(uint8)RxUARTCurrentMsglenght,
								TRUE,
								RxUARTCurrentMsg[0] );

			//msa_State = MSA_IDLE_STATE;
			//HalLedSet (HAL_LED_1, HAL_LED_MODE_BLINK);
		  }

          break;

        case MAC_MCPS_DATA_CNF:
          pData = (macCbackEvent_t *) pMsg;

          /* If last transmission completed, ready to send the next one
           *
           * Sul completed c'è da discutere
           */
          if ((pData->dataCnf.hdr.status == MAC_CHANNEL_ACCESS_FAILURE) ||
              (pData->dataCnf.hdr.status == MAC_NO_ACK))
          {

        	  // to do...
        	  char errorUart[] = "$MAC Error: bad address, access failure or No Ack ";
        	  errorUart[49] = 0xA;
        	  uint8 lenght = HalUARTWrite(HAL_UART_PORT,(uint8*)errorUart, 50);

          }

		  /* elimino dalla memoria RxUARTCurrentMsg una volta cerato il pacchetto MAC*/
		  osal_mem_free(RxUARTCurrentMsg);

		  msa_State = MSA_IDLE_STATE;



          osal_msg_deallocate((uint8 *) pData->dataCnf.pDataReq);
          break;

        case MAC_MCPS_DATA_IND:
          pData = (macCbackEvent_t*)pMsg;

          /* MSA_DataCheck is always TRUE for now */
          if (MSA_DataCheck ( pData->dataInd.msdu.p, pData->dataInd.msdu.len ))
          {

			//giro il messaggio ricevuto da MAC verso la uart

			// calcolo la lunghezza del pacchetto ricevuto via MAC

			TxUARTCurrentMsglenght = pData->dataInd.msdu.len;

			/*
			 *  Alloco la memoria che mi serve per contenere il messaggio in arrivo;
			 *  cast esplicito (uint8 *) necessario perchè il return (void *)
			 *  della funzione osal_mem_alloc()
			 *	deve avere sempre un cast esplicito.
			 *
			 */
			TxUARTCurrentMsg =(uint8 *) osal_mem_alloc(TxUARTCurrentMsglenght);

			/*
			 * Leggo e memorizzo il messaggio nella memoria allocata
			 * in precedenza, mediante il puntatore a variabile TxUARTCurrentMsg
			 */
			for(uint8 i = 0; i < TxUARTCurrentMsglenght;i++){
				TxUARTCurrentMsg[i] = pData->dataInd.msdu.p[i];
			}

			TxUARTCurrentMsg[0] = pData->dataInd.mac.srcAddr.addr.shortAddr;
			/* aggiunta di alessio
			if (TxUARTCurrentMsg[1]==0x3E)
				{
				TxUARTCurrentMsg[1]=0x3C;
				}
			*/
			/*send message via uart*/
			Msa_Uart_Send_Msg();

			/*La funzione HalUARTWrite fa una copia reale del msg nel buffer
			 * quindi posso deallocare il msg appena creato*/
			osal_mem_free(TxUARTCurrentMsg);

          }

          break;

        case MAC_MLME_DISASSOCIATE_CNF:
        	pData = (macCbackEvent_t*)pMsg;
        	if (MSA_ROLE==MSA_COORDINATOR)
			  {
        		HalLcdWriteString("Disass. done",1);
        		HalLcdWriteString("",2);
				uint8 disass[42]="$Disass done for device with ShortAddr:   ";
				//for (uint8 i = 0; i < SADDR_EXT_LEN;i++){

					unsigned char add[2] = "  ";
					_itoa(pData->disassociateCnf.deviceAddress.addr.shortAddr-48,add,16);
					disass[39] = add[0];
					disass[40] = add[1];
				//}
				disass[41]=0xA;
				HalUARTWrite(HAL_UART_PORT,disass,42);
			  }
			else if (MSA_ROLE==MSA_END_DEVICE)
			{
				uint8 disassEndDev[48]="$Disass. from coordinator, now you can PowerOff ";
				disassEndDev[47]=0xA;
				HalUARTWrite(HAL_UART_PORT,disassEndDev,48);
				HalLcdWriteString("Now you can",1);
				HalLcdWriteString("safely powerOff",2);
				HalLedSet(HAL_LED_4,HAL_LED_MODE_OFF);

				MAC_MlmeResetReq(true);
				msa_IsStarted = false;
			}

        	break;

        case MAC_MLME_DISASSOCIATE_IND:
        	pData = (macCbackEvent_t*)pMsg;
        	if (MSA_ROLE==MSA_COORDINATOR)
			  {
        		HalLcdWriteString("Disass. done.",1);
        		HalLcdWriteString("",2);
        		uint8 disass[33]="$Disass Extaddr:                 ";
        		uint8 indexAddr = 16;
        		for (uint8 i =0; i < SADDR_EXT_LEN;i++){

        			unsigned char addr[2] = "  ";
        			_itoa(pData->disassociateInd.deviceAddress[i],addr,16);
        			disass[indexAddr] = addr[0];
        			indexAddr ++;
        			disass[indexAddr] = addr[1];
					indexAddr ++;
        		}
        		disass[32]=0xA;
        		HalUARTWrite(HAL_UART_PORT,disass,33);

			  }
			else if (MSA_ROLE==MSA_END_DEVICE)
			{
				uint8 disassEndDev[48]="$Disass. from coordinator, now you can PowerOff ";
				disassEndDev[47]=0xA;
				HalUARTWrite(HAL_UART_PORT,disassEndDev,48);
				HalLcdWriteString("Now you can",1);
				HalLcdWriteString("safely powerOff",2);
				HalLedSet(HAL_LED_4,HAL_LED_MODE_OFF);

				MAC_MlmeResetReq(true);
				msa_IsStarted = false;
			}


        break;

        case MSA_DISASSOCIATE:

        	if (MSA_ROLE==MSA_COORDINATOR)
        	  {
        		address.addr.shortAddr =  pMsg[1];
        	  }
        	else if (MSA_ROLE==MSA_END_DEVICE)
        	{
        		address.addr.shortAddr = msa_CoordShortAddr;
            }
        	address.addrMode = SADDR_MODE_SHORT;

        	disReq.txIndirect = false;

        	if (MSA_ROLE==MSA_COORDINATOR)
        	  {
        		disReq.disassociateReason = MAC_DISASSOC_COORD;
            	HalLcdWriteString("Disass. sent to",1);
            	HalLcdWriteStringValue("address :",address.addr.shortAddr - 48,10,2);
            	uint8 disassCoord[27]="$Disass. Req. to:          ";
            	unsigned char ad[2]="  ";
            	_itoa(address.addr.shortAddr-48, ad, 10);
            	disassCoord[24] = ad[0];
            	disassCoord[25] = ad[1];
				disassCoord[26] = 0xA;
				HalUARTWrite(HAL_UART_PORT,disassCoord,27);
        	  }
        	else if (MSA_ROLE==MSA_END_DEVICE)
        	{
        		disReq.disassociateReason = MAC_DISASSOC_DEVICE;
        		HalLcdWriteString("Disass. sent to",1);
        		HalLcdWriteString("Coordinator",2);


        	  }
        	disReq.devicePanId = msa_PanId;
        	disReq.deviceAddress = address;

        	MAC_MlmeDisassociateReq(&disReq);

        break;


      }

      /* Deallocate */
      osal_msg_deallocate((uint8 *) pMsg);
    }

    return events ^ SYS_EVENT_MSG;
  }

  if (events & PRINT_NEXT_ENERGY){

	  printenergy();
	  return events ^ PRINT_NEXT_ENERGY;
  }

  return 0;

}


/**************************************************************************************************
 *
 * @fn          Msa_Uart_Received_Msg
 *
 * @brief       This routine handles message from UART Rx buffer
 *
 * @param
 *
 * @return
 *
 **************************************************************************************************/
void Msa_Uart_Received_Msg(void){

	if(msa_State == MSA_SEND_STATE){

		//To do...

	}
	else{
		/*
		 *  Forward message from UART to MAC Radio channel
		 *
		 *  Alloco la memoria che mi serve per contenere il messaggio in arrivo;
		 *  cast esplicito (uint8 *) necessario perchè il return (void *)
		 *  della funzione osal_mem_alloc()
		 *	deve avere sempre un cast esplicito.
		 *
		 *  Leggo e memorizzo il messaggio nella memoria allocata
		 *  in precedenza, mediante il puntatore a variabile RxUARTCurrentMsg
		 *
		 */

		RxUARTCurrentMsglenght = Hal_UART_RxBufLen(HAL_UART_PORT);
		RxUARTCurrentMsg =(uint8 *) osal_mem_alloc(RxUARTCurrentMsglenght);

		uint8 i =HalUARTRead(HAL_UART_PORT,RxUARTCurrentMsg,RxUARTCurrentMsglenght);

		if(!sysMsgfromUart())
		{
			msa_State = MSA_SEND_STATE;

			mymessage = (uint8*) osal_msg_allocate(sizeof (uint8));
			if (mymessage!= NULL){
				*mymessage = MSA_SEND_EVENT;
				osal_msg_send(MSA_TaskId,mymessage);
			}
		}
		else{
			HalLcdWriteString("Disassociate Req",1);
			HalLcdWriteStringValue("Address:",RxUARTCurrentMsg[1],10,2);
			if(RxUARTCurrentMsg[1] == 68){
				mymessage = (uint8*) osal_msg_allocate(2*sizeof (uint8));
				if (mymessage!= NULL){
					mymessage[0] = MSA_DISASSOCIATE;
					mymessage[1] = RxUARTCurrentMsg[2];
					HalLcdWriteString("Disass Osal Msg",1);
					HalLcdWriteStringValue("Address",RxUARTCurrentMsg[2],10,2);
					osal_msg_send(MSA_TaskId,mymessage);
				}
			}

			/* elimino dalla memoria RxUARTCurrentMsg una volta cerato il pacchetto MAC*/
			osal_mem_free(RxUARTCurrentMsg);

		}
	}
}

/**************************************************************************************************
 *
 * @fn          sysMsgfromUart
 *
 * @brief       This routine handles message from UART,
 * 				check the first Byte if it's a system message
 *
 * @param
 *
 * @return		true if the first byte is 0x24
 *
 **************************************************************************************************/
bool sysMsgfromUart(){
	if(RxUARTCurrentMsg[0] == 0x24){
		return true;
	}
	return false;
}

/**************************************************************************************************
 *
 * @fn          Msa_Uart_Send_Msg
 *
 * @brief       This routine handles message to UART Tx buffer
 *
 * @param
 *
 * @return
 *
 **************************************************************************************************/
void Msa_Uart_Send_Msg(){

	/*invio alla uart il payload del msg MAC ricevuto*/
	uint8 lenght = HalUARTWrite(HAL_UART_PORT,TxUARTCurrentMsg,TxUARTCurrentMsglenght);

	//HalLcdWriteStringValue("Send lenght",lenght,10,2);
}

/**************************************************************************************************
 *
 * @fn          MAC_CbackEvent
 *
 * @brief       This callback function sends MAC events to the application.
 *              The application must implement this function.  A typical
 *              implementation of this function would allocate an OSAL message,
 *              copy the event parameters to the message, and send the message
 *              to the application's OSAL event handler.  This function may be
 *              executed from task or interrupt context and therefore must
 *              be reentrant.
 *
 * @param       pData - Pointer to parameters structure.
 *
 * @return      None.
 *
 **************************************************************************************************/
void MAC_CbackEvent(macCbackEvent_t *pData)
{

  macCbackEvent_t *pMsg = NULL;

  uint8 len = msa_cbackSizeTable[pData->hdr.event];

  switch (pData->hdr.event)
  {
      case MAC_MLME_BEACON_NOTIFY_IND:

      len += sizeof(macPanDesc_t) + pData->beaconNotifyInd.sduLength +
             MAC_PEND_FIELDS_LEN(pData->beaconNotifyInd.pendAddrSpec);
      if ((pMsg = (macCbackEvent_t *) osal_msg_allocate(len)) != NULL)
      {
        /* Copy data over and pass them up */
        osal_memcpy(pMsg, pData, sizeof(macMlmeBeaconNotifyInd_t));
        pMsg->beaconNotifyInd.pPanDesc =
        		(macPanDesc_t *) ((uint8 *) pMsg + sizeof(macMlmeBeaconNotifyInd_t));
        osal_memcpy(pMsg->beaconNotifyInd.pPanDesc, pData->beaconNotifyInd.pPanDesc,
        		sizeof(macPanDesc_t));
        pMsg->beaconNotifyInd.pSdu = (uint8 *) (pMsg->beaconNotifyInd.pPanDesc + 1);
        osal_memcpy(pMsg->beaconNotifyInd.pSdu, pData->beaconNotifyInd.pSdu,
        		pData->beaconNotifyInd.sduLength);
      }
      break;

    case MAC_MCPS_DATA_IND:
      pMsg = pData;
      break;

    default:
      if ((pMsg = (macCbackEvent_t *) osal_msg_allocate(len)) != NULL)
      {
        osal_memcpy(pMsg, pData, len);
      }
      break;
  }

  if (pMsg != NULL)
  {
    osal_msg_send(MSA_TaskId, (byte *) pMsg);
  }
}

/**************************************************************************************************
 *
 * @fn      MacSampelApp_ScanReq()
 *
 * @brief   Performs active scan on specified channel
 *
 * @param   None
 *
 * @return  None
 *
 **************************************************************************************************/
void MSA_ScanReq(uint8 scanType, uint8 scanDuration)
{
  macMlmeScanReq_t scanReq;

  /* Fill in information for scan request structure */
  /*  channel mask  */
  //

  scanReq.scanType = scanType;
  scanReq.scanDuration = scanDuration;

  /*end device scan*/
  if(scanType == MAC_SCAN_PASSIVE){

	  scanReq.scanChannels = (uint32) MSA_MAC_CHANNEL_ALL_MASK;
	  scanReq.maxResults = MSA_MAC_MAX_RESULTS;
  	  scanReq.result.pPanDescriptor = msa_PanDesc;
  	  HalLcdWriteScreen((char*)StartStr,"passive scan");
  	  uint8 lineEndD[23]="$Starting Passive Scan ";
      lineEndD[22]=0xA;
      HalUARTWrite(HAL_UART_PORT,lineEndD,23);
  }

  /*coordinator energy detect, first thing to do to act as a coordinator */
  else if (scanType == MAC_SCAN_ED){
	  HalLcdWriteScreen((char*)StartStr,"energy detection");
	  uint8 lineCoord[27]="$Starting Energy Detection ";
	  lineCoord[26]=0xA;
	  HalUARTWrite(HAL_UART_PORT,lineCoord,27);
	  /* channel mask modified  */
	  scanReq.scanChannels = (uint32) MSA_MAC_CHANNEL_ALL_MASK;
	  scanReq.maxResults = MSA_MAC_CHANNEL_ALL;
  	  scanReq.result.pEnergyDetect = msa_EnergyDetect;
  }

  /*coordinator scan on lowest-noise channel, before becoming coordinator*/
  else if(scanType == MAC_SCAN_ACTIVE){


	  scanReq.scanChannels = (uint32) 1 << msa_ChannelExpect;
	  scanReq.maxResults = MSA_MAC_MAX_RESULTS;
  	  scanReq.result.pPanDescriptor = msa_PanDesc;
  }

  /* Call scan request */
  MAC_MlmeScanReq(&scanReq);
}

/**************************************************************************************************
 *
 * @fn      MSA_HandleKeys
 *
 * @brief   Callback service for keys
 *
 * @param   keys  - keys that were pressed
 *          state - shifted
 *
 * @return  void
 *
 **************************************************************************************************/
void MSA_HandleKeys(uint8 keys, uint8 shift)
{

  if ( keys & HAL_KEY_SW_1 )
  {
    /* Start the device as a direct message device and beacon disabled*/
    if (!msa_IsStarted)
    {
      if (MSA_ROLE == MSA_COORDINATOR){
    	  /*   beacon enabled   */
        //MSA_ScanReq(MAC_SCAN_PASSIVE, MSA_MAC_BEACON_ORDER + 1);
    	  /* scan energy detect circa 9 superframe time duration */
    	  char starting[22] = "$Starting Coordinator ";
    	  starting[21] = 0xA;
    	  HalUARTWrite(HAL_UART_PORT,(uint8*)starting,22);
    	  MSA_ScanReq(MAC_SCAN_ED, 3);
      }
      else if (MSA_ROLE == MSA_END_DEVICE){
    	  char starting[22] = "$Starting End Device  ";
    	  starting[21] = 0xA;
    	  HalUARTWrite(HAL_UART_PORT,(uint8*)starting,22);
    	  MSA_ScanReq(MAC_SCAN_PASSIVE, 5);
      }
    }
  }

  if ( keys & HAL_KEY_SW_2 )
  {
  }

  if ( keys & HAL_KEY_SW_3 )
  {
  }

  if ( keys & HAL_KEY_SW_4 )
  {

  }

  if ( keys & HAL_KEY_SW_5 )
  {
  }

  /*
	if ( keys & HAL_KEY_SW_6 )
	{
	}
  */

}



/**************************************************************************************************
 *
 * @fn      printenergy
 *
 * @brief   print energy detection on lcd and uart
 *
 * @param
 *
 * @return  void
 *
 **************************************************************************************************/


void printenergy(){

	if(energyIndex < MSA_MAC_CHANNEL_ALL){

		uint8 CurrentEnergy = msa_EnergyDetect[energyIndex];
		unsigned char curCh[3]="   ";
		_itoa(currentCh, curCh, 10);
		/* uart write energy scan */
		char energyCh[22]="$Energy on ch  :      ";
		energyCh[13]= curCh[0];  // ascii value
		energyCh[14]= curCh[1];  // ascii value
		energyCh[21]= 0xA;
		_itoa(CurrentEnergy, curCh, 10);
		energyCh[17] = curCh[0];
		energyCh[18] = curCh[1];
		energyCh[19] = curCh[2];
		HalUARTWrite(HAL_UART_PORT,(uint8*)energyCh,22);

		HalLcdWriteStringValue("Energy on ch:",currentCh,10,1);
		HalLcdWriteValue(CurrentEnergy,10,2);
		energyIndex++;
		currentCh++;
		osal_start_timer (PRINT_NEXT_ENERGY, time);
	}
	else {
		uint8 scActive[22]="$Starting Active Scan ";
		scActive[21]=0xA;
		HalUARTWrite(HAL_UART_PORT,scActive,22);

		HalLcdWriteStringValue("Coord on ch: ",msa_ChannelExpect,10,1);
		HalLcdWriteStringValue("Coord addr: ",msa_CoordShortAddr-48,10,2);
		char st[18]="$Coord on ch:     ";
		unsigned char x[2] = "  ";
		_itoa(msa_ChannelExpect, x, 10);
		st[15]= x[0];
		st[16]= x[1];
		st[17]= 0xA;
		HalUARTWrite  (HAL_UART_PORT,(uint8*)st,18);

	}
}

/*
 *
 * -----------------------  Coordinator's functions section  ----------------------------
 *
 *
 */

/**************************************************************************************************
 *
 * @fn      MSA_CoordinatorStartup()
 *
 * @brief   Update the timer per tick
 *
 * @param   beaconEnable: TRUE/FALSE
 *
 * @return  None
 *
 **************************************************************************************************/
void MSA_CoordinatorStartup()
{
	/*  Coordinator */


	macMlmeStartReq_t   startReq;

	/* Setup MAC_EXTENDED_ADDRESS */
	sAddrExtCpy(msa_ExtAddr, msa_ExtAddr1);
	MAC_MlmeSetReq(MAC_EXTENDED_ADDRESS, &msa_ExtAddr);

	/* Setup MAC_SHORT_ADDRESS */
	MAC_MlmeSetReq(MAC_SHORT_ADDRESS, &msa_CoordShortAddr);

	/* Setup MAC_BEACON_PAYLOAD_LENGTH */
	MAC_MlmeSetReq(MAC_BEACON_PAYLOAD_LENGTH, &msa_BeaconPayloadLen);

	/* Setup MAC_BEACON_PAYLOAD */
	MAC_MlmeSetReq(MAC_BEACON_PAYLOAD, &msa_BeaconPayload);

	/* Enable RX */
	MAC_MlmeSetReq(MAC_RX_ON_WHEN_IDLE, &msa_MACTrue);

	/* Setup MAC_ASSOCIATION_PERMIT */
	MAC_MlmeSetReq(MAC_ASSOCIATION_PERMIT, &msa_MACTrue);

	/* Fill in the information for the start request structure */
	startReq.startTime = 0;
	startReq.panCoordinator = TRUE;
	startReq.panId = msa_PanId;
	startReq.logicalChannel = msa_ChannelExpect;
	startReq.beaconOrder = msa_BeaconOrder;
	startReq.superframeOrder = msa_SuperFrameOrder;
	startReq.batteryLifeExt = FALSE;
	startReq.coordRealignment = FALSE;
	startReq.realignSec.securityLevel = FALSE;
	startReq.beaconSec.securityLevel = FALSE;

	/* Call start request to start the device as a coordinator */
	MAC_MlmeStartReq(&startReq);

}

/**************************************************************************************************
 *
 * @fn      MSA_AssociateRsp()
 *
 * @brief   This routine is called by Associate_Ind inorder to return the response to the device
 *
 * @param   pMsg - pointer to the structure recieved by MAC_MLME_ASSOCIATE_IND
 *
 * @return  None
 *
 **************************************************************************************************/
void MSA_AssociateRsp(macCbackEvent_t* pMsg)
{
	/* coordinator */
  /* Assign the short address  for the Device, from pool */
  uint16 assocShortAddress = msa_DevShortAddrList[msa_NumOfDevices];

  /* Build the record for this device */
  sAddrExtCpy(msa_DeviceRecord[msa_NumOfDevices].extAddr, pMsg->associateInd.deviceAddress);
  msa_DeviceRecord[msa_NumOfDevices].devShortAddr = msa_DevShortAddrList[msa_NumOfDevices];
  msa_DeviceRecord[msa_NumOfDevices].isDirectMsg = pMsg->associateInd.capabilityInformation & MAC_CAPABLE_RX_ON_IDLE;
  msa_NumOfDevices++;
  index++;

  /* If the number of devices are more than MAX_DEVICE_NUM, turn off the association permit */
  if (msa_NumOfDevices == MSA_MAX_DEVICE_NUM) {
	  msa_NumOfDevices = 0;
    //MAC_MlmeSetReq(MAC_ASSOCIATION_PERMIT, &msa_MACFalse);
  }
  if (index >= MSA_MAX_DEVICE_NUM) {
	  index = 0;
    //MAC_MlmeSetReq(MAC_ASSOCIATION_PERMIT, &msa_MACFalse);
  }
  /* Fill in association respond message */
  sAddrExtCpy(msa_AssociateRsp.deviceAddress, pMsg->associateInd.deviceAddress);
  msa_AssociateRsp.assocShortAddress = assocShortAddress;
  msa_AssociateRsp.status = MAC_SUCCESS;
  msa_AssociateRsp.sec.securityLevel = MAC_SEC_LEVEL_NONE;

  	  /*uart print*/
  	char newDevice[39]="$New Device associated with ShortAdd:  ";
	unsigned char sAdd[4] = "    ";
	_itoa(assocShortAddress, sAdd, 16);

	newDevice[37]= sAdd[1];

	newDevice[38]= 0xA;
	HalUARTWrite(HAL_UART_PORT,(uint8*)newDevice,39);

  /* Call Associate Response */
  MAC_MlmeAssociateRsp(&msa_AssociateRsp);
}


/*
 *
 *  -------------------   End-Device's functions section   -----------------------------
 *
 *
 */

/**************************************************************************************************
 *
 * @fn      MSA_DeviceStartup()
 *
 * @brief   Update the timer per tick
 *
 * @param   beaconEnable: TRUE/FALSE
 *
 * @return  None
 *
 **************************************************************************************************/
void MSA_DeviceStartup()
{
	/*end device*/

	/*
	 * Temporary address used by the end device to communicate(only the first time)
	 * with the coordinator to reach e good address for the network
	 * built from Osal System timer(return uint32 value) with logical & 0xFF and & 0xAF,
	 * that returns 2 different unit8 value	for the last 2 Byte of the address
	 * (the first 6 Byte are fixed)
	 *
	 */

	msa_ExtAddr2[6] = (uint8) (osal_GetSystemClock() & 0xFF );
	msa_ExtAddr2[7] = (uint8) (osal_GetSystemClock() & 0xFF );

	sAddrExtCpy(msa_ExtAddr, msa_ExtAddr2);
	MAC_MlmeSetReq(MAC_EXTENDED_ADDRESS, &msa_ExtAddr);

  /* Setup MAC_BEACON_PAYLOAD_LENGTH */
  MAC_MlmeSetReq(MAC_BEACON_PAYLOAD_LENGTH, &msa_BeaconPayloadLen);

  /* Setup MAC_BEACON_PAYLOAD */
  MAC_MlmeSetReq(MAC_BEACON_PAYLOAD, &msa_BeaconPayload);

  /* Setup PAN ID */
  MAC_MlmeSetReq(MAC_PAN_ID, &msa_PanId);

  /* This device is setup for Direct Message */
  if (msa_IsDirectMsg)
    MAC_MlmeSetReq(MAC_RX_ON_WHEN_IDLE, &msa_MACTrue);
  else
    MAC_MlmeSetReq(MAC_RX_ON_WHEN_IDLE, &msa_MACFalse);

  /* Setup Coordinator short address */
  MAC_MlmeSetReq(MAC_COORD_SHORT_ADDRESS, &msa_AssociateReq.coordAddress.addr.shortAddr);

  if (msa_BeaconOrder != 15)
  {
    /* Setup Beacon Order */
    MAC_MlmeSetReq(MAC_BEACON_ORDER, &msa_BeaconOrder);

    /* Setup Super Frame Order */
    MAC_MlmeSetReq(MAC_SUPERFRAME_ORDER, &msa_SuperFrameOrder);

    /* Sync request */
    MSA_SyncReq();
  }

  /* Power saving */
  MSA_PowerMgr (MSA_PWR_MGMT_ENABLED);

}

/**************************************************************************************************
 *
 * @fn      MSA_AssociateReq()
 *
 * @brief
 *
 * @param    None
 *
 * @return  None
 *
 **************************************************************************************************/
void MSA_AssociateReq(void)
{
		/*  end device */

	  uint8 devCh[19]="$Beacon on ch:     ";
	  unsigned char DCh[2] = "  ";
	  _itoa(msa_ChannelExpect, DCh, 10);
	  devCh[16]= DCh[0];
	  devCh[17]= DCh[1];
	  devCh[18]= 0xA;
	  HalUARTWrite  (HAL_UART_PORT,devCh,19);


	  MAC_MlmeAssociateReq(&msa_AssociateReq);
}

/**************************************************************************************************
 *
 * @fn      MSA_BeaconPayLoadCheck()
 *
 * @brief   Check if the beacon comes from MSA but not zigbee
 *
 * @param   pSdu - pointer to the buffer that contains the data
 *
 * @return  TRUE or FALSE
 *
 **************************************************************************************************/
bool MSA_BeaconPayLoadCheck(uint8* pSdu)
{
	/* end device that check coordinator's beacon payload */
  uint8 i = 0;
  for (i=0; i<msa_BeaconPayloadLen; i++)
  {
    if (pSdu[i] != msa_BeaconPayload[i])
    {
      return FALSE;
    }
  }

  return TRUE;
}

/**************************************************************************************************
 *
 * @fn      MSA_SyncReq()
 *
 * @brief   Sync Request
 *
 * @param   None
 *
 * @return  None
 *
 **************************************************************************************************/
void MSA_SyncReq(void)
{ /* end device */
  macMlmeSyncReq_t syncReq;

  /* Fill in information for sync request structure */
  syncReq.logicalChannel = msa_ChannelExpect;
  syncReq.channelPage    = MAC_CHANNEL_PAGE_0;
  syncReq.trackBeacon    = TRUE;

  /* Call sync request */
  MAC_MlmeSyncReq(&syncReq);
}

/*
 *	 ----------------------------   other functions     ---------------------------------
 *
 */

/**************************************************************************************************
 *
 * @fn      MSA_McpsDataReq()
 *
 * @brief   This routine calls the Data Request
 *
 * @param   data       - contains the data that would be sent
 *          dataLength - length of the data that will be sent
 *
 * @return  None
 *
 **************************************************************************************************/
void MSA_McpsDataReq(uint8* data, uint8 dataLength, bool directMsg, uint16 dstShortAddr)
{
  macMcpsDataReq_t  *pData;
  static uint8      handle = 0;

  if ((pData = MAC_McpsDataAlloc(dataLength, MAC_SEC_LEVEL_NONE, MAC_KEY_ID_MODE_NONE)) != NULL)
  {
    pData->mac.srcAddrMode = SADDR_MODE_SHORT;
    pData->mac.dstAddr.addrMode = SADDR_MODE_SHORT;
    pData->mac.dstAddr.addr.shortAddr = dstShortAddr;
    pData->mac.dstPanId = msa_PanId;
    pData->mac.msduHandle = handle++;
    pData->mac.txOptions = MAC_TXOPTION_ACK;

    /* If it's the coordinator and the device is in-direct message */
    if (msa_IsCoordinator)
    {
      if (!directMsg)
      {
        pData->mac.txOptions |= MAC_TXOPTION_INDIRECT;
      }
    }

    /* Copy data */
    osal_memcpy (pData->msdu.p, data, dataLength);

    /* Send out data request */
    MAC_McpsDataReq(pData);
  }

}

/**************************************************************************************************
 *
 * @fn      MSA_McpsPollReq()
 *
 * @brief   Performs a poll request on the coordinator, NOT USED
 *
 * @param   None
 *
 * @return  None
 *
 **************************************************************************************************/
void MSA_McpsPollReq(void)
{
  macMlmePollReq_t  pollReq;

  /* Fill in information for poll request */
  pollReq.coordAddress.addrMode = SADDR_MODE_SHORT;
  pollReq.coordAddress.addr.shortAddr = msa_CoordShortAddr;
  pollReq.coordPanId = msa_PanId;
  pollReq.sec.securityLevel = MAC_SEC_LEVEL_NONE;

  /* Call poll reuqest */
  MAC_MlmePollReq(&pollReq);
}


/**************************************************************************************************
 *
 * @fn      MSA_DataCheck()
 *
 * @brief   Check if the data match with the predefined data
 *
 * @param    data - pointer to the buffer where the data will be checked against the predefined data
 *           dataLength - length of the data
 *
 * @return  TRUE if the payload is >0
 *
 **************************************************************************************************/
bool MSA_DataCheck(uint8* data, uint8 dataLength)
{
	/* if packet payload is empty return false*/
	if(dataLength > 0)
		return TRUE;
	else
		return FALSE;
}

/**************************************************************************************************
 *
 * @fn      MAC_CbackCheckPending
 *
 * @brief   Returns the number of indirect messages pending in the application
 *
 * @param   None
 *
 * @return  Number of indirect messages in the application
 *
 **************************************************************************************************/
uint8 MAC_CbackCheckPending(void)
{
  return (0);
}

/*********************************************************************
 * @fn      _itoa
 *
 * @brief   convert a 16bit number to ASCII
 *
 * @param   num -
 *          buf -
 *          radix -
 *
 * @return  void
 *
 *********************************************************************/
void _itoa(uint16 num, byte *buf, byte radix)
{
  char c,i;
  byte *p, rst[5];

  p = rst;
  for ( i=0; i<5; i++,p++ )
  {
    c = num % radix;  // Isolate a digit
    *p = c + (( c < 10 ) ? '0' : '7');  // Convert to Ascii
    num /= radix;
    if ( !num )
      break;
  }

  for ( c=0 ; c<=i; c++ )
    *buf++ = *p--;  // Reverse character order

  //buf = '\0';
}


/**************************************************************************************************
 **************************************************************************************************/

