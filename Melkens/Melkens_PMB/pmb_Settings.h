
#include "pmb_System.h"


/* Powyzej jakiej wartosci (w stopniach) ma byc dokrecanie w trybie NORM  */
#define dCORRECTION_ANGLE_THRESHOLD 0.5f

/* Dokladnosc w stopniach podczas skrecania (przy 90 stopniach wystarczy osiagnac 85 zeby zakonczyc krok) */
#define d90DEG_OFFSET 0
#define d45DEG_OFFSET 0

/* Kroki trasy ktore ma poszerzac */
#define dROUTE_A_FEEDING_TABLE_1_STEP 2
#define dROUTE_A_FEEDING_TABLE_2_STEP 6

/* O ile cnt ma poszerzac trase */
#define dFEEDING_TABLE_OFFSET 20
#define dROUTE_REPEAT_COUNT 5

/* Turn 90 degrees */
#define dANGLE_IN_90_TURN 90

/* Simulated 45 degrees */
#define dANGLE_IN_45_TURN 45

/* IMU influence on step finish decision, default 50% IMU, 50% encoders */
#define dIMU_JUDGEMENT_FACTOR 1.0f
#define dECODER_JUDGEMENT_FACTOR (1.0f - dIMU_JUDGEMENT_FACTOR)

/* After achieving 120% of encode value, step should be broken */
#define dENCODER_STEP_MAX_MULTIPLIER 1.5f

/* Prawy i lewy motor - domyslna predkosc */
#define DEFAULT_SPEED 500

/* lift motor - domyslna predkosc */
#define DEFAULT_SPEED_LIFT 500

/* lift motor - domyslna predkosc */
#define DEFAULT_SPEED_BELT 500


#if COMPILE_SWITCH_MOONION
/* zagarniacz - domyslna predkosc */
    #define DEFAULT_SPEED_THUMBLE 200
#else
/* Slimak - domyslna predkosc */
    #define DEFAULT_SPEED_THUMBLE 1500
#endif

/* Wal - liczba countow na pelen obrot silnika*/
#define ENCODER_MAX_VALUE 10000

/* centymetry przebyta na pelen obrot silnika*/
#if COMPILE_SWITCH_MOONION
#define dDISTANCE_PER_MOTOR_ROTATION 0.67f
#else
#define dDISTANCE_PER_MOTOR_ROTATION 1.26f
#endif

#define dMAGNETS_DEBOUCE_CNT 5U
#define dMAGNETS_ACTIVE_THRESHOLD 10U

#define dMAGNETS_DISTANCE_BETWEEN_SENSORS  8.0f

#define dPI 3.1416d

#define dMIDDLE_MAGNET_INDEX 15
#define dMAGNET_BAR_LENGTH 65.0f
#define dMAGNET_BAR_VIRTUAL_STEP dMAGNET_BAR_LENGTH / 60.0f
#define dMAGNET_NO_DETECTION 123.0f

#define dDEBUG_DISPLAY_MAGNETS
#define dDEBUG_POWER_RAILS_ALWAYS_ON

#define dDEBUG_ENABLE_BUZZER_ROUTE

#define dMOTOR_WHEEL_DISCONNECT_TIMEOU_MS 3000
#define dIMU_DISCONNECT_TIMEOUT_MS        5000
#define dMAGNETS_DISCONNECT_TIMEOUT_MS        5000

#if COMPILE_SWITCH_MOONION
    #define dBATTERY_OVERVOLTAGE 5200 /* default: 5200 - 52V */
    #define dBATTERY_LOW_VOLTAGE 4600 /* default 4600 - 46V */
    #define dBATTERU_CRITICAL_VOLTAGE 4400 /* default 4400 - 44V */
    #define dBATTERY_HYSTERESIS 20 /* 200mV hysteresis for calculating battery level */
#else
    #define dBATTERY_OVERVOLTAGE 3000 /* default: 2600 - 26V */
    #define dBATTERY_LOW_VOLTAGE 2200 /* default 2300 - 23V */
    #define dBATTERU_CRITICAL_VOLTAGE 2100 /* default 2300 - 22V */
    #define dBATTERY_HYSTERESIS 50 /* 500mV hysteresis for calculating battery level */
#endif
