#include "string.h"
#include "../DmaController/DmaController.h"


typedef struct DmaChannelAddress_t
{
    uint16_t SourceAddress;
    uint16_t DestinationAddress;
    bool     IsTransferComplete;
    
}DmaChannelAddress;

DmaChannelAddress DMAChannel[DMA_NUMBER_OF_CHANNELS];

void DmaController_SetDestinationAddress(uint16_t Address, DMA_CHANNEL Channel)
{
    DMAChannel[Channel].DestinationAddress = Address;
}

void DmaController_SetSourceAddress(uint16_t Address, DMA_CHANNEL Channel)
{
    DMAChannel[Channel].SourceAddress = Address;
}



/**
  Prototype:        void DMA_Initialize(void)
  Input:            none
  Output:           none
  Description:      DMA_Initialize is an
                    initialization routine that takes inputs from the GUI.
  Comment:          
  Usage:            DMA_Initialize();
 */
void DMA_Initialize(void) 
{ 
    // DMAEN enabled; PRSSEL Fixed priority; 
    DMACON = (0x8000 | 0x01) & 0x7FFF; //Enable DMA later
    // LADDR 4096; 
    DMAL= 0xF00;
    // HADDR 20479; 
    DMAH= 0x4FFF;

    /* ======================== DMA CHANEL 0 CONFIGURATION =============== */
    // CHEN enabled; DAMODE Incremented; TRMODE Continuous; CHREQ disabled; RELOAD enabled; SIZE 8 bit; NULLW disabled; SAMODE Unchanged; 
    DMACH0 = 0x243 & 0xFFFE; //Enable DMA Channel later;
    // HALFIF disabled; LOWIF disabled; HALFEN disabled; DONEIF disabled; OVRUNIF disabled; CHSEL UART3 TX; HIGHIF disabled; 
    DMAINT0= 0x6900;
    
    DMAChannel[DMA_CHANNEL_0].IsTransferComplete = false;
    // SADDR TransmitBufferEncoder array
    //DMASRC0= IMUHandler_GetTransmitBufferAddress();
    DMASRC0 = DMAChannel[DMA_CHANNEL_0].SourceAddress;
    // DADDR UART3TX
    //DMADST0= 0xF10;
    DMADST0 = DMAChannel[DMA_CHANNEL_0].DestinationAddress;
    // CNT 16; 
    DMACNT0= 0x10;
    // Clearing Channel 0 Interrupt Flag;
    IFS0bits.DMA0IF = false;
    // Enabling Channel 0 Interrupt
    IEC0bits.DMA0IE = true;

    /* ======================== DMA CHANEL 1 CONFIGURATION =============== */

    // CHEN disabled; SAMODE Unchanged; SIZE 8 bit; DAMODE Unchanged; CHREQ disabled; RELOAD disabled; TRMODE One-Shot; NULLW disabled; 
    DMACH1 = 0x217 & 0xFFFE; //Enable DMA Channel later;
    // HALFIF disabled; LOWIF disabled; HALFEN disabled; DONEIF disabled; OVRUNIF disabled; HIGHIF disabled; CHSEL INT0; 
    //TRANSFER SOURCE: RX3; DONEIF: ENABLED
    DMAINT1= 0x6800;
    
    DMAChannel[DMA_CHANNEL_1].IsTransferComplete = false;
    // SADDR UART3RX
    //DMASRC1= 0xF0C;
    DMASRC1 = DMAChannel[DMA_CHANNEL_1].SourceAddress;
    // DADDR ReceiveBufferEncoder array
    //DMADST1= Address_DMA1;
    DMADST1 = DMAChannel[DMA_CHANNEL_1].DestinationAddress;
    // CNT 8; "GET_ENCO" 
    DMACNT1= 0x08; 
    // Clearing Channel 1 Interrupt Flag;
    IFS0bits.DMA1IF = false;
    IEC0bits.DMA1IE = true;

    // SAMODE Unchanged; CHEN disabled; SIZE 16 bit; DAMODE Unchanged; CHREQ disabled; RELOAD disabled; NULLW disabled; TRMODE One-Shot; 
    DMACH2 = 0x00 & 0xFFFE; //Enable DMA Channel later;
    // HALFIF disabled; LOWIF disabled; HALFEN disabled; DONEIF disabled; OVRUNIF disabled; HIGHIF disabled; CHSEL INT0; 
    DMAINT2= 0x00;
    // SADDR 0; 
    DMASRC2= 0x00;
    // DADDR 0; 
    DMADST2= 0x00;
    // CNT 0; 
    DMACNT2= 0x00;
    // Clearing Channel 2 Interrupt Flag;
    IFS1bits.DMA2IF = false;

    // SAMODE Unchanged; CHEN disabled; SIZE 16 bit; DAMODE Unchanged; TRMODE One-Shot; NULLW disabled; CHREQ disabled; RELOAD disabled; 
    DMACH3 = 0x00 & 0xFFFE; //Enable DMA Channel later;
    // HALFIF disabled; LOWIF disabled; HALFEN disabled; DONEIF disabled; OVRUNIF disabled; CHSEL INT0; HIGHIF disabled; 
    DMAINT3= 0x00;
    // SADDR 0; 
    DMASRC3= 0x00;
    // DADDR 0; 
    DMADST3= 0x00;
    // CNT 0; 
    DMACNT3= 0x00;
    // Clearing Channel 3 Interrupt Flag;
    IFS1bits.DMA3IF = false;
    
    //Enable DMA
    DMACONbits.DMAEN = 1;
    DMA_ChannelEnable(DMA_CHANNEL_1);
    //DMA_ChannelEnable(DMA_CHANNEL_0);
    
}

