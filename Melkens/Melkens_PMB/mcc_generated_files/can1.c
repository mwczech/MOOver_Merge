/**
  CAN1 Generated Driver File

  @Company
    Microchip Technology Inc.

  @File Name
    can1.c

  @Summary
    This is the generated driver implementation file for the CAN1 driver using PIC24 / dsPIC33 / PIC32MM MCUs

  @Description
    This source file provides APIs for CAN1.
    Generation Information :
        Product Revision   :  PIC24 / dsPIC33 / PIC32MM MCUs - 1.171.1
        Device             :  dsPIC33CK256MP506
    The generated drivers are tested against the following:
        Compiler           :  XC16 v1.70
        MPLAB              :  MPLAB X v5.50
*/

/*
    (c) 2020 Microchip Technology Inc. and its subsidiaries. You may use this
    software and any derivatives exclusively with Microchip products.

    THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
    EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
    WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
    PARTICULAR PURPOSE, OR ITS INTERACTION WITH MICROCHIP PRODUCTS, COMBINATION
    WITH ANY OTHER PRODUCTS, OR USE IN ANY APPLICATION.

    IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
    WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
    BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
    FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
    ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
    THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.

    MICROCHIP PROVIDES THIS SOFTWARE CONDITIONALLY UPON YOUR ACCEPTANCE OF THESE
    TERMS.
*/

/**
  Section: Included Files
*/

#include <string.h>    
#include "can1.h"
#include "pin_manager.h"

#define CAN1_FIFO_ALLOCATE_RAM_SIZE    496U //CAN FIFO allocated ram size based on (number of FIFO x FIFO message Payload size x Message object DLC size)
#define CAN1_NUM_OF_RX_FIFO            6U    // No of RX FIFO's configured
#define CAN1_RX_FIFO_MSG_DATA          8U   // CAN RX FIFO Message object data field size

#define CAN1_TX_MSG_SEND_REQ_BIT_POS   0x200U // CAN FIFO TX Message Send Request bit 
#define CAN1_TX_INC_FIFO_PTR_BIT_POS   0x100U // CAN FIFO Increment Head/Tail bit  

// CAN Message object arbitration field information
#define CAN_MSG_OBJ_DLC_FIELD_SIZE          0xFU
#define CAN_MSG_OBJ_ID_TYPE_FIELD_POS       0x10U
#define CAN_MSG_OBJ_ID_TYPE_SHIFT_POS       0x4U
#define CAN_MSG_OBJ_FRAME_TYPE_FIELD_POS    0x20U
#define CAN_MSG_OBJ_FRAME_TYPE_SHIFT_POS    0x5U
#define CAN_MSG_OBJ_BRS_FIELD_POS           0x40U
#define CAN_MSG_OBJ_BRS_SHIFT_POS           0x6U
#define CAN_MSG_OBJ_FORMAT_TYPE_FIELD_POS   0x80U
#define CAN_MSG_OBJ_FORMAT_TYPE_SHIFT_POS   0x7U
#define CAN_STD_MSG_ID_MAX_SIZE             0x7FFU
#define CAN_MSG_OBJ_SID_SHIFT_POS           0x12U
#define CAN_EXT_MSG_ID_HIGH_MAX_SIZE        0x1FFFU
#define CAN_EXT_MSG_ID_LOW_MAX_SIZE         0x1FU
#define CAN_MSG_OBJ_EID_LOW_SHIFT_POS       0xBU
#define CAN_MSG_OBJ_EID_HIGH_SHIFT_POS      0x5U

/**
  CAN FD Receive FIFO Status Enumeration

  @Summary
    Defines the CAN FD receive FIFO status Enumeration.

  @Description
    This enumeration defines the CAN FD receive FIFO status.
*/
typedef enum 
{
    CAN_RX_MSG_NOT_AVAILABLE = 0x0,
    CAN_RX_MSG_AVAILABLE = 0x1,
    CAN_RX_MSG_OVERFLOW = 0x8
} CAN1_RX_FIFO_STATUS;

//CAN RX FIFO Channels
typedef enum 
{
    CAN1_FIFO_1 = 1,
    CAN1_FIFO_2 = 2,
    CAN1_FIFO_3 = 3,
    CAN1_FIFO_4 = 4,
    CAN1_FIFO_5 = 5,
    CAN1_FIFO_6 = 6,
} CAN1_RX_FIFO_CHANNELS;

/**
 Section: Private Variable Definitions
*/
//Start CAN Message Memory Base Address
static uint8_t __attribute__((aligned(4)))can1FifoMsg[CAN1_FIFO_ALLOCATE_RAM_SIZE];

// CAN Default Interrupt Handler
static void (*CAN1_InvalidMessageInterruptHandler)(void) = NULL;
static void (*CAN1_BusWakeUpActivityInterruptHandler)(void) = NULL;
static void (*CAN1_BusErrorInterruptHandler)(void) = NULL;
static void (*CAN1_ModeChangeInterruptHandler)(void) = NULL;
static void (*CAN1_SystemErrorInterruptHandler)(void) = NULL;
static void (*CAN1_TxAttemptInterruptHandler)(void) = NULL;
static void (*CAN1_RxBufferOverFlowInterruptHandler)(void) = NULL;

//CAN Receive FIFO Message object data field 
static uint8_t rxMsgData[CAN1_RX_FIFO_MSG_DATA];

/**
  CAN1 Receive FIFO Message Object head count information maintenance data structure

  @Summary
    Defines the CAN1 receive FIFO message object head count information maintenance data structure.

  @Description
    This defines the object required for the maintenance of the RX FIFO message object.
*/
typedef struct 
{
    const CAN1_RX_FIFO_CHANNELS channel;
    uint8_t headCount;
} CAN1_RX_FIFO_MSG;

static volatile CAN1_RX_FIFO_MSG rxFIFOMsg[CAN1_NUM_OF_RX_FIFO] = 
{
    //Receive FIFO, FIFO head count
    {CAN1_FIFO_1, 0U},
    {CAN1_FIFO_2, 0U},
    {CAN1_FIFO_3, 0U},
    {CAN1_FIFO_4, 0U},
    {CAN1_FIFO_5, 0U},
    {CAN1_FIFO_6, 0U},
}; 

