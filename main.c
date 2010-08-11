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

#define TASK_COUNTDOWN_DELAY	247

#define CODE_NUM_CLEAR	16
const uint8_t seg7_digits[] = {
		CODE_0, CODE_1, CODE_2, CODE_3, CODE_4, CODE_5, CODE_6, CODE_7, CODE_8, CODE_9,
		CODE_A, CODE_B, CODE_C, CODE_D, CODE_E, CODE_F,
		/* 16 */ CODE_CLEAR
};


volatile uint8_t flags = 0x00;

// Time to switch beep on, show it on 7-seg display
volatile uint8_t showme = 0x00;

#define BEEP_OFF	8
#define MELODY_LENGTH 12
volatile uint8_t melody[MELODY_LENGTH] = {
		1,2,3,
		1,1,8,
		2,2,8,
		1,1,8,
};
volatile uint8_t melody_indx = 0x00;

#define TICKS_COUNT_IN_MINUTE 	16UL*60
volatile uint16_t ticks_in_minute = TICKS_COUNT_IN_MINUTE;




ISR( TiMER_INTERRUPT_VECTOR )
{
	OS_SystemTimerTick();
	SystemTimer_ResetCounter();
}


void Task_7SegDisplay(void)
{
	if(FlagOn(DISPLAY_ON)){
		if(FlagOn(DISPLAY_FIRST_OR_SECOND)){
			FlagClear(DISPLAY_FIRST_OR_SECOND);

			Display7SegCommon_OFF();
			HC595_PutUInt8( 0xff );

			Display7SegCommon_Out1();
			HC595_PutUInt8( ~seg7_digits[(showme & 0x0f)] );
		}else{
			FlagSet(DISPLAY_FIRST_OR_SECOND);

			Display7SegCommon_OFF();
			HC595_PutUInt8( 0xff );

			Display7SegCommon_Out2();
			HC595_PutUInt8( ~seg7_digits[(showme & 0xf0)>>4] );
		}
		OS_AddTaskToTimerQueue(Task_7SegDisplay, 5);
	}
}

void Task_Beep(void)
{
	if(FlagOn(BEEP_ON)){
		if(BeeperPin()){
			BeeperPin_Down();
		}else{
			if(melody[melody_indx] != BEEP_OFF){
				BeeperPin_Up();
			}
		}
		OS_AddTaskToTimerQueue(Task_Beep, melody[melody_indx]);
	}
}

void Task_BeepStop(void)
{
	FlagClear(BEEP_ON);
	BeeperPin_Down();
}

void BeepOnClick(uint8_t melody_index)
{
	if(FlagOff(BEEP_ON)){
		FlagSet(BEEP_ON); // set flag

		melody_indx = melody_index; // switch melody

		OS_AddTaskToEvalQueue(Task_Beep); // add task to start beeper
		OS_AddTaskToTimerQueue(Task_BeepStop, 240); // add task to stop in 100 ticks
	}else{
		FlagClear(BEEP_ON);
	}
}

void Task_Countdown(void)
{
	if(FlagOn(COUNTDOWN_ON)){
		if(FlagOff(BEEP_ON)){
			if(--ticks_in_minute == 0){
				ticks_in_minute = TICKS_COUNT_IN_MINUTE;
				if((showme == 0) || (--showme == 0)){
					OS_AddTaskToEvalQueue(Task_Beep);
					FlagSet(BEEP_ON);
				}
			}
		}else{
			if(++melody_indx == MELODY_LENGTH){
				melody_indx = 0;
			}
		}
		OS_AddTaskToTimerQueue(Task_Countdown, TASK_COUNTDOWN_DELAY);
	}
}

void Task_LittleButtonClicked(void)
{
	if(PIND & _BV(PD2)){
		showme = 0x00;

		BeepOnClick(1);

		FlagClear(DISPLAY_ON);
		FlagClear(COUNTDOWN_ON);

		Display7SegCommon_OFF();
		HC595_PutUInt8( 0xff );
	}

	FlagClear(INT0_PROCESSED);
	EIFR = _BV(INTF0);
	GIMSK |= _BV(INT0);
}

void Task_MouseWheel(void)
{
	// Encoder
	if(!(PIND & _BV(PD3))){
		if(PIND & _BV(PD4)){
			showme++;
		}else{
			showme--;
		}
		ticks_in_minute = TICKS_COUNT_IN_MINUTE;

		Task_BeepStop();

		if(FlagOff(DISPLAY_ON)){ // if not already added to eval queue
			FlagSet(DISPLAY_ON);
			OS_AddTaskToEvalQueue( Task_7SegDisplay );
		}

		if(FlagOff(COUNTDOWN_ON)){
			FlagSet(COUNTDOWN_ON);
			OS_AddTaskToEvalQueue( Task_Countdown );
		}
	}

	FlagClear(INT1_PROCESSED);
	EIFR = _BV(INTF1);
	GIMSK |= _BV(INT1);
}


ISR( INT0_vect )
{
	GIMSK &=(uint8_t)~_BV(INT0);
	if(FlagOff(INT0_PROCESSED) && FlagOff(INT1_PROCESSED)){
		FlagOn(INT0_PROCESSED);
		OS_AddTaskToTimerQueue( Task_LittleButtonClicked, 2 );
	}
	EIFR = _BV(INTF0);
}

ISR( INT1_vect )
{
	GIMSK &=(uint8_t)~_BV(INT1);
	if(FlagOff(INT0_PROCESSED) && FlagOff(INT1_PROCESSED)){
		FlagOn(INT1_PROCESSED);
		OS_AddTaskToTimerQueue( Task_MouseWheel, 2 );
	}
	EIFR = _BV(INTF1);
}



static inline void GPIO_Iint(void)
{
	PORTA = 0x00;
	DDRA = 0x00;

	PORTB = 0x00;
	DDRB = 0x00;

	PORTD = 0x00;
	DDRD = 0x00;

	ACSR = _BV(ACD); // Analog comparator off

	MCUCR = _BV(ISC00) | _BV(ISC10); // INT0,1 log change
	GIMSK = _BV(INT0) | _BV(INT1); // INT0 - little button, INT1 - encoder (mouse wheel)
}


int main(void)
{
	GPIO_Iint();
	BeeperPin_Init();
	Display7SegCommon_Init();
	HC595_Init();

	OS_Iinialize();
	OS_InitSystemTimerAndSei();

	for(;;){
		OS_EvalTask();
	}

	return 93; // not reachable
}

