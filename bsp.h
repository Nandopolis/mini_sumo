#ifndef bsp_h
#define bsp_h

#define PWM_A OCR0A
#define PWM_B OCR0B

#include "sumo.h"

enum {
	BSP_TICKS_PER_SEC = 200,
	RIGHT = 0,
	LEFT = 255,
	A_IN_1 = 1,
	A_IN_2 = 4,
	PWM_A_PIN = 6,
	B_IN_1 = 7,
	B_IN_2 = 0,
	PWM_B_PIN = 5,
	ES_R = 4,
	ES_L = 5,
	DS_R = 2,
	DS_L = 3,
	PBUT = 2,
	LED = 5
};

void BSP_InitGPIO();
void BSP_InitTimer0();
void BSP_InitADC();

void BSP_SetTimer1(uint16_t ms);

void BSP_ToggleLed();
void BSP_LedOn();
void BSP_LedOff();

void BSP_Stop();
void BSP_GoForward();
void BSP_GoBack();
void BSP_TurnRight();
void BSP_TurnLeft();
void BSP_TurnSpeed(uint8_t offset, uint8_t dir);

void BSP_ReadMode(Sumo * const me);
uint8_t BSP_ReadOpSensors(Sumo * const me);

#endif /* bsp_h */