/**
  CAN1 FIFO parameters information data structure.

  @Summary
    Defines the CAN1 FIFO parameters information data structure.

  @Description
    This Data structure is to implement a CAN FIFO parameters information.
*/
typedef struct 
{
    uint8_t payloadSize;
    uint8_t msgDeepSize;
    uint16_t *address;
} CAN1_FIFO_INFO;

/**
 Section: Private Function Definitions
*/

/**
  @Summary
    Get the DLC enum based decimal value.

  @Description
    This routine get the DLC enum based decimal value.

  @Preconditions
    CAN1_Initialize function should be called before calling this function.

  @Param
    dlc - Data Length Code enum value

  @Returns
    Return the Data Length Code decimal value
    
  @Example
    None
*/
static uint8_t CAN1_DlcToDataBytesGet(const CAN_DLC dlc)
{
    static const uint8_t dlcByteSize[] = {0U, 1U, 2U, 3U, 4U, 5U, 6U, 7U, 8U};
    return dlcByteSize[dlc];
}

/**
  @Summary
    Get the FIFO user address, message deep size and payload size information.

  @Description
    This routine get the FIFO user address, message deep size and payload size
    information.

  @Preconditions
    CAN1_Initialize function should be called before calling this function.

  @Param
    fifoNum - FIFO Channel selection 
    fifoInfo - pointer to the CAN FIFO info. 

  @Returns
    None
    
  @Example
    None
*/

static void CAN1_FIFO_InfoGet(const uint8_t fifoNum, CAN1_FIFO_INFO *fifoInfo)
{   
    switch (fifoNum) 
    {
        case CAN1_TXQ:
            fifoInfo->address = (uint16_t *) &C1TXQUAL;
            fifoInfo->payloadSize = 8U;
            fifoInfo->msgDeepSize = 24U;
            break;
     
        case CAN1_FIFO_1:
            fifoInfo->address = (uint16_t *) &C1FIFOUA1L;
            fifoInfo->payloadSize = 8U;
            fifoInfo->msgDeepSize = 1U;
            break;
     
        case CAN1_FIFO_2:
            fifoInfo->address = (uint16_t *) &C1FIFOUA2L;
            fifoInfo->payloadSize = 8U;
            fifoInfo->msgDeepSize = 1U;
            break;
     
        case CAN1_FIFO_3:
            fifoInfo->address = (uint16_t *) &C1FIFOUA3L;
            fifoInfo->payloadSize = 8U;
            fifoInfo->msgDeepSize = 1U;
            break;
     
        case CAN1_FIFO_4:
            fifoInfo->address = (uint16_t *) &C1FIFOUA4L;
            fifoInfo->payloadSize = 8U;
            fifoInfo->msgDeepSize = 1U;
            break;
     
        case CAN1_FIFO_5:
            fifoInfo->address = (uint16_t *) &C1FIFOUA5L;
            fifoInfo->payloadSize = 8U;
            fifoInfo->msgDeepSize = 1U;
            break;
     
        case CAN1_FIFO_6:
            fifoInfo->address = (uint16_t *) &C1FIFOUA6L;
            fifoInfo->payloadSize = 8U;
            fifoInfo->msgDeepSize = 1U;
            break;
     
        case CAN1_FIFO_7:
            fifoInfo->address = (uint16_t *) &C1FIFOUA7L;
            fifoInfo->payloadSize = 8U;
            fifoInfo->msgDeepSize = 1U;
            break;
     
        default:
            fifoInfo->address = NULL;
            fifoInfo->payloadSize = 0U;
            fifoInfo->msgDeepSize = 0U;
            break;
    }
}

/**
  @Summary
    Reset the CAN1 Receive message head count.

  @Description
    This routine reset the CAN1 Receive message head count.

  @Preconditions
    CAN1_Initialize function should be called before calling this function.

  @Param
    None  

  @Returns
    None

  @Example
    None
*/
static void CAN1_RX_FIFO_ResetInfo()
{
    uint8_t count;
    
    for(count = 0; count < CAN1_NUM_OF_RX_FIFO; count++)
    {
        rxFIFOMsg[count].headCount = 0;
    }
}

/**
  @Summary
    CAN1 Receive FIFO status.

  @Description
    This returns the CAN1 Receive FIFO status.

  @Preconditions
    CAN1_Initializer function should have been called before calling this function.

  @Param
    fifoNum  - FIFO Channel selection

  @Returns
    CAN Receive FIFO status.
    CAN_RX_MSG_NOT_AVAILABLE - CAN Receive FIFO is empty
    CAN_RX_MSG_AVAILABLE     - CAN Receive FIFO at least one message available
    CAN_RX_MSG_OVERFLOW      - CAN Receive FIFO is Overflow

  @Example
    None
*/
static CAN1_RX_FIFO_STATUS CAN1_RX_FIFO_StatusGet(const CAN1_RX_FIFO_CHANNELS fifoNum)
{
    CAN1_RX_FIFO_STATUS rxFifoStatus;
    
    switch (fifoNum) 
    {
        case CAN1_FIFO_1:
            rxFifoStatus = (C1FIFOSTA1 & (CAN_RX_MSG_AVAILABLE | CAN_RX_MSG_OVERFLOW));
            break;

        case CAN1_FIFO_2:
            rxFifoStatus = (C1FIFOSTA2 & (CAN_RX_MSG_AVAILABLE | CAN_RX_MSG_OVERFLOW));
            break;

        case CAN1_FIFO_3:
            rxFifoStatus = (C1FIFOSTA3 & (CAN_RX_MSG_AVAILABLE | CAN_RX_MSG_OVERFLOW));
            break;

        case CAN1_FIFO_4:
            rxFifoStatus = (C1FIFOSTA4 & (CAN_RX_MSG_AVAILABLE | CAN_RX_MSG_OVERFLOW));
            break;

        case CAN1_FIFO_5:
            rxFifoStatus = (C1FIFOSTA5 & (CAN_RX_MSG_AVAILABLE | CAN_RX_MSG_OVERFLOW));
            break;

        case CAN1_FIFO_6:
            rxFifoStatus = (C1FIFOSTA6 & (CAN_RX_MSG_AVAILABLE | CAN_RX_MSG_OVERFLOW));
            break;

        default:
            rxFifoStatus = CAN_RX_MSG_NOT_AVAILABLE;
            break;
    }
    
    return rxFifoStatus;
}

