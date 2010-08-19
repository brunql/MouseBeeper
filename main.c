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

const uint8_t seg7_digits[] = {
		CODE_0, CODE_1, CODE_2, CODE_3, CODE_4, CODE_5, CODE_6, CODE_7, CODE_8, CODE_9,
		CODE_A, CODE_B, CODE_C, CODE_D, CODE_E, CODE_F
};


volatile uint8_t flags = 0x00;

// Time to switch beep on, show it on 7-seg display
volatile uint8_t showme = 0x00;

#define OFF 				0x01
#define MELODY_LENGTH 		15
volatile uint8_t melody[MELODY_LENGTH] = {
		(0x24), (OFF), (0x24),
		(OFF), (OFF), (OFF),
		(OFF),  (OFF), (OFF),
		(OFF),  (OFF), (OFF),
		(OFF),  (OFF), (OFF),
};
volatile uint8_t melody_indx = 0x00;



volatile uint8_t nota = 0x00;

#define TICKS_COUNT_IN_MINUTE 	16UL * 60
volatile uint16_t ticks_in_minute = TICKS_COUNT_IN_MINUTE;

#define PLAYING_MELODY_TIMES	MELODY_LENGTH * 10
volatile uint8_t playing_melody_times = PLAYING_MELODY_TIMES;


void Task_SwitchAllOff(void);
void Task_BeepStop(void);
void Task_BeepStart(void);

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
	if(FlagOn(PLAYING_MELODY)){
		if(--playing_melody_times != 0x00){
			if(melody[melody_indx] == OFF){
				FlagClear(BEEP_ON);

				// 7-Seg display on
				FlagSet(DISPLAY_ON);
				OS_AddTaskToEvalQueue( Task_7SegDisplay );
			}else{
				FlagSet(BEEP_ON);

				// 7-Seg display off
				FlagClear(DISPLAY_ON);
				Display7SegCommon_OFF();
			}

			nota = melody[melody_indx];

			if(++melody_indx == MELODY_LENGTH){
				melody_indx = 0;
			}

			OS_AddTaskToTimerQueue(Task_Beep, 500); // Get next nota in melody
		}else{
			Task_SwitchAllOff();
		}
	}
}

void Task_SwitchAllOff(void)
{
	FlagClear(PLAYING_MELODY);
	FlagClear(BEEP_ON);
	FlagClear(DISPLAY_ON);
	FlagClear(COUNTDOWN_ON);

	Display7SegCommon_OFF();
	HC595_PutUInt8( 0xff );
}

void Task_Countdown(void)
{
	if(FlagOn(COUNTDOWN_ON)){
		if(FlagOff(PLAYING_MELODY)){
			if(--ticks_in_minute == 0){
				ticks_in_minute = TICKS_COUNT_IN_MINUTE;
				if(--showme == 0){
					FlagSet(PLAYING_MELODY);
					playing_melody_times = PLAYING_MELODY_TIMES;
					melody_indx = 0;

					Task_Beep();
				}
			}
		}
		OS_AddTaskToTimerQueue(Task_Countdown, TASK_COUNTDOWN_DELAY);
	}
}

void Task_BeepStop(void)
{
	FlagClear(BEEP_ON);
}

void Task_LittleButtonClicked(void)
{
	if(PIND & _BV(PD2)){
		showme = 0x00;

		Task_SwitchAllOff();

		if(FlagOff(BEEP_ON)){
			FlagSet(BEEP_ON); // set flag
			nota = 0x50;
			OS_AddTaskToTimerQueue(Task_BeepStop, 0xEE); // add task to stop beep
		}else{
			FlagClear(BEEP_ON);
		}
	}
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

		FlagClear(PLAYING_MELODY);
		FlagClear(BEEP_ON);

		if(FlagOff(DISPLAY_ON)){ // if not already added to eval queue
			FlagSet(DISPLAY_ON);
			OS_AddTaskToEvalQueue( Task_7SegDisplay );
		}

		if(FlagOff(COUNTDOWN_ON)){
			FlagSet(COUNTDOWN_ON);
			OS_AddTaskToEvalQueue( Task_Countdown );
		}
	}
}

void Task_Int0_On(void)
{
	FlagClear(INT0_PROCESSED);
	EIFR = _BV(INTF0);
	GIMSK |= _BV(INT0);
}

void Task_Int1_On(void)
{
	FlagClear(INT1_PROCESSED);
	EIFR = _BV(INTF1);
	GIMSK |= _BV(INT1);
}


ISR( INT0_vect )
{
	GIMSK &=(uint8_t)~_BV(INT0);
	if(FlagOff(INT0_PROCESSED) && FlagOff(INT1_PROCESSED)){
		FlagOn(INT0_PROCESSED);
		OS_AddTaskToEvalQueue( Task_LittleButtonClicked );
		OS_AddTaskToTimerQueue( Task_Int0_On , 2 );
	}
	EIFR = _BV(INTF0);
}

ISR( INT1_vect )
{
	GIMSK &=(uint8_t)~_BV(INT1);
	if(FlagOff(INT0_PROCESSED) && FlagOff(INT1_PROCESSED)){
		FlagOn(INT1_PROCESSED);
		OS_AddTaskToEvalQueue( Task_MouseWheel );
		OS_AddTaskToTimerQueue( Task_Int1_On , 2 );
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

	uint8_t nota_temp = 1;

	for(;;){
		OS_EvalTask();

		if(FlagOn(BEEP_ON)){
			if(--nota_temp == 0x00){
				nota_temp = nota;
				if(BeeperPin()){
					BeeperPin_Down();
				}else{
					BeeperPin_Up();
				}
			}
		}else{
			BeeperPin_Down();
		}
	}

	return 93; // not reachable
}

