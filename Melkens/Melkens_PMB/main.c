/**
  Generated main.c file from MPLAB Code Configurator

  @Company
    Microchip Technology Inc.

  @File Name
    main.c

  @Summary
    This is the generated main.c using PIC24 / dsPIC33 / PIC32MM MCUs.

  @Description
    This source file provides main entry point for system initialization and application code development.
    Generation Information :
        Product Revision  :  PIC24 / dsPIC33 / PIC32MM MCUs - 1.171.1
        Device            :  dsPIC33CK256MP506
    The generated drivers are tested against the following:
        Compiler          :  XC16 v1.70
        MPLAB 	          :  MPLAB X v5.50
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
#include "main.h"

#include "mcc_generated_files/system.h"
#include "mcc_generated_files/interrupt_manager.h"
#include "mcc_generated_files/pwm.h"
#include "mcc_generated_files/pin_manager.h"

#include "TimeManager/TimeManager.h"
#include "IMUHandler/IMUHandler.h"
#include "AnalogHandler/AnalogHandler.h"
#include "BatteryManager/BatteryManager.h"

#include "pmb_RouteManager.h"
#include "DriveIndicator.h"
#include "pmb_CAN.h"
#include "pmb_Display.h"
#include "pmb_Keyboard.h"
#include "DiagnosticsHandler.h"
#include "pmb_MotorManager.h"
#include "pmb_RouteManager.h"
#include "pmb_System.h"
#include "pmb_Scheduler.h"

volatile uint8_t Counter500ms = 10;

int main(void)
{
    SYSTEM_Initialize();
    PMB_Initialize();

    CAN1_OperationModeSet(CAN_NORMAL_2_0_MODE);
    PWM_DutyCycleSet(SAFETY_PWM, 50);

    PWM_DutyCycleSet(LIFT, 50);
    
    PWM_ModuleDisable(LIFT);

    
    TimeManager_Init();
    IMUHandler_Init();
    AnalogHandler_Init();
    MotorManager_Initialise();
    DriveIndicator_Init();
    BatteryManager_ResetBattery(); 
    Scheduler_Init();
    #if COMPILE_SWITCH_MOONION 

    DBG1_SetLow();
    DBG2_SetLow();
    DBG3_SetLow();
    DBG4_SetLow();
    
    #else

    DBG1_SetDigitalInput();
    DBG1_EnablePullup();//lower lift switch 1-open 0-closed

    DBG2_SetDigitalInput();
    DBG2_EnablePullup();//upper lift switch 1-open 0-closed
    
    DBG4_SetDigitalInput();
    DBG3_SetDigitalOutput();
    
    #endif
    
    LED1_SetLow();
    LED2_SetLow();
    LED3_SetLow();
    
    System_PowerRailRequestSequence(Sequence_PowerStageOn);//enable power
    
    while (1){
        if(TimeManager_Is1msPassed()){
            System_Perform1ms();
            DriveIndicator_1msPerform();
            //RouteManager_Perform1ms();
            IMUHandler_Perform1ms();
            
            MotorManager_Perform1ms();
            CAN_Polling();
        }
        if(TimeManager_Is10msPassed()){
            //RouteManager_SendCurrentRouteStep();
            Read_Data_Keyboard();
            //Read_Data_Display();
            //Display_SendData();
        }
     
        if(TimeManager_Is100msPassed()){
            BatteryManager_Perform100ms();
            //RouteManager_Perform100ms();
            MotorManager_Perform100ms();
            AnalogHandler_Perform100ms();
            Diagnostics_Perform100ms();
            CalculateAnalogRealValues(); 
            if( !MotorManager_IsMotorEnabled(Motor_Thumble)){
                IMUHandler_SetThumbleCurrent(0);
            }
            if(Counter500ms == 0){
                //LED2_Toggle();
                Counter500ms = 5;
            }
            else{
                Counter500ms -= 1;
            }
        }
        
        if(TimeManager_Is1sPassed()){
            //Scheduler_Perform1s();
            //LED2_Toggle();
        }
        /* Route and motor state machines */    
        //RouteManager_StateMachine();
        MotorManager_StateMachine();
        MotorManager_PerformAfterMainLoop();

        /* Clear display and keyboard events after all states machine handles it */
        //Display_ClearEvent();
        Keyboard_ClearEvent();
        Remote_ClearEvent();

        INTERRUPT_GlobalDisable();
	    TimeManager_UpdateFlags();
        INTERRUPT_GlobalEnable();
    }
    return 1; 
}
/**
 End of File
*/

