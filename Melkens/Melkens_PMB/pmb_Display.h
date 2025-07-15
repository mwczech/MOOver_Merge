#include <xc.h> // include processor files - each processor file is guarded. 

typedef enum DisplayButton_t{
    Display_UP = 0,
    Display_DOWN,
    Display_LEFT,
    Display_RIGHT,
    Display_SLIDER_WHEELS,
    Display_SLIDER_THUMBLE,
    Display_BARREL_STOP,
    Display_BARREL_FORWARD,
    Display_BARREL_REVERSE,
    Display_EMERGANCY_STOP,
    Display_PLAY,
    Display_PAUSE,
    Display_ENABLE_POWER,
    Display_DISABLE_POWER,
    Display_ENABLE_CHARGER,
    Display_DISABLE_CHARGER,
    Display_Lift_UP,
    Display_Lift_DOWN,
    Display_Lift_STOP,
    Display_UPPER_BELT_ON,
    Display_UPPER_BELT_OFF,
    Display_LOWER_BELT_ON,
    Display_LOWER_BELT_OFF,
    Display_SLIDER_UPPER_BELT,
    Display_SLIDER_LOWER_BELT,
    Display_ROUTE_A,
    Display_ROUTE_B,
    Display_ROUTE_C,
    Display_ROUTE_D,
    Display_ROUTE_E,
    Display_ROUTE_F,
    Display_ROUTE_G,
    Display_ROUTE_H,
    Display_ROUTE_I,
    Display_ROUTE_J,
    Display_ROUTE_K, /* Important! Route enums should be ALWAYS before Remote_Released enum */
    Display_Released
            
}DisplayButton;

void Display_SendData(void);
void Read_Data_Display(void);
DisplayButton Display_GetEvent(void);
void Display_ClearEvent(void);
void CalculateAnalogRealValues(void);