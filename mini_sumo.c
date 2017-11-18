/*
 * mini_sumo.c
 *
 * Created: 10/31/2017 2:33:39 AM
 *  Author: Nandopolis
 */ 

#define F_CPU 16000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include "qpn.h"
#include "bsp.h"
#include "sumo.h"

Sumo AO_Sumo;
static QEvt l_sumoQSto[20];

QActiveCB const Q_ROM QF_active[] = {
	{ (QActive *)0,         (QEvt *)0,      0U                },
	{ (QActive *)&AO_Sumo,  l_sumoQSto,     Q_DIM(l_sumoQSto) }
};

int main(void)
{
	QF_init(Q_DIM(QF_active));
	QActive_ctor(&AO_Sumo.super, Q_STATE_CAST(&Sumo_initial));

    while(1)
    {
        //TODO:: Please write your application code 
		QF_run();
    }
}

//============================================================================
// interrupts...
ISR(TIMER2_COMPA_vect) {
	QF_tickXISR(0); // process time events for tick rate 0
}

ISR(TIMER1_OVF_vect) {
	TCCR1B &= ~(1<<CS12);
	TIMSK1 &= ~(1<<TOIE1);
	QACTIVE_POST_ISR(&AO_Sumo.super, TIMER1OVF_SIG, 0);
}

ISR(PCINT0_vect) {
	if (~(PINB & (1<<PBUT))) {
		QACTIVE_POST_ISR(&AO_Sumo.super, BUTTON_SIG, 0);
	}
}

ISR(PCINT1_vect) {
	uint8_t line;

	line = (PINC & ((1<<ES_L) | (1<<ES_R)))>>4;
	if (line < 3) {
		AO_Sumo.last_edge = line;
		QACTIVE_POST_ISR(&AO_Sumo.super, EDGE_SIG, 0);
	} else {
		QACTIVE_POST_ISR(&AO_Sumo.super, INSIDE_SIG, 0);
	}
}

//============================================================================
// QF callbacks...
void QF_onStartup(void) {
	// set Timer2 in CTC mode, 1/1024 prescaler, start the timer ticking...
	TCCR2A = (1U << WGM21) | (0U << WGM20);
	TCCR2B = (1U << CS22 ) | (1U << CS21) | (1U << CS20); // 1/2^10
	ASSR  &= ~(1U << AS2);
	TIMSK2 = (1U << OCIE2A); // enable TIMER2 compare Interrupt
	TCNT2  = 0U;

	// set the output-compare register based on the desired tick frequency
	OCR2A  = (F_CPU / BSP_TICKS_PER_SEC / 1024U) - 1U;

	BSP_InitGPIO();
	BSP_InitTimer0();
	BSP_InitADC();
}
//............................................................................
void QV_onIdle(void) {   // called with interrupts DISABLED
	// Put the CPU and peripherals to the low-power mode. You might
	// need to customize the clock management for your application,
	// see the datasheet for your particular AVR MCU.
	SMCR = (0 << SM0) | (1 << SE); // idle mode, adjust to your project
	QV_CPU_SLEEP();  // atomically go to sleep and enable interrupts
}
//............................................................................
void Q_onAssert(char const Q_ROM * const file, int line) {
	// implement the error-handling policy for your application!!!
	QF_INT_DISABLE(); // disable all interrupts
	QF_RESET();  // reset the CPU
}