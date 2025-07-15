/*
 * File:   pmb_Keyboard.c
 * Author: Jaca
 *
 * Created on March 29, 2023, 10:33 PM
 */


#include "xc.h"
#include <string.h>
#include <stdio.h>

#include "mcc_generated_files/uart1.h"

#include "pmb_Keyboard.h"

KeyboardEvent KeyboardGlobalEvent;
uint8_t readBuffer_Control_Panel[8];

void Read_Data_Keyboard(void){
    /*READ DATA FROM KEYBOARD (UART1)*/
    if(UART1_ReadBuffer(&readBuffer_Control_Panel[0], 8)== 8){
        /*BUTTON UP EVENTS*/  
        if(readBuffer_Control_Panel[2]=='1'){
            KeyboardGlobalEvent.Button = Keyboard_UP;
            if(readBuffer_Control_Panel[4]=='S')
                KeyboardGlobalEvent.PressTime = Keyboard_ShortPress;
            }else if(readBuffer_Control_Panel[4]=='L'){
                KeyboardGlobalEvent.PressTime = Keyboard_LongPress;   
            }
        /*BUTTON DOWN EVENTS*/  
        if(readBuffer_Control_Panel[2]=='3'){
            KeyboardGlobalEvent.Button = Keyboard_DOWN;
            if(readBuffer_Control_Panel[4]=='S')
                KeyboardGlobalEvent.PressTime = Keyboard_ShortPress;
            }else if(readBuffer_Control_Panel[4]=='L'){
                KeyboardGlobalEvent.PressTime = Keyboard_LongPress;   
            }
        /*BUTTON LEFT EVENTS*/  
        if(readBuffer_Control_Panel[2]=='2'){
            KeyboardGlobalEvent.Button = Keyboard_LEFT;
            if(readBuffer_Control_Panel[4]=='S')
                KeyboardGlobalEvent.PressTime = Keyboard_ShortPress;
            }else if(readBuffer_Control_Panel[4]=='L'){
                KeyboardGlobalEvent.PressTime = Keyboard_LongPress;   
            }
        /*BUTTON RIGHT EVENTS*/  
        if(readBuffer_Control_Panel[2]=='0'){
            KeyboardGlobalEvent.Button = Keyboard_RIGHT;
            if(readBuffer_Control_Panel[4]=='S')
                KeyboardGlobalEvent.PressTime = Keyboard_ShortPress;
            }else if(readBuffer_Control_Panel[4]=='L'){
                KeyboardGlobalEvent.PressTime = Keyboard_LongPress;   
            }
}
}

KeyboardEvent Keyboard_GetEvent(void){
    return KeyboardGlobalEvent;
}

void Keyboard_ClearEvent(void){
    KeyboardGlobalEvent.Button = Keyboard_Released;
}