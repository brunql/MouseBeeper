/*
 * bos.h Brunql OS
 *
 *  Created on: 23.01.2010
 *      Author: brunql
 */

#ifndef BOS_H_
#define BOS_H_

#include "bos_hal.h"
#include "main.h"

//#define NULL 0

#define EVAL_QUEUE_SIZE		5	/* Оперативка == хуита + стек... стек! СТЕК!!!  СУКА!!!!!!!!*/
#define TIMER_QUEUE_SIZE 	6

typedef void (*ptrTask)(void);

extern void OS_Iinialize(void);
extern void OS_AddTaskToEvalQueue(ptrTask task);
extern void OS_AddTaskToTimerQueue(ptrTask task, uint16_t time_to_eval);
extern void OS_SystemTimerTick(void);
extern void OS_EvalTask(void);


extern uint8_t evalQueueNowIndex;
extern uint8_t evalQueueAddIndex;
extern ptrTask evalQueue[ EVAL_QUEUE_SIZE ];

void OS_Error(uint8_t error);

#endif /* BOS_H_ */