/**
  @Summary
    Clear the CAN1 Receive FIFO overflow status bit.

  @Description
    This routine is used to clear the CAN1 Receive FIFO overflow status bit

  @Preconditions
    CAN1_Initialize function should be called before calling this function.

  @Param
    fifoNum - FIFO Channel selection

  @Returns
    None
    
  @Example
    None
*/
static void CAN1_RX_FIFO_OverflowStatusFlagClear(const CAN1_RX_FIFO_CHANNELS fifoNum)
{   
    switch (fifoNum) 
    {
        case CAN1_FIFO_1:
            C1FIFOSTA1bits.RXOVIF = false;
            break;
            
        case CAN1_FIFO_2:
            C1FIFOSTA2bits.RXOVIF = false;
            break;
            
        case CAN1_FIFO_3:
            C1FIFOSTA3bits.RXOVIF = false;
            break;
            
        case CAN1_FIFO_4:
            C1FIFOSTA4bits.RXOVIF = false;
            break;
            
        case CAN1_FIFO_5:
            C1FIFOSTA5bits.RXOVIF = false;
            break;
            
        case CAN1_FIFO_6:
            C1FIFOSTA6bits.RXOVIF = false;
            break;
            
        default:
            break;
    }
}

/**
  @Summary
    Update the Receive FIFO message increment tail position.

  @Description
    This routine Update the Receive FIFO message increment tail position.

  @Preconditions
    CAN1_Initialize function should be called before calling this function.

  @Param
    fifoNum - FIFO Channel selection  

  @Returns
    None
    
  @Example
    None
*/
static void CAN1_RX_FIFO_IncrementMsgPtr(const uint8_t fifoNum) 
{
    switch (fifoNum) 
    {   
        case CAN1_FIFO_1:
            C1FIFOCON1Lbits.UINC = 1; // Update the CAN1_FIFO_1 message pointer.
            break;
        
        case CAN1_FIFO_2:
            C1FIFOCON2Lbits.UINC = 1; // Update the CAN1_FIFO_2 message pointer.
            break;
        
        case CAN1_FIFO_3:
            C1FIFOCON3Lbits.UINC = 1; // Update the CAN1_FIFO_3 message pointer.
            break;
        
        case CAN1_FIFO_4:
            C1FIFOCON4Lbits.UINC = 1; // Update the CAN1_FIFO_4 message pointer.
            break;
        
        case CAN1_FIFO_5:
            C1FIFOCON5Lbits.UINC = 1; // Update the CAN1_FIFO_5 message pointer.
            break;
        
        case CAN1_FIFO_6:
            C1FIFOCON6Lbits.UINC = 1; // Update the CAN1_FIFO_6 message pointer.
            break;
        
        default:
            break;
    }    
}

/**
  @Summary
    Get the Receiver FIFO message index value.

  @Description
    This routine get the Receiver FIFO message index value.

  @Preconditions
    CAN1_Initialize function should be called before calling this function.

  @Param
    fifoNum - FIFO Channel selection  

  @Returns
    Return the FIFO message index value
    
  @Example
    None
*/
static uint16_t CAN1_RX_FIFO_MessageIndexGet(const CAN1_RX_FIFO_CHANNELS fifoNum) 
{
    uint16_t fifoMsgIndex;
    
    switch (fifoNum) 
    {            
        case CAN1_FIFO_1:
            fifoMsgIndex = C1FIFOSTA1bits.FIFOCI;
            break;

        case CAN1_FIFO_2:
            fifoMsgIndex = C1FIFOSTA2bits.FIFOCI;
            break;

        case CAN1_FIFO_3:
            fifoMsgIndex = C1FIFOSTA3bits.FIFOCI;
            break;

        case CAN1_FIFO_4:
            fifoMsgIndex = C1FIFOSTA4bits.FIFOCI;
            break;

        case CAN1_FIFO_5:
            fifoMsgIndex = C1FIFOSTA5bits.FIFOCI;
            break;

        case CAN1_FIFO_6:
            fifoMsgIndex = C1FIFOSTA6bits.FIFOCI;
            break;

        default:
            fifoMsgIndex = 0;
            break;
    }
    
    return fifoMsgIndex;
}

/**
  @Summary
    Enable the transmit FIFO message send request bit.

  @Description
    This routine Enable The FIFO transmit message send request bit.

  @Preconditions
    CAN1_Initialize function should be called before calling this function.

  @Param
    fifoChannel - CAN1 TX priority FIFO selection  

  @Returns
    None
    
  @Example
    None
*/
static void CAN1_TX_FIFO_MessageSendRequest(const CAN1_TX_FIFO_CHANNELS fifoChannel)
{   
    switch (fifoChannel) 
    {           
        case CAN1_TXQ:
            // Update the CAN1_TXQ message pointer; Set TXREQ bit
            C1TXQCONL = (C1TXQCONL | (CAN1_TX_MSG_SEND_REQ_BIT_POS | CAN1_TX_INC_FIFO_PTR_BIT_POS));
            break;

        case CAN1_FIFO_7:
            // Update the CAN1_FIFO_7 message pointer; Set TXREQ bit
            C1FIFOCON7L = (C1FIFOCON7L | (CAN1_TX_MSG_SEND_REQ_BIT_POS | CAN1_TX_INC_FIFO_PTR_BIT_POS));
            break;

        default:
            break;
    }      
}

