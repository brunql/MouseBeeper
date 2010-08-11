/*
 * bos_hal.h - Brunql real time OS. Headers for hal.
 *
 *  Created on: 23.01.2010
 *      Author: brunql
 */

#ifndef BOS_HAL_H_
#define BOS_HAL_H_

#include <avr/io.h>
#include <avr/interrupt.h> // for sei();
#include "bos.h"

#define TiMER_INTERRUPT_VECTOR	TIMER0_OVF_vect

static inline void SystemTimer_Init(void)
{
	TCCR0A = 0x00;
	TCCR0B = (0<<CS02) | (1<<CS01) | (0<<CS00);  /* (F_CPU / 256) / 8 == each 250us == 2048 ticks */
	TCNT0 = 0x00;
}


static inline void SystemTimer_InterrupEnable(void)
{
	TIMSK = _BV( TOIE0 );
}

static inline void SystemTimer_ResetCounter(void){
	/* TCNT1 = 0; */
}


static inline void OS_InitSystemTimerAndSei(void)
{
	SystemTimer_Init();
	SystemTimer_InterrupEnable();
	sei();
}

#endif /* BOS_HAL_H_ */
