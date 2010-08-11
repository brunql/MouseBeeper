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


extern const uint8_t seg7_digits[];

//
// Led common for 7-seg is PD6, PB0;
//
inline void Display7SegCommon_Init(void){
	PORTD &= (uint8_t)~_BV(PD6); DDRD |= _BV(PD6);
	PORTB &= (uint8_t)~_BV(PB0); DDRB |= _BV(PB0);
}

inline void Display7SegCommon_Out1(void){
	PORTD |= _BV(PD6);
	PORTB &= (uint8_t)~_BV(PB0);
}

inline void Display7SegCommon_Out2(void){
	PORTD &= (uint8_t)~_BV(PD6);
	PORTB |= _BV(PB0);
}

inline void Display7SegCommon_OFF(void){
	PORTD &= (uint8_t)~_BV(PD6);
	PORTB &= (uint8_t)~_BV(PB0);
}

//
// Beeper connected to PA0
//
inline void BeeperPin_Init(void){
	PORTA &= (uint8_t)~_BV(PA0);
	DDRA |= _BV(PA0);
}

inline uint8_t BeeperPin(void){
	return (PINA & _BV(PA0));
}

inline void BeeperPin_Up(void){
	PORTA |= _BV(PA0);
}

inline void BeeperPin_Down(void){
	PORTA &= (uint8_t)~_BV(PA0);
}


enum FLAGS{
	DISPLAY_ON,
	DISPLAY_FIRST_OR_SECOND,
	BEEP_ON,
	INT0_PROCESSED,
	INT1_PROCESSED,
	COUNTDOWN_ON
};

extern volatile uint8_t flags;

static inline uint8_t FlagOn(uint8_t flag){
	return (flags & (1<<flag));
}

static inline uint8_t FlagOff(uint8_t flag){
	return !FlagOn(flag);
}

static inline void FlagSet(uint8_t flag){
	flags |= (1<<flag);
}

static inline void FlagClear(uint8_t flag){
	flags &= (uint8_t)~(1<<flag);
}


#endif /* MAIN_H_ */
