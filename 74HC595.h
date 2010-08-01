/*
 * 74HC595.h
 *
 *  Created on: 31.07.2010
 *      Author: brunql
 */

#ifndef HC595_H_INCLUDED
#define HC595_H_INCLUDED 1

#include <avr/io.h>

#define HC595_PORT 	(PORTB)
#define HC595_DDR 	(DDRB)

#define HC595_DATA_PIN 		_BV(PB1)
#define HC595_CLK_PIN 		_BV(PB4)
#define HC595_LATCH_PIN		_BV(PB3)

#define HC595_LATCH_UP		{ HC595_PORT |= HC595_LATCH_PIN; }
#define HC595_LATCH_DOWN	{ HC595_PORT &= (uint8_t)~HC595_LATCH_PIN; }
#define HC595_LATCH_PULSE	{ HC595_LATCH_UP; __asm__ __volatile__("nop"::); HC595_LATCH_DOWN; }

#define HC595_CLK_DOWN		{ HC595_PORT &= (uint8_t)~HC595_CLK_PIN; }
#define HC595_CLK_UP		{ HC595_PORT |= HC595_CLK_PIN; }

extern void HC595_Init(void);
extern void HC595_PutUInt8(uint8_t);
extern void HC595_PutUInt16(uint16_t);

#endif /* HC595_H_INCLUDED */
