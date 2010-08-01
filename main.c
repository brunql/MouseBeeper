/*
 * main.c
 *
 *  Created on: 31.07.2010
 *      Author: brunql
 */

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/atomic.h>
#include <stdlib.h>

#include "main.h"
#include "bos.h"
#include "74HC595.h"



#define CODE_NUM_CLEAR	16

const uint8_t seg7_digits[] = {
		CODE_0, CODE_1, CODE_2, CODE_3, CODE_4, CODE_5, CODE_6, CODE_7, CODE_8, CODE_9,
		CODE_A, CODE_B, CODE_C, CODE_D, CODE_E, CODE_F,
		/* 16 */ CODE_CLEAR
};

volatile uint8_t showme = 0x00;
volatile uint8_t first_or_second = 0x00;

volatile uint8_t beep_on = 0x00;
volatile uint8_t display7seg_on = 0x00;
volatile uint8_t int0_flag = 0x00;
volatile uint8_t int1_flag = 0x00;
volatile uint8_t melody = 0x01;
volatile uint8_t melody_temp = 0x00;

volatile uint8_t sec_to_min = 60;


void Task_7SegDisplay(void);

static inline void BeeperON(void){  beep_on = 0xff; }
static inline void BeeperOFF(void){ beep_on = 0x00; }

static inline void Display7SegON(void)
{
	OS_AddTaskToEvalQueue( Task_7SegDisplay );
	display7seg_on = 0xff;
}
static inline void Display7SegOFF(void)
{
	display7seg_on = 0x00;
}



ISR( TiMER_INTERRUPT_VECTOR )
{
	OS_SystemTimerTick();
	SystemTimer_ResetCounter();
}


void Task_7SegDisplay(void)
{
	if(first_or_second == 1){
		first_or_second = 2;

		Display7SegCommon_OFF();
		HC595_PutUInt8( 0xff );

		Display7SegCommon_Out1();
		HC595_PutUInt8( ~seg7_digits[(showme & 0x0f)] );
	}else{
		first_or_second = 1;

		Display7SegCommon_OFF();
		HC595_PutUInt8( 0xff );

		Display7SegCommon_Out2();
		HC595_PutUInt8( ~seg7_digits[(showme & 0xf0)>>4] );
	}

	if(display7seg_on){
		OS_AddTaskToTimerQueue(Task_7SegDisplay, 5);
	}
}

void Task_Countdown(void)
{
	if(beep_on){
		if(++melody == 15){
			melody = 1;
		}
	}else{
		if(--sec_to_min == 0x00){
			sec_to_min = 60;
			if(--showme == 0x00){
				beep_on = 0xff;
			}
		}
	}
}

void Task_Beep(void)
{
	if(beep_on){
//		if(--melody_temp == 0x00){
//			melody_temp = melody;

			if(PINA & _BV(PA0)){
				PORTA &=(uint8_t)~_BV(PA0);
				DDRA &=(uint8_t)~_BV(PA0);
			}else{
				PORTA |=_BV(PA0);
				DDRA |= _BV(PA0);
			}
//		}
	}
}

void Task_SwitchOffAll(void)
{
	showme = 0x00;

	BeeperOFF();
	Display7SegOFF();
	Display7SegCommon_OFF();
	HC595_PutUInt8( 0xff );
}

void Task_LittleButtonClicked(void)
{
	if(PIND & _BV(PD2)){
		OS_AddTaskToEvalQueue( Task_SwitchOffAll );
	}

	int0_flag = 0;
	GIMSK |= _BV(INT0);
	EIFR = _BV(INTF0);
}

void Task_MouseWheel(void)
{
	if(!(PIND & _BV(PD3))){
		if(PIND & _BV(PD4)){
			showme++;
		}else{
			showme--;
		}

		Display7SegON();
		BeeperOFF();
	}

	int1_flag = 0;
	GIMSK |= _BV(INT1);
	EIFR = _BV(INTF1);
}


ISR( INT0_vect )
{
	GIMSK &=(uint8_t)~_BV(INT0);
	if(! int0_flag){
		int0_flag = 1;
		OS_AddTaskToTimerQueue( Task_LittleButtonClicked, 10 );
	}
	EIFR = _BV(INTF0);
}

ISR( INT1_vect )
{
	GIMSK &=(uint8_t)~_BV(INT1);
	if(! int1_flag){
		int1_flag = 1;
		OS_AddTaskToTimerQueue( Task_MouseWheel, 10 );
	}
	EIFR = _BV(INTF1);
}


static inline void GPIO_Iint(void)
{
	PORTA = 0x00;
	DDRA = _BV(PA0); // Beeper

	PORTB = 0x00;
	DDRB = 0x00;

	PORTD = 0x00;
	DDRD = 0x00;

	ACSR = _BV(ACD); // Analog comparator off

	MCUCR = _BV(ISC00) | _BV(ISC10); // INT0,1 log change
	GIMSK = _BV(INT0) | _BV(INT1);
}


int main(void)
{
	GPIO_Iint();
	Display7SegCommon_Init();
	HC595_Init();

	OS_Iinialize();
	OS_InitSystemTimerAndSei();

	for(;;){
		OS_EvalTask();
	}

	return 93; // not reachable
}