/**
  @Summary
    Read the message object from Receive FIFO and update to the user message object 
    pointer.

  @Description
    This routine read the message object from Receive FIFO and update to the user  
    message object pointer.

  @Preconditions
    CAN1_Initialize function should be called before calling this function.

  @Param
    rxFifoObj - a pointer to the Receive FIFO where the message would be stored 
    rxCanMsg  - pointer to the CAN receive message object. 

  @Returns
    None
    
  @Example
    None
*/
static void CAN1_MessageReadFromFifo(uint16_t *rxFifoObj, CAN_MSG_OBJ *rxCanMsg)
{   
    uint8_t dlcByteSize = 0;
        
    // SID <10:0> and EID <4:0>
    uint16_t rx0Data = *rxFifoObj;
    rxFifoObj += 1;

    // SID11 and EID <18:5>
    uint16_t rx1Data = *rxFifoObj;
    rxFifoObj += 1;

    // DLC <3:0>, IDE <1>, FDF <1> 
    rxCanMsg->field.dlc = (*rxFifoObj & CAN_MSG_OBJ_DLC_FIELD_SIZE);
    rxCanMsg->field.idType = ((*rxFifoObj & CAN_MSG_OBJ_ID_TYPE_FIELD_POS) >> CAN_MSG_OBJ_ID_TYPE_SHIFT_POS);
    rxCanMsg->field.frameType = ((*rxFifoObj & CAN_MSG_OBJ_FRAME_TYPE_FIELD_POS) >> CAN_MSG_OBJ_FRAME_TYPE_SHIFT_POS);
    rxCanMsg->field.brs = ((*rxFifoObj & CAN_MSG_OBJ_BRS_FIELD_POS) >> CAN_MSG_OBJ_BRS_SHIFT_POS);
    rxCanMsg->field.formatType = ((*rxFifoObj & CAN_MSG_OBJ_FORMAT_TYPE_FIELD_POS) >> CAN_MSG_OBJ_FORMAT_TYPE_SHIFT_POS);

    /* message is standard identifier */
    if(rxCanMsg->field.idType == CAN_FRAME_STD) 
    {
        // SID <10:0>
        rxCanMsg->msgId = (rx0Data & CAN_STD_MSG_ID_MAX_SIZE);   
    }
    else 
    {
        /* message is extended identifier */
        // EID <28:18>, EID <17:0>
        rxCanMsg->msgId = (((uint32_t)(rx0Data & CAN_STD_MSG_ID_MAX_SIZE) << CAN_MSG_OBJ_SID_SHIFT_POS) | 
                            ((uint32_t)(rx1Data & CAN_EXT_MSG_ID_HIGH_MAX_SIZE) << CAN_MSG_OBJ_EID_HIGH_SHIFT_POS) | 
                            ((uint32_t)(rx0Data >> CAN_MSG_OBJ_EID_LOW_SHIFT_POS) & CAN_EXT_MSG_ID_LOW_MAX_SIZE));
    }

    rxFifoObj += 2;

    dlcByteSize = CAN1_DlcToDataBytesGet(rxCanMsg->field.dlc);

    // Coping Receive FIFO data starting memory location
    memset(rxMsgData, 0, CAN1_RX_FIFO_MSG_DATA);
    memcpy((char *) rxMsgData, (char *) rxFifoObj, dlcByteSize);
    rxCanMsg->data = rxMsgData;
}

/**
  @Summary
    Read the message object from user input and update to the CAN1 TX FIFO.

  @Description
    This routine Read the message object from user input and update to the CAN1 TX FIFO.

  @Preconditions
    CAN1_Initialize function should be called before calling this function.

  @Param
    txFifoObj   - a pointer to the TX FIFO where the message would be stored 
    txCanMsg    - pointer to the CAN1 transmit message object. 

  @Returns
    None
    
  @Example
    None
*/
static void CAN1_MessageWriteToFifo(uint16_t *txFifoObj, CAN_MSG_OBJ *txCanMsg)
{
    uint8_t dlcByteSize = 0;

    /* message is standard identifier */
    if(txCanMsg->field.idType == CAN_FRAME_STD) 
    {
        // SID <10:0>
        *txFifoObj = (txCanMsg->msgId & CAN_STD_MSG_ID_MAX_SIZE);
        txFifoObj += 2;
    } 
    else 
    {
        /* message is extended identifier */
        // EID <28:18> and EID <4:0>
        *txFifoObj = (((txCanMsg->msgId >> CAN_MSG_OBJ_SID_SHIFT_POS) & CAN_STD_MSG_ID_MAX_SIZE) | 
                        (txCanMsg->msgId & CAN_EXT_MSG_ID_LOW_MAX_SIZE) << CAN_MSG_OBJ_EID_LOW_SHIFT_POS);
        txFifoObj += 1;

        // EID <5:17>
        *txFifoObj = ((txCanMsg->msgId >>  CAN_MSG_OBJ_EID_HIGH_SHIFT_POS) & CAN_EXT_MSG_ID_HIGH_MAX_SIZE);
        txFifoObj += 1;       
    }

    // DLC <3:0>, IDE <1>, BRS <1>, FDF <1> 
    *txFifoObj = (txCanMsg->field.dlc & CAN_MSG_OBJ_DLC_FIELD_SIZE) | 
                    ((txCanMsg->field.idType << CAN_MSG_OBJ_ID_TYPE_SHIFT_POS) & CAN_MSG_OBJ_ID_TYPE_FIELD_POS) | 
                    ((txCanMsg->field.frameType << CAN_MSG_OBJ_FRAME_TYPE_SHIFT_POS) & CAN_MSG_OBJ_FRAME_TYPE_FIELD_POS) | 
                    ((txCanMsg->field.brs << CAN_MSG_OBJ_BRS_SHIFT_POS) & CAN_MSG_OBJ_BRS_FIELD_POS) | 
                    ((txCanMsg->field.formatType << CAN_MSG_OBJ_FORMAT_TYPE_SHIFT_POS) & CAN_MSG_OBJ_FORMAT_TYPE_FIELD_POS);

    // Data frame message
    if(txCanMsg->field.frameType == CAN_FRAME_DATA)
    {
        txFifoObj += 2;   

        dlcByteSize = CAN1_DlcToDataBytesGet(txCanMsg->field.dlc);

        // Coping TX message object to FIFO
        memcpy(txFifoObj, txCanMsg->data, dlcByteSize);
    }
}

/**
  @Summary
    Configure the CAN1 TX FIFO settings.

  @Description
    This routine configure the CAN1 TX FIFO settings.

  @Preconditions
    None

  @Param
    None  

  @Returns
    None
    
  @Example
    None
*/
static void CAN1_TX_FIFO_Configuration(void)
{
    // TXQEIE disabled; TXREQ disabled; TXQNIE disabled; TXATIE enabled; UINC disabled; FRESET enabled; 
    C1TXQCONL = 0x410;
    // TXAT Unlimited attempts; PLSIZE 8; FSIZE 24; TXPRI 1; 
    C1TXQCONH = 0x1741;
    // TFHRFHIE disabled; TFERFFIE disabled; RXTSEN disabled; TXREQ disabled; RXOVIE disabled; RTREN disabled; TXEN enabled; TXATIE enabled; UINC disabled; FRESET enabled; TFNRFNIE disabled; 
    C1FIFOCON7L = 0x490;
    // TXAT Unlimited attempts; PLSIZE 8; FSIZE 1; TXPRI 0; 
    C1FIFOCON7H = 0x40;
}

