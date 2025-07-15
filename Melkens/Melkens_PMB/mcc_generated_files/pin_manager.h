#ifndef _PIN_MANAGER_H
#define _PIN_MANAGER_H

#include <xc.h>

#define CHAR_AN_SetHigh()          (_LATA0 = 1)
#define CHAR_AN_SetLow()           (_LATA0 = 0)
#define CHAR_AN_Toggle()           (_LATA0 ^= 1)
#define CHAR_AN_GetValue()         _RA0
#define CHAR_AN_SetDigitalInput()  (_TRISA0 = 1)
#define CHAR_AN_SetDigitalOutput() (_TRISA0 = 0)

#define StatSw4_SetHigh()          (_LATA1 = 1)
#define StatSw4_SetLow()           (_LATA1 = 0)
#define StatSw4_Toggle()           (_LATA1 ^= 1)
#define StatSw4_GetValue()         _RA1
#define StatSw4_SetDigitalInput()  (_TRISA1 = 1)
#define StatSw4_SetDigitalOutput() (_TRISA1 = 0)
#define StatSw4_SetAnalog()        (_ANSELA1 = 1)
#define StatSw4_DisableAnalog()    (_ANSELA1 = 0)

#define StatSw3_SetHigh()          (_LATA2 = 1)
#define StatSw3_SetLow()           (_LATA2 = 0)
#define StatSw3_Toggle()           (_LATA2 ^= 1)
#define StatSw3_GetValue()         _RA2
#define StatSw3_SetDigitalInput()  (_TRISA2 = 1)
#define StatSw3_SetDigitalOutput() (_TRISA2 = 0)
#define StatSw3_SetAnalog()        (_ANSELA2 = 1)
#define StatSw3_DisableAnalog()    (_ANSELA2 = 0)

#define StatSw2_SetHigh()          (_LATA3 = 1)
#define StatSw2_SetLow()           (_LATA3 = 0)
#define StatSw2_Toggle()           (_LATA3 ^= 1)
#define StatSw2_GetValue()         _RA3
#define StatSw2_SetDigitalInput()  (_TRISA3 = 1)
#define StatSw2_SetDigitalOutput() (_TRISA3 = 0)
#define StatSw2_SetAnalog()        (_ANSELA3 = 1)
#define StatSw2_DisableAnalog()    (_ANSELA3 = 0)

#define StatSw1_SetHigh()          (_LATA4 = 1)
#define StatSw1_SetLow()           (_LATA4 = 0)
#define StatSw1_Toggle()           (_LATA4 ^= 1)
#define StatSw1_GetValue()         _RA4
#define StatSw1_SetDigitalInput()  (_TRISA4 = 1)
#define StatSw1_SetDigitalOutput() (_TRISA4 = 0)
#define StatSw1_SetAnalog()        (_ANSELA4 = 1)
#define StatSw1_DisableAnalog()    (_ANSELA4 = 0)

#define Warning_Light_SetHigh()          (_LATB10 = 1)
#define Warning_Light_SetLow()           (_LATB10 = 0)
#define Warning_Light_Toggle()           (_LATB10 ^= 1)
#define Warning_Light_GetValue()         _RB10
#define Warning_Light_SetDigitalInput()  (_TRISB10 = 1)
#define Warning_Light_SetDigitalOutput() (_TRISB10 = 0)

#define Char_Rst_SetHigh()          (_LATB11 = 1)
#define Char_Rst_SetLow()           (_LATB11 = 0)
#define Char_Rst_Toggle()           (_LATB11 ^= 1)
#define Char_Rst_GetValue()         _RB11
#define Char_Rst_SetDigitalInput()  (_TRISB11 = 1)
#define Char_Rst_SetDigitalOutput() (_TRISB11 = 0)

#define DC_STATUS_LS_SetHigh()          (_LATB2 = 1)
#define DC_STATUS_LS_SetLow()           (_LATB2 = 0)
#define DC_STATUS_LS_Toggle()           (_LATB2 ^= 1)
#define DC_STATUS_LS_GetValue()         _RB2
#define DC_STATUS_LS_SetDigitalInput()  (_TRISB2 = 1)
#define DC_STATUS_LS_SetDigitalOutput() (_TRISB2 = 0)

#define IM_SENSE_SetHigh()          (_LATB3 = 1)
#define IM_SENSE_SetLow()           (_LATB3 = 0)
#define IM_SENSE_Toggle()           (_LATB3 ^= 1)
#define IM_SENSE_GetValue()         _RB3
#define IM_SENSE_SetDigitalInput()  (_TRISB3 = 1)
#define IM_SENSE_SetDigitalOutput() (_TRISB3 = 0)

