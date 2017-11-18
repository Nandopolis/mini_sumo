/*
 * bsp.c
 *
 * Created: 11/1/2017 9:58:29 PM
 *  Author: Nandopolis
 */ 

#define F_CPU 16000000UL

#include <avr/io.h>
#include "bsp.h"
#include "sumo.h"

void BSP_InitGPIO() {
	// set I/O pins
	DDRB = (1<<A_IN_1) | (1<<B_IN_2) | (1<<LED);
	DDRD = (1<<A_IN_2) | (1<<B_IN_1);

	// set pull up resistors
	PORTD = (1<<DS_L) | (1<<DS_R);
	PORTB = (1<<PBUT);

	// set pin change interrupt
	PCICR = (1<<PCIE1) | (1<<PCIE0);
	PCMSK0 = (1<<PBUT);
	PCMSK1 = (1<<ES_L) | (1<<ES_R);
}

void BSP_InitTimer0() {
	// set pwm pins
	DDRD |= (1<<PWM_A_PIN) | (1<<PWM_B_PIN);

	// set phase correct pwm mode, no prescaler
	TCCR0A = (1<<COM0A1) | (1<<COM0B1) | (1<<WGM00);
	TCCR0B = (1<<CS00);

	// clear pwm
	PWM_A = 0x00;
	PWM_B = 0x00;
}

void BSP_InitADC() {
	PRR &= ~(1<<PRADC);
	ADMUX = (1<<REFS0) | (1<<ADLAR);
	ADCSRA = (1<<ADEN) | (1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0);
	DIDR0 = (1<<ADC2D) | (1<<ADC1D) | (1<<ADC0D);
}

void BSP_SetTimer1(uint16_t ms) {
	TIMSK1 = (1<<TOIE1);
	TIFR1 = (1<<TOV1);
	TCNT1 = 65535 - (125 * ms) / 2;
	TCCR1B = (1<<CS12);
}

void BSP_ToggleLed() {
	PINB |= (1<<LED);
}

void BSP_LedOn() {
	PORTB |= (1<<LED);
}

void BSP_LedOff() {
	PORTB &= ~(1<<LED);
}

void BSP_Stop() {
	PORTD |= (1<<A_IN_2);
	PORTB |= (1<<A_IN_1);
	PORTB |= (1<<B_IN_2);
	PORTD |= (1<<B_IN_1);
	PWM_A = 0x00;
	PWM_B = 0x00;
}

void BSP_GoForward() {
	PORTD |= (1<<A_IN_2);
	PORTB &= ~(1<<A_IN_1);
	PORTB |= (1<<B_IN_2);
	PORTD &= ~(1<<B_IN_1);
	PWM_A = 0xFF;
	PWM_B = 0xC0;
}

void BSP_GoBack() {
	PORTD &= ~(1<<A_IN_2);
	PORTB |= (1<<A_IN_1);
	PORTB &= ~(1<<B_IN_2);
	PORTD |= (1<<B_IN_1);
	PWM_A = 0xFF;
	PWM_B = 0xC0;
}

void BSP_TurnRight() {
	PORTD |= (1<<A_IN_2);
	PORTB &= ~(1<<A_IN_1);
	PORTB &= ~(1<<B_IN_2);
	PORTD |= (1<<B_IN_1);
	PWM_A = 0xFF;
	PWM_B = 0xFF;
}

void BSP_TurnLeft() {
	PORTD &= ~(1<<A_IN_2);
	PORTB |= (1<<A_IN_1);
	PORTB |= (1<<B_IN_2);
	PORTD &= ~(1<<B_IN_1);
	PWM_A = 0xFF;
	PWM_B = 0xFF;
}

void BSP_TurnSpeed(uint8_t offset, uint8_t dir) {
	uint16_t b_speed;
	uint8_t diff;

	diff = offset * 5;
	b_speed = 0xFF - (diff & ~dir);
	b_speed = b_speed * 3 / 4;
	PORTD |= (1<<A_IN_2);
	PORTB &= ~(1<<A_IN_1);
	PORTB |= (1<<B_IN_2);
	PORTD &= ~(1<<B_IN_1);
	PWM_A = 0xFF - (diff & dir);
	PWM_B = (uint8_t)b_speed;
}

void BSP_ReadMode(Sumo * const me) {
	uint8_t read;
	read = PIND & ((1<<DS_L) | (1<<DS_R));
	read = read>>2;
	me->combat_mode = read;
}

uint8_t BSP_ReadOpSensors(Sumo * const me) {
	uint8_t i, index = 0, active = 0;
	uint8_t static const min_range[] = {28, 39, 28};

	ADMUX &= 0xF0; 
	for (i = 0; i < 3; i++) {
		ADCSRA |= (1<<ADSC);
		while (ADCSRA & (1<< ADSC));
		if (ADCH > active) {
			active = ADCH;
			index = i;
		}
		ADMUX++;
	}
	if (active > min_range[index]) {
		QACTIVE_POST(&(me->super), LEFT_SIG + index, active);
		return 0;
	}
	return 1;
}
