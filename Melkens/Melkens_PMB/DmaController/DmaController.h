/* 
 * File:   DmaController.h
 * Author: pawelton
 *
 * Created on 9 czerwca 2023, 12:10
 */

#ifndef DMACONTROLLER_H
#define	DMACONTROLLER_H

#include <xc.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#ifdef	__cplusplus
extern "C" {
#endif
    
typedef enum 
{
    DMA_CHANNEL_0 =  0,       
    DMA_CHANNEL_1 =  1,       
    DMA_CHANNEL_2 =  2,       
    DMA_CHANNEL_3 =  3,       
    DMA_NUMBER_OF_CHANNELS = 4
} DMA_CHANNEL;


/**
  Section: Interface Routines
*/

/**
  @Summary
    This function initializes DMA instance : 1

  @Description
    This routine initializes the DMA driver instance for : 1
    index, making it ready for clients to open and use it. It also initializes any
    internal data structures.
    This routine must be called before any other DMA routine is called. 

  @Preconditions
    None.

  @Param
    None.

  @Returns
    None.

  @Comment
    
 
  @Example
    <code>
        unsigned short int srcArray[100];
        unsigned short int dstArray[100];
        int i;
        int count;
        for (i=0; i<100; i++)
        {
            srcArray[i] = i+1;
            dstArray[i] = 0;
        }
        
        DMA_Initialize();
        DMA_SoftwareTriggerEnable(CHANNEL1);
        
        count = DMA_TransferCountGet;
        while(count > 0)
        {
        while(DMA_IsSoftwareRequestPending(CHANNEL1));
        DMA_SoftwareTriggerEnable(CHANNEL1);
        }
    </code>

*/
void DMA_Initialize(void);

/**
 @Summary
  Enables the channel in the DMA

 @Description
  This routine is used to enable a channel in the DMA. This routine
  sets the value of the CHEN bit to 1.
 
 @Preconditions
  DMA_Initializer() function should have been 
  called before calling this function.
 
 @Returns
  None

 @Param
  None
  
 @Example
 Refer to DMA_Initializer(); for an example
 */
void DMA_ChannelEnable(DMA_CHANNEL  channel);

/**
 @Summary
 Disables the channel in the DMA

 @Description
  This routine is used to disable a channel in the DMA. This routine
  sets the value of the CHEN bit to 0.
 
@Preconditions
 DMA_Initializer() function should have been 
 called before calling this function.
 
@Returns
 None

@Param
 None
  
@Example
 Refer to DMA_Initializer(); for an example
*/
void DMA_ChannelDisable(DMA_CHANNEL  channel);

/**
  @Summary
    Sets the transfer count of the DMA

  @Description
    This routine is used to set the DMA transfer count. This routine sets the
    value of the DMACNT register. 
 
  @Preconditions
    DMA_Initializer() function should have been 
    called before calling this function.
 
  @Returns
    None

  @Param
    None
  
  @Example
    Refer to DMA_Initializer(); for an example
 */
void DMA_TransferCountSet(DMA_CHANNEL channel, uint16_t transferCount);

/**
  @Summary
    Returns the transfer count of the DMA

  @Description
    This routine is used to determine the DMA transfer count. This routine
    returns the value of the DMACNT register. 
 
  @Preconditions
    DMA_Initializer() function should have been 
    called before calling this function.
 
  @Returns
    Returns the transfer count of the DMA

  @Param
    None
  
  @Example
    Refer to DMA_Initializer(); for an example
 */
uint16_t DMA_TransferCountGet(DMA_CHANNEL channel);

/**
  @Summary
    Enables the software trigger of the DMA

  @Description
    This routine is used to enable the software trigger of the DMA. This routine
    sets the value of the CHREQ bit to 1.
 
  @Preconditions
    DMA_Initializer() function should have been 
    called before calling this function.
 
  @Returns
    None

  @Param
    None
  
  @Example
    Refer to DMA_Initializer(); for an example
 */
void DMA_SoftwareTriggerEnable(DMA_CHANNEL channel);
/**
  @Summary
    Sets the source address for the DMA

  @Description
    This routine is used to set the source address for a DMA channel. 
 
  @Preconditions
    DMA_Initializer() function should have been 
    called before calling this function.
 
  @Returns
    None

  @Param
    None
  
  @Example
    Refer to DMA_Initializer(); for an example
 */
void DMA_SourceAddressSet(DMA_CHANNEL channel, uint16_t);
/**
  @Summary
    Sets the destination address for the DMA

  @Description
    This routine is used to set the destination address for a DMA channel. 
 
  @Preconditions
    DMA_Initializer() function should have been 
    called before calling this function.
 
  @Returns
    None

  @Param
    None
  
  @Example
    Refer to DMA_Initializer(); for an example
 */
void DMA_DestinationAddressSet(DMA_CHANNEL channel, uint16_t);
/**
  @Summary
    Returns true when the software DMA request is completed

  @Description
    This routine is used to determine if the software DMA request is completed. This routine
    returns the value of the CHREQ bit. When DMA request is complete the routine
    returns 1. It returns 0 otherwise.
 
  @Preconditions
    DMA_Initializer() function should have been 
    called before calling this function.
 
  @Returns
    Returns true if software request is completed

  @Param
    None
  
  @Example
    Refer to DMA_Initializer(); for an example
 */
bool DMA_IsSoftwareRequestPending(DMA_CHANNEL channel);

/**
  @Summary
    Returns true when the buffered DMA write is completed

  @Description
    This routine is used to determine if the buffered DMA write is completed. This routine
    returns the value of the DBUFWF bit. When buffered DMA write is complete the routine
    returns 1. It returns 0 otherwise.
 
  @Preconditions
    DMA_Initializer() function should have been 
    called before calling this function.
 
  @Returns
    Returns true if buffered DMA write is completed

  @Param
    None
  
  @Example
    Refer to DMA_Initializer(); for an example
 */
bool DMA_IsBufferedWriteComplete(DMA_CHANNEL channel);

/**
  @Summary
    Returns true when the high address limit flag is set

  @Description
    This routine is used to determine if the DMA channel has tried to access an
    address higher than the DMAH limit. This routine returns the value of the 
    HIGHIF bit. When the high address limit has been crossed the routine
    returns 1. It returns 0 otherwise.
 
  @Preconditions
    DMA_Initializer() function should have been 
    called before calling this function.
 
  @Returns
    Returns true if high address limit has been crossed.

  @Param
    None
  
  @Example
    Refer to DMA_Initializer(); for an example
 */
bool DMA_IsHighAddressLimitFlagSet(DMA_CHANNEL channel);

/**
  @Summary
    Returns true when the low address limit flag is set

  @Description
    This routine is used to determine if the DMA channel has tried to access an
    address lower than the DMAL limit. This routine returns the value of the 
    LOWIF bit. When the low address limit has been crossed the routine
    returns 1. It returns 0 otherwise.
 
  @Preconditions
    DMA_Initializer() function should have been 
    called before calling this function.
 
  @Returns
    Returns true if the low address limit has been crossed.

  @Param
    None
  
  @Example
    Refer to DMA_Initializer(); for an example
 */
bool DMA_IsLowAddressLimitFlagSet(DMA_CHANNEL channel);

/**
  @Summary
    Returns true when the operation is done.

  @Description
    This routine is used to determine if the DMA operation is done.. This routine 
    returns the value of the DONEIF bit. When the operation is done the routine 
    returns 1. It returns 0 otherwise.
 
  @Preconditions
    DMA_Initializer() function should have been 
    called before calling this function.
 
  @Returns
    Returns true if the DMA operation is done.

  @Param
    None
  
  @Example
    Refer to DMA_Initializer(); for an example
 */
bool DMA_IsOperationDone(DMA_CHANNEL channel);

/**
  @Summary
    Returns true when a DMA overrun has occured

  @Description
    This routine is used to determine if a DMA overrun has occured. This routine 
    returns the value of the OVRUNIF bit. When a DMA overrun has occurred the routine
    returns 1. It returns 0 otherwise.
 
  @Preconditions
    DMA_Initializer() function should have been 
    called before calling this function.
 
  @Returns
    Returns true if a DMA overrun has occurred. 

  @Param
    None
  
  @Example
    Refer to DMA_Initializer(); for an example
 */
bool DMA_IsOverrunFlagSet(DMA_CHANNEL channel);

/**
  @Summary
    Returns true when the operation is halfway done

  @Description
    This routine is used to determine if the DMA operation is 50% complete. This 
    routine returns the value of the HALFIF bit. When the DMA operation is halfway 
    done the routine returns 1. It returns 0 otherwise.
 
  @Preconditions
    DMA_Initializer() function should have been 
    called before calling this function.
 
  @Returns
    Returns true if the operation is halfway completed

  @Param
    None
  
  @Example
    Refer to DMA_Initializer(); for an example
 */
bool DMA_IsOperationHalfComplete(DMA_CHANNEL channel);

/**
  @Summary
    Callback for DMA Channel0.

  @Description
    This routine is callback for DMA Channel0

  @Param
    None.

  @Returns
    None
 
  @Example 
	Refer to DMA_Initialize(); for an example
*/
void DMA_Channel0_CallBack(void);


/**
  @Summary
    Callback for DMA Channel1.

  @Description
    This routine is callback for DMA Channel1

  @Param
    None.

  @Returns
    None
 
  @Example 
	Refer to DMA_Initialize(); for an example
*/
void DMA_Channel1_CallBack(void);

/**
  @Summary
    Polled implementation

  @Description
    This routine is used to implement the tasks for polled implementations.
  
  @Preconditions
    DMA_Initialize() function should have been 
    called before calling this function.
 
  @Returns 
    None
 
  @Param
    None
 
  @Example
    Refer to DMA_Initialize(); for an example
    
*/
void DMA_Channel1_Tasks(void);

/**
  @Summary
    Callback for DMA Channel2.

  @Description
    This routine is callback for DMA Channel2

  @Param
    None.

  @Returns
    None
 
  @Example 
	Refer to DMA_Initialize(); for an example
*/
void DMA_Channel2_CallBack(void);

/**
  @Summary
    Polled implementation

  @Description
    This routine is used to implement the tasks for polled implementations.
  
  @Preconditions
    DMA_Initialize() function should have been 
    called before calling this function.
 
  @Returns 
    None
 
  @Param
    None
 
  @Example
    Refer to DMA_Initialize(); for an example
    
*/
void DMA_Channel2_Tasks(void);

/**
  @Summary
    Callback for DMA Channel3.

  @Description
    This routine is callback for DMA Channel3

  @Param
    None.

  @Returns
    None
 
  @Example 
	Refer to DMA_Initialize(); for an example
*/
void DMA_Channel3_CallBack(void);

/**
  @Summary
    Polled implementation

  @Description
    This routine is used to implement the tasks for polled implementations.
  
  @Preconditions
    DMA_Initialize() function should have been 
    called before calling this function.
 
  @Returns 
    None
 
  @Param
    None
 
  @Example
    Refer to DMA_Initialize(); for an example
    
*/
void DMA_Channel3_Tasks(void);

void DmaController_SetDestinationAddress(uint16_t Address, DMA_CHANNEL Channel);
void DmaController_SetSourceAddress(uint16_t Address, DMA_CHANNEL Channel);
bool DMA_IsTransferComplete(DMA_CHANNEL Channel);
void DMA_ResetTransferStatus(DMA_CHANNEL Channel);

#ifdef	__cplusplus
}
#endif

#endif	/* DMACONTROLLER_H */

