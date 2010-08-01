/*
 * bos.c - Brunql real time OS. Main file.
 *
 *  Created on: 23.01.2010
 *      Author: brunql
 */
#include <util/atomic.h>
#include <stdlib.h>
#include "bos.h"

#include "main.h"
#include "74HC595.h"

enum BOS_ERRORS{
	EVAL_QUEUE_OVERFLOW_EVAL_TASK 	= CODE_0,
	EVAL_QUEUE_OVERFLOW_ADD_TASK  	= CODE_1,
	EVAL_QUEUE_ADD_ERROR			= CODE_2,
	TIMER_QUEUE_OVERFLOW			= CODE_3
};

struct task {
	ptrTask task;
	uint16_t time_to_eval;
};

struct task timerQueue[ TIMER_QUEUE_SIZE ];

uint8_t evalQueueNowIndex = 0x00;
uint8_t evalQueueAddIndex = 0x00;
ptrTask evalQueue[ EVAL_QUEUE_SIZE ];


void OS_Error(uint8_t error);


void OS_Iinialize(void)
{
	for(uint8_t i=0; i < TIMER_QUEUE_SIZE; i++){
		timerQueue[i].task = NULL;
		timerQueue[i].time_to_eval = 0;
	}

	for(uint8_t i=0; i < EVAL_QUEUE_SIZE; i++){
		evalQueue[i] = NULL;
	}
}

void OS_AddTaskToEvalQueue(ptrTask task)
{
	ATOMIC_BLOCK( ATOMIC_RESTORESTATE ){
		if (evalQueue[evalQueueAddIndex] == NULL){
			evalQueue[evalQueueAddIndex] = task;

			if(++evalQueueAddIndex == EVAL_QUEUE_SIZE){
				evalQueueAddIndex = 0x00;
			}
			if((evalQueueAddIndex == evalQueueNowIndex) &&
					evalQueue[evalQueueAddIndex] != NULL){
				OS_Error(EVAL_QUEUE_OVERFLOW_ADD_TASK);
			}
		}else{
			OS_Error(EVAL_QUEUE_ADD_ERROR);
		}
	}
}

void OS_AddTaskToTimerQueue(ptrTask task, uint16_t time_to_eval)
{
	ATOMIC_BLOCK( ATOMIC_RESTORESTATE ){
		// It was an extensive study on a piece of paper, the result is:
		uint8_t i;
		for(i=0; i < TIMER_QUEUE_SIZE; i++){
			if(timerQueue[i].task == NULL){
				timerQueue[i].task = task;
				timerQueue[i].time_to_eval = time_to_eval;
				break; // !
			}
		}

		if(i == TIMER_QUEUE_SIZE){
			OS_Error(TIMER_QUEUE_OVERFLOW);
		}
	}
}

void OS_SystemTimerTick(void)
{
	// In interrupt from timer
	for(uint8_t i=0; i < TIMER_QUEUE_SIZE; i++){
		if(timerQueue[i].task != NULL){
			if( --timerQueue[i].time_to_eval == 0){
				OS_AddTaskToEvalQueue(timerQueue[i].task);
				timerQueue[i].task = NULL;
				timerQueue[i].time_to_eval = 0;
			}
		}
	}
}

void OS_EvalTask(void)
{
	ptrTask evalMe = NULL;
	ATOMIC_BLOCK( ATOMIC_RESTORESTATE ){
		if(evalQueue[evalQueueNowIndex] != NULL){
			evalMe = evalQueue[evalQueueNowIndex];
			evalQueue[evalQueueNowIndex] = NULL;
			if( ++evalQueueNowIndex == EVAL_QUEUE_SIZE ){
				evalQueueNowIndex = 0x00;
			}
			if((evalQueueAddIndex == evalQueueNowIndex)	&&
					evalQueue[evalQueueNowIndex] != NULL ){
				OS_Error(EVAL_QUEUE_OVERFLOW_EVAL_TASK);
			}
		}// else do nothing, has no tasks
	}
	if(evalMe) (evalMe)();
}

void OS_Error(uint8_t error)
{
	cli();

	Display7SegCommon_Out1();
	HC595_PutUInt8(~error);

	for(;;){ /* error infinity loop */ }
}