/**
  @Summary
    Configure the CAN1 Receive FIFO settings.

  @Description
    This routine configure the CAN1 Receive FIFO settings.

  @Preconditions
    None

  @Param
    None  

  @Returns
    None
    
  @Example
    None
*/

static void CAN1_RX_FIFO_Configuration(void)
{          
    // TFHRFHIE disabled; TFERFFIE disabled; RXTSEN disabled; TXREQ disabled; RXOVIE enabled; RTREN disabled; TXEN disabled; TXATIE disabled; UINC disabled; FRESET enabled; TFNRFNIE disabled; 
    C1FIFOCON1L = 0x408;
    // TXAT Disabled; PLSIZE 8; FSIZE 1; TXPRI 0; 
    C1FIFOCON1H = 0x00;
    // TFHRFHIE disabled; TFERFFIE disabled; RXTSEN disabled; TXREQ disabled; RXOVIE enabled; RTREN disabled; TXEN disabled; TXATIE disabled; UINC disabled; FRESET enabled; TFNRFNIE disabled; 
    C1FIFOCON2L = 0x408;
    // TXAT Unlimited attempts; PLSIZE 8; FSIZE 1; TXPRI 0; 
    C1FIFOCON2H = 0x40;
    // TFHRFHIE disabled; TFERFFIE disabled; RXTSEN disabled; TXREQ disabled; RXOVIE enabled; RTREN disabled; TXEN disabled; TXATIE disabled; UINC disabled; FRESET enabled; TFNRFNIE disabled; 
    C1FIFOCON3L = 0x408;
    // TXAT Disabled; PLSIZE 8; FSIZE 1; TXPRI 0; 
    C1FIFOCON3H = 0x00;
    // TFHRFHIE disabled; TFERFFIE disabled; RXTSEN disabled; TXREQ disabled; RXOVIE enabled; RTREN disabled; TXEN disabled; TXATIE disabled; UINC disabled; FRESET enabled; TFNRFNIE disabled; 
    C1FIFOCON4L = 0x408;
    // TXAT Disabled; PLSIZE 8; FSIZE 1; TXPRI 0; 
    C1FIFOCON4H = 0x00;
    // TFHRFHIE disabled; TFERFFIE disabled; RXTSEN disabled; TXREQ disabled; RXOVIE enabled; RTREN disabled; TXEN disabled; TXATIE disabled; UINC disabled; FRESET enabled; TFNRFNIE disabled; 
    C1FIFOCON5L = 0x408;
    // TXAT Disabled; PLSIZE 8; FSIZE 1; TXPRI 0; 
    C1FIFOCON5H = 0x00;
    // TFHRFHIE disabled; TFERFFIE disabled; RXTSEN disabled; TXREQ disabled; RXOVIE enabled; RTREN disabled; TXEN disabled; TXATIE disabled; UINC disabled; FRESET enabled; TFNRFNIE disabled; 
    C1FIFOCON6L = 0x408;
    // TXAT Disabled; PLSIZE 8; FSIZE 1; TXPRI 0; 
    C1FIFOCON6H = 0x00;
}

/**
  @Summary
    Configure the CAN1 Filter and mask settings.

  @Description
    This routine configure the CAN1 Filter and mask settings.

  @Preconditions
    None

  @Param
    None  

  @Returns
    None
    
  @Example
    None
*/
static void CAN1_RX_FIFO_FilterMaskConfiguration(void)
{
    /* Configure RX FIFO Filter control settings*/
    
    // message stored in FIFO1
    C1FLTCON0Lbits.F0BP = 0x01;
    // EID 31; SID 352; 
    C1FLTOBJ0L = 0xF960;
    // EID 3; EXIDE enabled; SID11 disabled; 
    C1FLTOBJ0H = 0x4003;
    // MSID 2047; MEID 31; 
    C1MASK0L = 0xFFFF;
    // MEID 8191; MSID11 disabled; MIDE enabled; 
    C1MASK0H = 0x5FFF;
    // Enable the filter 0
    C1FLTCON0Lbits.FLTEN0 = 0x01;
    
    // message stored in FIFO2
    C1FLTCON0Lbits.F1BP = 0x02;
    // EID 31; SID 448; 
    C1FLTOBJ1L = 0xF9C0;
    // EID 3; EXIDE enabled; SID11 disabled; 
    C1FLTOBJ1H = 0x4003;
    // MSID 2047; MEID 31; 
    C1MASK1L = 0xFFFF;
    // MEID 8191; MSID11 disabled; MIDE enabled; 
    C1MASK1H = 0x5FFF;
    // Enable the filter 1
    C1FLTCON0Lbits.FLTEN1 = 0x01;
    
    // message stored in FIFO3
    C1FLTCON0Hbits.F2BP = 0x03;
    // EID 30; SID 352; 
    C1FLTOBJ2L = 0xF160;
    // EID 3; EXIDE enabled; SID11 disabled; 
    C1FLTOBJ2H = 0x4003;
    // MSID 2047; MEID 31; 
    C1MASK2L = 0xFFFF;
    // MEID 8191; MSID11 disabled; MIDE enabled; 
    C1MASK2H = 0x5FFF;
    // Enable the filter 2
    C1FLTCON0Hbits.FLTEN2 = 0x01;
    
    // message stored in FIFO4
    C1FLTCON0Hbits.F3BP = 0x04;
    // EID 30; SID 448; 
    C1FLTOBJ3L = 0xF1C0;
    // EID 3; EXIDE enabled; SID11 disabled; 
    C1FLTOBJ3H = 0x4003;
    // MSID 2047; MEID 31; 
    C1MASK3L = 0xFFFF;
    // MEID 8191; MSID11 disabled; MIDE enabled; 
    C1MASK3H = 0x5FFF;
    // Enable the filter 3
    C1FLTCON0Hbits.FLTEN3 = 0x01;
    
    // message stored in FIFO5
    C1FLTCON1Lbits.F5BP = 0x05;
    // EID 29; SID 352; 
    C1FLTOBJ5L = 0xE960;
    // EID 3; EXIDE enabled; SID11 disabled; 
    C1FLTOBJ5H = 0x4003;
    // MSID 2047; MEID 31; 
    C1MASK5L = 0xFFFF;
    // MEID 8191; MSID11 disabled; MIDE enabled; 
    C1MASK5H = 0x5FFF;
    // Enable the filter 5
    C1FLTCON1Lbits.FLTEN5 = 0x01;
    
    // message stored in FIFO6
    C1FLTCON1Hbits.F6BP = 0x06;
    // EID 29; SID 448; 
    C1FLTOBJ6L = 0xE9C0;
    // EID 3; EXIDE enabled; SID11 disabled; 
    C1FLTOBJ6H = 0x4003;
    // MSID 2047; MEID 31; 
    C1MASK6L = 0xFFFF;
    // MEID 8191; MSID11 disabled; MIDE enabled; 
    C1MASK6H = 0x5FFF;
    // Enable the filter 6
    C1FLTCON1Hbits.FLTEN6 = 0x01;
}