void __attribute__ ((weak)) DMA_Channel0_CallBack(void)
{
    // Add your custom callback code here
    __asm("nop");
}

void __attribute__ ( ( interrupt, no_auto_psv ) ) _DMA0Interrupt( void )
{
	// DMA Channel0 callback function 
	DMA_Channel0_CallBack();
	
    IFS0bits.DMA0IF = 0;
}

void __attribute__ ( ( interrupt, no_auto_psv ) ) _DMA1Interrupt( void )
{
	// DMA Channel0 callback function 
    IFS0bits.DMA1IF = 0;
	DMA_Channel1_CallBack();
}
void __attribute__ ((weak)) DMA_Channel1_CallBack(void)
{  
    /* Message with specified length has been received */
    DMAChannel[DMA_CHANNEL_1].IsTransferComplete = true;
    DMACH1bits.CHEN = 0;
    // Add your custom callback code here
}

void DMA__Channel1_Tasks( void )
{
	if(IFS0bits.DMA1IF)
	{
		// DMA Channel1 callback function
		IFS0bits.DMA1IF = 0;
		DMA_Channel1_CallBack();
	}
}
void __attribute__ ((weak)) DMA_Channel2_CallBack(void)
{
    // Add your custom callback code here
}

void DMA__Channel2_Tasks( void )
{
	if(IFS1bits.DMA2IF)
	{
		// DMA Channel2 callback function 
		DMA_Channel2_CallBack();
		IFS1bits.DMA2IF = 0;
	}
}
void __attribute__ ((weak)) DMA_Channel3_CallBack(void)
{
    // Add your custom callback code here
}

void DMA__Channel3_Tasks( void )
{
	if(IFS1bits.DMA3IF)
	{
		// DMA Channel3 callback function 
		DMA_Channel3_CallBack();
		
		IFS1bits.DMA3IF = 0;
	}
}

void DMA_ChannelEnable(DMA_CHANNEL  channel)
{
    switch(channel) {
        case DMA_CHANNEL_0:
                DMACH0bits.CHEN = 1;
                break; 
        case DMA_CHANNEL_1:
                DMACH1bits.CHEN = 1;
                break; 
        case DMA_CHANNEL_2:
                DMACH2bits.CHEN = 1;
                break; 
        case DMA_CHANNEL_3:
                DMACH3bits.CHEN = 1;
                break; 
        default: break;
    }
}
void DMA_ChannelDisable(DMA_CHANNEL  channel)
{
    switch(channel) {
        case DMA_CHANNEL_0:
                DMACH0bits.CHEN = 0;
                break;    
        case DMA_CHANNEL_1:
                DMACH1bits.CHEN = 0;
                break;    
        case DMA_CHANNEL_2:
                DMACH2bits.CHEN = 0;
                break;    
        case DMA_CHANNEL_3:
                DMACH3bits.CHEN = 0;
                break;    
        default: break;
    }
}
void DMA_TransferCountSet(DMA_CHANNEL channel, uint16_t transferCount)
{
    switch(channel) {
        case DMA_CHANNEL_0:
                DMACNT0 = transferCount;
                break;
        case DMA_CHANNEL_1:
                DMACNT1 = transferCount;
                break;
        case DMA_CHANNEL_2:
                DMACNT2 = transferCount;
                break;
        case DMA_CHANNEL_3:
                DMACNT3 = transferCount;
                break;
        default: break;
    }
}
uint16_t DMA_TransferCountGet(DMA_CHANNEL channel)
{
    switch(channel) {
        case DMA_CHANNEL_0:
                return (DMACNT0);
        case DMA_CHANNEL_1:
                return (DMACNT1);
        case DMA_CHANNEL_2:
                return (DMACNT2);
        case DMA_CHANNEL_3:
                return (DMACNT3);
        default: return 0;
    }
}
void DMA_SoftwareTriggerEnable(DMA_CHANNEL channel )
{
    switch(channel) {
        case DMA_CHANNEL_0:
                DMACH0bits.CHREQ = 1;
                break;
        case DMA_CHANNEL_1:
                DMACH1bits.CHREQ = 1;
                break;
        case DMA_CHANNEL_2:
                DMACH2bits.CHREQ = 1;
                break;
        case DMA_CHANNEL_3:
                DMACH3bits.CHREQ = 1;
                break;
        default: break;
    }
}