#define IO_RB4_SetHigh()          (_LATB4 = 1)
#define IO_RB4_SetLow()           (_LATB4 = 0)
#define IO_RB4_Toggle()           (_LATB4 ^= 1)
#define IO_RB4_GetValue()         _RB4
#define IO_RB4_SetDigitalInput()  (_TRISB4 = 1)
#define IO_RB4_SetDigitalOutput() (_TRISB4 = 0)

#define IO_RB5_SetHigh()          (_LATB5 = 1)
#define IO_RB5_SetLow()           (_LATB5 = 0)
#define IO_RB5_Toggle()           (_LATB5 ^= 1)
#define IO_RB5_GetValue()         _RB5
#define IO_RB5_SetDigitalInput()  (_TRISB5 = 1)
#define IO_RB5_SetDigitalOutput() (_TRISB5 = 0)

#define Soft_Start_EN_SetHigh()          (_LATB6 = 1)
#define Soft_Start_EN_SetLow()           (_LATB6 = 0)
#define Soft_Start_EN_Toggle()           (_LATB6 ^= 1)
#define Soft_Start_EN_GetValue()         _RB6
#define Soft_Start_EN_SetDigitalInput()  (_TRISB6 = 1)
#define Soft_Start_EN_SetDigitalOutput() (_TRISB6 = 0)

#define DC_STATUS_HS_SetHigh()          (_LATB7 = 1)
#define DC_STATUS_HS_SetLow()           (_LATB7 = 0)
#define DC_STATUS_HS_Toggle()           (_LATB7 ^= 1)
#define DC_STATUS_HS_GetValue()         _RB7
#define DC_STATUS_HS_SetDigitalInput()  (_TRISB7 = 1)
#define DC_STATUS_HS_SetDigitalOutput() (_TRISB7 = 0)

#define D_SetHigh()          (_LATB8 = 1)
#define D_SetLow()           (_LATB8 = 0)
#define D_Toggle()           (_LATB8 ^= 1)
#define D_GetValue()         _RB8
#define D_SetDigitalInput()  (_TRISB8 = 1)
#define D_SetDigitalOutput() (_TRISB8 = 0)

#define SafSwA_Rst_SetHigh()          (_LATC1 = 1)
#define SafSwA_Rst_SetLow()           (_LATC1 = 0)
#define SafSwA_Rst_Toggle()           (_LATC1 ^= 1)
#define SafSwA_Rst_GetValue()         _RC1
#define SafSwA_Rst_SetDigitalInput()  (_TRISC1 = 1)
#define SafSwA_Rst_SetDigitalOutput() (_TRISC1 = 0)

#define Status_SafSwA_SetHigh()          (_LATC2 = 1)
#define Status_SafSwA_SetLow()           (_LATC2 = 0)
#define Status_SafSwA_Toggle()           (_LATC2 ^= 1)
#define Status_SafSwA_GetValue()         _RC2
#define Status_SafSwA_SetDigitalInput()  (_TRISC2 = 1)
#define Status_SafSwA_SetDigitalOutput() (_TRISC2 = 0)

#define SafSwB_Rst_SetHigh()          (_LATC3 = 1)
#define SafSwB_Rst_SetLow()           (_LATC3 = 0)
#define SafSwB_Rst_Toggle()           (_LATC3 ^= 1)
#define SafSwB_Rst_GetValue()         _RC3
#define SafSwB_Rst_SetDigitalInput()  (_TRISC3 = 1)
#define SafSwB_Rst_SetDigitalOutput() (_TRISC3 = 0)

#define LED1_SetHigh()          (_LATC5 = 1)
#define LED1_SetLow()           (_LATC5 = 0)
#define LED1_Toggle()           (_LATC5 ^= 1)
#define LED1_GetValue()         _RC5
#define LED1_SetDigitalInput()  (_TRISC5 = 1)
#define LED1_SetDigitalOutput() (_TRISC5 = 0)

#define LED2_SetHigh()          (_LATC10 = 1)
#define LED2_SetLow()           (_LATC10 = 0)
#define LED2_Toggle()           (_LATC10 ^= 1)
#define LED2_GetValue()         _RC10
#define LED2_SetDigitalInput()  (_TRISC10 = 1)
#define LED2_SetDigitalOutput() (_TRISC10 = 0)

#define LED3_SetHigh()          (_LATC11 = 1)
#define LED3_SetLow()           (_LATC11 = 0)
#define LED3_Toggle()           (_LATC11 ^= 1)
#define LED3_GetValue()         _RC11
#define LED3_SetDigitalInput()  (_TRISC11 = 1)
#define LED3_SetDigitalOutput() (_TRISC11 = 0)