/**
  @Summary
    Configure the CAN1 Bit rate settings.

  @Description
    This routine configure the CAN1 Bit rate settings.

  @Preconditions
    None

  @Param
    None  

  @Returns
    None
    
  @Example
    None
*/
static void CAN1_BitRateConfiguration(void)
{
    // SJW 7; TSEG2 7; 
    C1NBTCFGL = 0x707;
    // BRP 0; TSEG1 30; 
    C1NBTCFGH = 0x1E;
}

/**
  @Summary
    Enable the CAN1 Error Notification interrupt.

  @Description
    This routine enables the CAN1  error notification interrupt.

  @Preconditions
    CAN1_Initialize function should be called before calling this function.

  @Param
    None  

  @Returns
    None
    
  @Example
    None
*/
static void CAN1_ErrorNotificationInterruptEnable(void)
{

    /* Initialize Interrupt Handler*/
    CAN1_SetInvalidMessageInterruptHandler(&CAN1_DefaultInvalidMessageHandler);
    CAN1_SetBusWakeUpActivityInterruptHandler(&CAN1_DefaultBusWakeUpActivityHandler);
    CAN1_SetBusErrorInterruptHandler(&CAN1_DefaultBusErrorHandler);
    CAN1_SetModeChangeInterruptHandler(&CAN1_DefaultModeChangeHandler);
    CAN1_SetSystemErrorInterruptHandler(&CAN1_DefaultSystemErrorHandler);
    CAN1_SetTxAttemptInterruptHandler(&CAN1_DefaultTxAttemptHandler);
    CAN1_SetRxBufferOverFlowInterruptHandler(&CAN1_DefaultRxBufferOverFlowHandler);
    
    // Clear the interrupt flags
    IFS1bits.C1IF = 0; // CAN Info Interrupt flag
    
    // IVMIF disabled; WAKIF disabled; CERRIF disabled; SERRIF disabled; TBCIF disabled; MODIF disabled; 
    C1INTL = 0x00;
    // IVMIE enabled; TEFIE disabled; RXOVIE enabled; RXIE disabled; WAKIE enabled; TXIE disabled; CERRIE enabled; SERRIE enabled; MODIE enabled; TXATIE enabled; TBCIE disabled; 
    C1INTH = 0xFC08;

    IEC1bits.C1IE = 1; // CAN Info Interrupt Enable bit   
}

/**
 Section: Driver Interface Function Definitions
*/
void CAN1_Initialize(void)
{
    /* Enable the CAN1 module */
    C1CONLbits.CON = 1;
    
    // RTXAT disabled; ESIGM disabled; TXBWS No delay; STEF disabled; SERRLOM disabled; ABAT disabled; REQOP Configuration mode; TXQEN enabled; 
    C1CONH = 0x410;  

    /* Place CAN1 module in configuration mode */
    if(CAN_OP_MODE_REQUEST_SUCCESS == CAN1_OperationModeSet(CAN_CONFIGURATION_MODE))
    {
        /* Initialize the C1FIFOBAL with the start address of the CAN1 FIFO message object area. */
        C1FIFOBAL = (uint16_t) &can1FifoMsg;
        
        // BRSDIS enabled; CON enabled; WAKFIL enabled; WFT T11 Filter; ISOCRCEN enabled; SIDL disabled; DNCNT 0; PXEDIS enabled; CLKSEL disabled; 
        C1CONL = 0x9760;
    
        // Disabled CAN1 Store in Transmit Event FIFO bit
        C1CONHbits.STEF = 0;
        // Enabled CAN1 Transmit Queue bit
        C1CONHbits.TXQEN = 1;
        
        /*configure CAN1 Bit rate settings*/
        CAN1_BitRateConfiguration();        
        
        /*configure CAN1 FIFO settings*/
        CAN1_TX_FIFO_Configuration();
        CAN1_RX_FIFO_Configuration();
        
        /* Configure Receive FIFO Filter and Mask settings*/
        CAN1_RX_FIFO_FilterMaskConfiguration();    

        /* Configure Interrupt*/
        CAN1_ErrorNotificationInterruptEnable();
        
        // Reset the CAN1 Receive FIFO head count
        CAN1_RX_FIFO_ResetInfo();
		
        /* Place CAN1 module in Normal Operation mode */
        CAN1_OperationModeSet(CAN_NORMAL_2_0_MODE);
}
}

CAN_OP_MODE_STATUS CAN1_OperationModeSet(const CAN_OP_MODES requestMode) 
{
    CAN_OP_MODE_STATUS status = CAN_OP_MODE_REQUEST_SUCCESS;
    
    if((CAN_CONFIGURATION_MODE == CAN1_OperationModeGet()) || (requestMode == CAN_DISABLE_MODE)
            || (requestMode == CAN_CONFIGURATION_MODE))
    {
        C1CONHbits.REQOP = requestMode;

        while(C1CONHbits.OPMOD != requestMode) 
        {
            //This condition is avoiding the system error case endless loop
            if(C1INTLbits.SERRIF == 1)
            {
                status = CAN_OP_MODE_SYS_ERROR_OCCURED;
                break;
            }
        }
    }
    else
    {
        status = CAN_OP_MODE_REQUEST_FAIL;
    }
    
    return status;
}

