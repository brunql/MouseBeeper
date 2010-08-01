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

#include "main.h"
#include "74HC595.h"

#define CODE_NUM_CLEAR	16

#define TIMER0_DIVIDER_VALUE	64

const uint8_t seg7_digits[] = {
		CODE_0, CODE_1, CODE_2, CODE_3, CODE_4, CODE_5, CODE_6, CODE_7, CODE_8, CODE_9,
		CODE_A, CODE_B, CODE_C, CODE_D, CODE_E, CODE_F,
		/* 16 */ CODE_CLEAR
};

volatile uint8_t showme = 0x00;
volatile uint8_t first_or_second = 0x00;

volatile uint8_t beep_on = 0x00;
volatile uint8_t timer0_divider = TIMER0_DIVIDER_VALUE;
volatile uint8_t melody = 0x01;
volatile uint8_t melody_temp = 0x00;

volatile uint8_t sec_to_min = 60;

static inline void Timers_Init(void)
{
	TCCR0A = 0x00;
	TCCR0B = _BV(CS00);

	TCCR1A = 0x00;
	TCCR1B = _BV(CS12) | _BV(CS10); // 1024
	TCCR1C = 0x00;
	OCR1A = 7812; // 1Hz
}

static inline void Timers_Start(void)
{
	TCNT0 = 0x00;
	TCNT1 = 0x0000;
	TIMSK = _BV(TOIE0) | _BV(OCIE1A);
}

static inline void Timers_Stop(void)
{
	TIMSK = 0x00;
}


ISR( TIMER0_OVF_vect )
{
	if(beep_on){
		if(--melody_temp == 0x00){

			melody_temp = melody;

			if(PINA & _BV(PA0)){
				PORTA &=(uint8_t)~_BV(PA0);
				DDRA &=(uint8_t)~_BV(PA0);
			}else{
				PORTA |=_BV(PA0);
				DDRA |= _BV(PA0);
			}
		}
	}

	if(--timer0_divider == 0){
		timer0_divider = TIMER0_DIVIDER_VALUE;


		if(first_or_second){
			first_or_second = 0x00;

			led_common_off();
			HC595_PutUInt8( 0xff );

			led_common_out_1();
			HC595_PutUInt8( ~seg7_digits[(showme & 0x0f)] );
		}else{
			first_or_second = 0x01;

			led_common_off();
			HC595_PutUInt8( 0xff );

			led_common_out_2();
			HC595_PutUInt8( ~seg7_digits[(showme & 0xf0)>>4] );
		}
	}
}


ISR( TIMER1_COMPA_vect )
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
	TCNT1 = 0x0000;
}

ISR( INT0_vect )
{
	_delay_ms(2);

	if(PIND & _BV(PD2)){

		Timers_Stop();
		showme = 0x00;
		beep_on = 0x00;

		led_common_off();
		HC595_PutUInt8( 0xff );
	}

	EIFR = _BV(INTF0);
}

ISR( INT1_vect )
{
	_delay_ms(2);

	if(!(PIND & _BV(PD3))){
		if(PIND & _BV(PD4)){
			showme++;
		}else{
			showme--;
		}

		Timers_Start();
		beep_on = 0x00;
	}

	EIFR = _BV(INTF1);
}



int main(void)
{
	PORTA = 0x00;
	DDRA = _BV(PA0); // Beeper

	PORTB = 0x00;
	DDRB = 0x00;

	PORTD = 0x00;
	DDRD = 0x00; //_BV(PD5);

	led_common_init();

	HC595_Init();

	ACSR = _BV(ACD); // Analog comparator off

	Timers_Init();

	MCUCR = _BV(ISC00) | _BV(ISC10); // INT0,1 log change
	GIMSK = _BV(INT0) | _BV(INT1);

	sei();

	for(;;){
	}

	return 93; // hard joke!
}