#define EN_SafSwB_SetHigh()          (_LATC6 = 1)
#define EN_SafSwB_SetLow()           (_LATC6 = 0)
#define EN_SafSwB_Toggle()           (_LATC6 ^= 1)
#define EN_SafSwB_GetValue()         _RC6
#define EN_SafSwB_SetDigitalInput()  (_TRISC6 = 1)
#define EN_SafSwB_SetDigitalOutput() (_TRISC6 = 0)

#define BAT_STATUS_SetHigh()          (_LATC7 = 1)
#define BAT_STATUS_SetLow()           (_LATC7 = 0)
#define BAT_STATUS_Toggle()           (_LATC7 ^= 1)
#define BAT_STATUS_GetValue()         _RC7
#define BAT_STATUS_SetDigitalInput()  (_TRISC7 = 1)
#define BAT_STATUS_SetDigitalOutput() (_TRISC7 = 0)

#define DBG1_SetHigh()          (_LATD0 = 1)
#define DBG1_SetLow()           (_LATD0 = 0)
#define DBG1_Toggle()           (_LATD0 ^= 1)
#define DBG1_GetValue()         _RD0
#define DBG1_SetDigitalInput()  (_TRISD0 = 1)
#define DBG1_SetDigitalOutput() (_TRISD0 = 0)
#define DBG1_EnablePullup()     (CNPUDbits.CNPUD0 = 1)

#define DBG2_SetHigh()          (_LATD2 = 1)
#define DBG2_SetLow()           (_LATD2 = 0)
#define DBG2_Toggle()           (_LATD2 ^= 1)
#define DBG2_GetValue()         _RD2
#define DBG2_SetDigitalInput()  (_TRISD2 = 1)
#define DBG2_SetDigitalOutput() (_TRISD2 = 0)
#define DBG2_EnablePullup()     (CNPUDbits.CNPUD2 = 1)

#define DBG3_SetHigh()          (_LATD3 = 1)
#define DBG3_SetLow()           (_LATD3 = 0)
#define DBG3_Toggle()           (_LATD3 ^= 1)
#define DBG3_GetValue()         _RD3
#define DBG3_SetDigitalInput()  (_TRISD3 = 1)
#define DBG3_SetDigitalOutput() (_TRISD3 = 0)

#define DBG4_SetHigh()          (_LATD4 = 1)
#define DBG4_SetLow()           (_LATD4 = 0)
#define DBG4_Toggle()           (_LATD4 ^= 1)
#define DBG4_GetValue()         _RD4
#define DBG4_SetDigitalInput()  (_TRISD4 = 1)
#define DBG4_SetDigitalOutput() (_TRISD4 = 0)

#define Buzzer_SetHigh()          (_LATD1 = 1)
#define Buzzer_SetLow()           (_LATD1 = 0)
#define Buzzer_Toggle()           (_LATD1 ^= 1)
#define Buzzer_GetValue()         _RD1
#define Buzzer_SetDigitalInput()  (_TRISD1 = 1)
#define Buzzer_SetDigitalOutput() (_TRISD1 = 0)

#define EN_Char_SetHigh()          (_LATD10 = 1)
#define EN_Char_SetLow()           (_LATD10 = 0)
#define EN_Char_Toggle()           (_LATD10 ^= 1)
#define EN_Char_GetValue()         _RD10
#define EN_Char_SetDigitalInput()  (_TRISD10 = 1)
#define EN_Char_SetDigitalOutput() (_TRISD10 = 0)

#define Status_SafSwB_SetHigh()          (_LATD11 = 1)
#define Status_SafSwB_SetLow()           (_LATD11 = 0)
#define Status_SafSwB_Toggle()           (_LATD11 ^= 1)
#define Status_SafSwB_GetValue()         _RD11
#define Status_SafSwB_SetDigitalInput()  (_TRISD11 = 1)
#define Status_SafSwB_SetDigitalOutput() (_TRISD11 = 0)

#define EN_SafSwA_SetHigh()          (_LATD12 = 1)
#define EN_SafSwA_SetLow()           (_LATD12 = 0)
#define EN_SafSwA_Toggle()           (_LATD12 ^= 1)
#define EN_SafSwA_GetValue()         _RD12
#define EN_SafSwA_SetDigitalInput()  (_TRISD12 = 1)
#define EN_SafSwA_SetDigitalOutput() (_TRISD12 = 0)

#define IO_RD5_SetHigh()          (_LATD5 = 1)
#define IO_RD5_SetLow()           (_LATD5 = 0)
#define IO_RD5_Toggle()           (_LATD5 ^= 1)
#define IO_RD5_GetValue()         _RD5
#define IO_RD5_SetDigitalInput()  (_TRISD5 = 1)
#define IO_RD5_SetDigitalOutput() (_TRISD5 = 0)

void PIN_MANAGER_Initialize (void);

#endif