CAN_OP_MODES CAN1_OperationModeGet(void) 
{
    return C1CONHbits.OPMOD;
}

bool CAN1_Receive(CAN_MSG_OBJ *rxCanMsg)
{
    uint8_t fifoChannel, count;
    CAN1_RX_FIFO_STATUS rxMsgStatus;
    CAN1_FIFO_INFO fifoInfo;
    bool status = false;
    
    //Iterate all receive FIFO's and read the message object
    for(count = 0; count < CAN1_NUM_OF_RX_FIFO; count++)
    {
        fifoChannel = rxFIFOMsg[count].channel;
        CAN1_FIFO_InfoGet(fifoChannel, &fifoInfo);
        rxMsgStatus = CAN1_RX_FIFO_StatusGet(fifoChannel);
        
        //If message object is available
        if(CAN_RX_MSG_AVAILABLE == (rxMsgStatus & CAN_RX_MSG_AVAILABLE)) 
        {           
            if(fifoInfo.address != NULL)
            {
                CAN1_MessageReadFromFifo((uint16_t *) *fifoInfo.address, rxCanMsg);
                CAN1_RX_FIFO_IncrementMsgPtr(fifoChannel);
                
                // Update the RX FIFO Head count for CAN1_ReceivedMessageCountGet function
                rxFIFOMsg[count].headCount += 1; //Update the read one message
                if(rxFIFOMsg[count].headCount >= fifoInfo.msgDeepSize) 
                {
                    rxFIFOMsg[count].headCount = 0; //Reset the read message count
                }
                                
                //User have to clear manually RX Overflow status
                if(CAN_RX_MSG_OVERFLOW == (rxMsgStatus & CAN_RX_MSG_OVERFLOW))
                {
                    CAN1_RX_FIFO_OverflowStatusFlagClear(fifoChannel);
                }
                
                status = true;
            }
            
            break;
        }
    }   
    
    return status;
}
uint8_t idx;
uint16_t FifoTable[24];
CAN_TX_MSG_REQUEST_STATUS CAN1_Transmit(const CAN1_TX_FIFO_CHANNELS fifoChannel, CAN_MSG_OBJ *txCanMsg)
{
    CAN1_FIFO_INFO fifoInfo;

    CAN1_FIFO_InfoGet(fifoChannel, &fifoInfo);
    //FifoTable[idx] = &fifoInfo.address;
    idx +=1;
    CAN_TX_MSG_REQUEST_STATUS txMsgStatus = CAN_TX_MSG_REQUEST_SUCCESS;
    
    // If CAN module is configured in Non-BRS mode and TX message object has BRS set
    if((txCanMsg->field.brs == 1) && (C1CONLbits.BRSDIS == 1))
    {
        txMsgStatus |= CAN_TX_MSG_REQUEST_BRS_ERROR;
    }
    
    // If CAN 2.0 mode, Tx Message object has more than 8 bytes of DLC Size
    // CAN 2.0 mode DLC supports upto 8 byte 
    if(txCanMsg->field.dlc > DLC_8) 
    {
       txMsgStatus |= CAN_TX_MSG_REQUEST_DLC_EXCEED_ERROR;
    }
    
    //If any CAN TX message object has DLC size more than CAN TX FIFO Payload size
    if(CAN1_DlcToDataBytesGet(txCanMsg->field.dlc) > fifoInfo.payloadSize) 
    {
        txMsgStatus |= CAN_TX_MSG_REQUEST_DLC_EXCEED_ERROR;
    }
    
    if(CAN_TX_MSG_REQUEST_SUCCESS == txMsgStatus)
    {
        if(CAN_TX_FIFO_AVAILABLE == CAN1_TransmitFIFOStatusGet(fifoChannel))
        {
            if(fifoInfo.address != NULL) 
            {
                CAN1_MessageWriteToFifo((uint16_t *) *fifoInfo.address, txCanMsg);
                CAN1_TX_FIFO_MessageSendRequest(fifoChannel);
            }         
        }
        else
        {
            txMsgStatus |= CAN_TX_MSG_REQUEST_FIFO_FULL;
        }
    }
    return txMsgStatus;
}

CAN_TX_FIFO_STATUS CAN1_TransmitFIFOStatusGet(const CAN1_TX_FIFO_CHANNELS fifoChannel)
{
    CAN_TX_FIFO_STATUS fifoStatus;
    
    switch (fifoChannel) 
    {
        case CAN1_TXQ:
            fifoStatus = ((C1TXQSTA & 0x1) ? CAN_TX_FIFO_AVAILABLE:CAN_TX_FIFO_FULL);
            break;
            
        case CAN1_FIFO_7:
            fifoStatus = ((C1FIFOSTA7 & 0x1) ? CAN_TX_FIFO_AVAILABLE:CAN_TX_FIFO_FULL);
            break;
            
        default:
            fifoStatus = CAN_TX_FIFO_FULL;
            break;
    }
    
    return fifoStatus;
}


uint8_t CAN1_ReceivedMessageCountGet(void)
{
    uint8_t fifoChannel = 0, count = 0, numOfMsg = 0, totalMsgObj = 0;
    CAN1_RX_FIFO_STATUS rxMsgStatus;
    CAN1_FIFO_INFO fifoInfo;
    uint16_t rxfifoMsgTail;
    
    //Iterate all receive FIFO's and get the message object count
    for(count = 0; count < CAN1_NUM_OF_RX_FIFO; count++)
    {
        fifoChannel = rxFIFOMsg[count].channel;
        CAN1_FIFO_InfoGet(fifoChannel, &fifoInfo);
        rxMsgStatus = CAN1_RX_FIFO_StatusGet(fifoChannel);
        
        //If message object is available
        if(CAN_RX_MSG_AVAILABLE == (rxMsgStatus & CAN_RX_MSG_AVAILABLE)) 
        {
            //If receive FIFO overflow has occurred, FIFO is full 
            if(CAN_RX_MSG_OVERFLOW == (rxMsgStatus & CAN_RX_MSG_OVERFLOW))
            {
                numOfMsg = fifoInfo.msgDeepSize;
            } 
            else
            {
                rxfifoMsgTail = CAN1_RX_FIFO_MessageIndexGet(fifoChannel);
                
                if(rxfifoMsgTail < rxFIFOMsg[count].headCount)
                {
                    numOfMsg = ((rxfifoMsgTail + fifoInfo.msgDeepSize) - rxFIFOMsg[count].headCount);
                }
                else if(rxfifoMsgTail > rxFIFOMsg[count].headCount)
                {
                    numOfMsg = rxfifoMsgTail - rxFIFOMsg[count].headCount;
                }
                else
                {
                    numOfMsg = fifoInfo.msgDeepSize;
                }
            }
            
            totalMsgObj += numOfMsg;
        }
    }
    
    return totalMsgObj;
}