void DMA_SourceAddressSet(DMA_CHANNEL  channel, uint16_t address) {
    switch(channel) {
        case DMA_CHANNEL_0:
                DMASRC0 = address;
                break;
        case DMA_CHANNEL_1:
                DMASRC1 = address;
                break;
        case DMA_CHANNEL_2:
                DMASRC2 = address;
                break;
        case DMA_CHANNEL_3:
                DMASRC3 = address;
                break;
        default: break;
    }    
}

void DMA_DestinationAddressSet(DMA_CHANNEL  channel, uint16_t address) {
    switch(channel) {
        case DMA_CHANNEL_0:
                DMADST0 = address;
                break;
        case DMA_CHANNEL_1:
                DMADST1 = address;
                break;
        case DMA_CHANNEL_2:
                DMADST2 = address;
                break;
        case DMA_CHANNEL_3:
                DMADST3 = address;
                break;
        default: break;
    }    
}

bool DMA_IsSoftwareRequestPending(DMA_CHANNEL  channel)
{
    switch(channel) {
        case DMA_CHANNEL_0:
                return(DMACH0bits.CHREQ);
        case DMA_CHANNEL_1:
                return(DMACH1bits.CHREQ);
        case DMA_CHANNEL_2:
                return(DMACH2bits.CHREQ);
        case DMA_CHANNEL_3:
                return(DMACH3bits.CHREQ);
        default: return 0;
    }
}

bool DMA_IsBufferedWriteComplete(DMA_CHANNEL channel )
{
    switch(channel) {
        case DMA_CHANNEL_0:
                return(DMAINT0bits.DBUFWF);
        case DMA_CHANNEL_1:
                return(DMAINT1bits.DBUFWF);
        case DMA_CHANNEL_2:
                return(DMAINT2bits.DBUFWF);
        case DMA_CHANNEL_3:
                return(DMAINT3bits.DBUFWF);
        default: return 0;
    }
}

bool DMA_IsHighAddressLimitFlagSet(DMA_CHANNEL channel )
{
    switch(channel) {
        case DMA_CHANNEL_0:
                return(DMAINT0bits.HIGHIF);
        case DMA_CHANNEL_1:
                return(DMAINT1bits.HIGHIF);
        case DMA_CHANNEL_2:
                return(DMAINT2bits.HIGHIF);
        case DMA_CHANNEL_3:
                return(DMAINT3bits.HIGHIF);
        default: return 0;
    }
}

bool DMA_IsLowAddressLimitFlagSet(DMA_CHANNEL channel)
{
    switch(channel) {
        case DMA_CHANNEL_0:
                return(DMAINT0bits.LOWIF);
        case DMA_CHANNEL_1:
                return(DMAINT1bits.LOWIF);
        case DMA_CHANNEL_2:
                return(DMAINT2bits.LOWIF);
        case DMA_CHANNEL_3:
                return(DMAINT3bits.LOWIF);
        default: return 0;
    }
}

bool DMA_IsOperationDone(DMA_CHANNEL channel)
{
    switch(channel) {
        case DMA_CHANNEL_0:
                return(DMAINT0bits.DONEIF);
        case DMA_CHANNEL_1:
                return(DMAINT1bits.DONEIF);
        case DMA_CHANNEL_2:
                return(DMAINT2bits.DONEIF);
        case DMA_CHANNEL_3:
                return(DMAINT3bits.DONEIF);
        default: return 0;
    }
}

bool DMA_IsOverrunFlagSet(DMA_CHANNEL channel )
{
    switch(channel) {
        case DMA_CHANNEL_0:
                return(DMAINT0bits.OVRUNIF);
        case DMA_CHANNEL_1:
                return(DMAINT1bits.OVRUNIF);
        case DMA_CHANNEL_2:
                return(DMAINT2bits.OVRUNIF);
        case DMA_CHANNEL_3:
                return(DMAINT3bits.OVRUNIF);
        default: return 0;
    }
}

bool DMA_IsOperationHalfComplete(DMA_CHANNEL channel)
{
    switch(channel) {
        case DMA_CHANNEL_0:
                return(DMAINT0bits.HALFIF);
        case DMA_CHANNEL_1:
                return(DMAINT1bits.HALFIF);
        case DMA_CHANNEL_2:
                return(DMAINT2bits.HALFIF);
        case DMA_CHANNEL_3:
                return(DMAINT3bits.HALFIF);
        default: return 0;
    }
}

bool DMA_IsTransferComplete(DMA_CHANNEL Channel)
{
    return DMAChannel[Channel].IsTransferComplete;
}

void DMA_ResetTransferStatus(DMA_CHANNEL Channel)
{
    DMAChannel[Channel].IsTransferComplete = false;
}