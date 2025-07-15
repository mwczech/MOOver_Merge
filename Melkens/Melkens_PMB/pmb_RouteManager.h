#define dROUTE_IDLE 255

void RouteManager_Init(void);
void RouteManager_StateMachine(void);
void RouteManager_Perform1ms( void );
void RouteManager_Perform100ms(void);
void RouteManager_ResetRouteSettings();

void RouteManager_GetTimingFromDisplay(uint8_t route, uint8_t Hour_1, uint8_t Minute_1, uint8_t Hour_2, uint8_t Minute_2, uint8_t Hour_3, uint8_t Minute_3, uint8_t Hour_4, uint8_t Minute_4);

void Check_Scheduling_Times(void);
uint8_t Check_On_Time(uint8_t h_on, uint8_t m_on);
uint8_t ReleaseSchedulerAllowance(uint8_t h_on, uint8_t m_on);
int16_t MotorManager_GetAngle(void);
uint8_t RouteManager_GetCurrentRouteStep(void);
void RouteManager_SetStepRequest(uint8_t Step);
void RouteManager_SendCurrentRouteStep(void);