bool CAN1_IsBusOff(void)
{
    return C1TRECHbits.TXBO;
}

bool CAN1_IsRxErrorPassive(void)
{
    return C1TRECHbits.RXBP;
}

bool CAN1_IsRxErrorWarning(void)
{
    return C1TRECHbits.RXWARN;
}

bool CAN1_IsRxErrorActive(void)
{
    bool errorState = false;
    if((0 < C1TRECLbits.RERRCNT) && (C1TRECLbits.RERRCNT < 128)) 
    {
        errorState = true;
    }
    
    return errorState;
}

bool CAN1_IsTxErrorPassive(void) 
{
    return C1TRECHbits.TXBP;
}

bool CAN1_IsTxErrorWarning(void) 
{
    return C1TRECHbits.TXWARN;
}

bool CAN1_IsTxErrorActive(void)
{
    bool errorState = false;
    if((0 < C1TRECLbits.TERRCNT) && (C1TRECLbits.TERRCNT < 128)) 
    {
        errorState = true;
    }
    
    return errorState;
}

void CAN1_Sleep(void)
{
    C1INTLbits.WAKIF = 0;
    C1INTHbits.WAKIE = 1;
    
    
    /* put the module in disable mode */
    CAN1_OperationModeSet(CAN_DISABLE_MODE);
}

void __attribute__((weak)) CAN1_DefaultInvalidMessageHandler(void) 
{

}

void __attribute__((weak)) CAN1_DefaultBusWakeUpActivityHandler(void) 
{

}

void __attribute__((weak)) CAN1_DefaultBusErrorHandler(void) 
{

}

void __attribute__((weak)) CAN1_DefaultModeChangeHandler(void) 
{

}

void __attribute__((weak)) CAN1_DefaultSystemErrorHandler(void) 
{

}

void __attribute__((weak)) CAN1_DefaultTxAttemptHandler(void) 
{

}

void __attribute__((weak)) CAN1_DefaultRxBufferOverFlowHandler(void) 
{

}

void CAN1_SetInvalidMessageInterruptHandler(void *handler)
{
    CAN1_InvalidMessageInterruptHandler = handler;
}

void CAN1_SetBusWakeUpActivityInterruptHandler(void *handler)
{
    CAN1_BusWakeUpActivityInterruptHandler = handler;
}

void CAN1_SetBusErrorInterruptHandler(void *handler)
{
    CAN1_BusErrorInterruptHandler = handler;
}

void CAN1_SetModeChangeInterruptHandler(void *handler)
{
    CAN1_ModeChangeInterruptHandler = handler;
}

void CAN1_SetSystemErrorInterruptHandler(void *handler)
{
    CAN1_SystemErrorInterruptHandler = handler;
}

void CAN1_SetTxAttemptInterruptHandler(void *handler)
{
    CAN1_TxAttemptInterruptHandler = handler;
}

void CAN1_SetRxBufferOverFlowInterruptHandler(void *handler)
{
    CAN1_RxBufferOverFlowInterruptHandler = handler;
}

void __attribute__((__interrupt__, no_auto_psv)) _C1Interrupt(void)
{
    if (C1INTLbits.IVMIF == 1)
    {
        if(CAN1_InvalidMessageInterruptHandler)
        {
            CAN1_InvalidMessageInterruptHandler();
        }

        C1INTLbits.IVMIF = 0;
    }

    if (C1INTLbits.WAKIF == 1) 
    {
        if(CAN1_BusWakeUpActivityInterruptHandler)
        {
            CAN1_BusWakeUpActivityInterruptHandler();
        }

        C1INTLbits.WAKIF = 0;
    }

    if (C1INTLbits.CERRIF == 1) 
    {
        if(CAN1_BusErrorInterruptHandler)
        {
            CAN1_BusErrorInterruptHandler();
        }

        C1INTLbits.CERRIF = 0;
    }

    if (C1INTLbits.MODIF == 1) 
    {
        if(CAN1_ModeChangeInterruptHandler)
        {
            CAN1_ModeChangeInterruptHandler();
        }

        C1INTLbits.MODIF = 0;
    }
    
    if (C1INTLbits.SERRIF == 1) 
    {
        if(CAN1_SystemErrorInterruptHandler)
        {
            CAN1_SystemErrorInterruptHandler();
        }

        C1INTLbits.SERRIF = 0;
    }
    
    if (C1INTLbits.TXATIF == 1) 
    {
        if(CAN1_TxAttemptInterruptHandler)
        {
            CAN1_TxAttemptInterruptHandler();
        }
    }
    
    if(C1INTLbits.RXOVIF == 1) 
    {
        if(CAN1_RxBufferOverFlowInterruptHandler)
        {
            CAN1_RxBufferOverFlowInterruptHandler();
        }
        
        if(C1FIFOSTA1bits.RXOVIF == 1)
        {
            C1FIFOSTA1bits.RXOVIF = 0;
        }
        
        if(C1FIFOSTA2bits.RXOVIF == 1)
        {
            C1FIFOSTA2bits.RXOVIF = 0;
        }
        
        if(C1FIFOSTA3bits.RXOVIF == 1)
        {
            C1FIFOSTA3bits.RXOVIF = 0;
        }
        
        if(C1FIFOSTA4bits.RXOVIF == 1)
        {
            C1FIFOSTA4bits.RXOVIF = 0;
        }
        
        if(C1FIFOSTA5bits.RXOVIF == 1)
        {
            C1FIFOSTA5bits.RXOVIF = 0;
        }
        
        if(C1FIFOSTA6bits.RXOVIF == 1)
        {
            C1FIFOSTA6bits.RXOVIF = 0;
        }
    }
    
    IFS1bits.C1IF = 0;
}

/**
 End of File
*/

