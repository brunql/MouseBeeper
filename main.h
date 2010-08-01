/*
 * main.h
 *
 *  Created on: 31.07.2010
 *      Author: brunql
 */

#ifndef MAIN_H_
#define MAIN_H_

// 7-Segment led display pinout for first 74HC595
//	╔═a═╗
//	f   b
//	╠═g═╣
//	e   c
//	╚═d═╝ (h)
#define a	_BV(6)
#define b	_BV(4)
#define c	_BV(3)
#define d	_BV(1)
#define e	_BV(2)
#define f	_BV(5)
#define g	_BV(7)
#define h	0x00 /* dot not connected */

#define CODE_0			(a | b | c | d | e | f)
#define CODE_1			(b | c)
#define CODE_2			(a | b | g | e | d)
#define CODE_3			(a | b | g | c | d)
#define CODE_4			(f | g | b | c)
#define CODE_5			(a | f | g | c | d)
#define CODE_6			(a | f | g | c | d | e)
#define CODE_7			(a | b | c)
#define CODE_8			(a | b | c | d | e | f | g)
#define CODE_9			(a | b | c | d | f | g)
#define CODE_A			(a | b | c | g | e | f)
#define CODE_B			(f | g | c | d | e)
#define CODE_C			(g | d | e)
#define CODE_D			(b | g | c | d | e)
#define CODE_E			(a | f | g | d | e)
#define CODE_F			(a | f | g | e)
#define CODE_DOT		(h)
#define CODE_CLEAR		(0x00)
#define CODE_DEGRE		(a | b | g | f)
#define CODE_CELSIUS 	(a | f | e | d)

// Led common for 7-seg is PD6, PB0;
static inline void led_common_init(void){
	PORTD &= (uint8_t)~_BV(PD6); DDRD |= _BV(PD6);
	PORTB &= (uint8_t)~_BV(PB0); DDRB |= _BV(PB0);
}

static inline void led_common_out_1(void){
	PORTD |= _BV(PD6);
	PORTB &= (uint8_t)~_BV(PB0);
}

static inline void led_common_out_2(void){
	PORTD &= (uint8_t)~_BV(PD6);
	PORTB |= _BV(PB0);
}

static inline void led_common_off(void){
	PORTD &= (uint8_t)~_BV(PD6);
	PORTB &= (uint8_t)~_BV(PB0);
}

#endif /* MAIN_H_ */