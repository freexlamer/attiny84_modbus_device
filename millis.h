/*
Based on https://github.com/sourceperl/millis

*/

#ifndef MILLIS_H
#define MILLIS_H

#include <avr/io.h>
#include <inttypes.h>
#include <avr/interrupt.h>

#if F_CPU == 1000000
	#define TIMER0_PRESCALER	(1<<CS01)|(0<<CS00)
	#define TIMER0_TOP			(F_CPU/8)/1000
#elif F_CPU == 8000000
	#define TIMER0_PRESCALER	(1<<CS01)|(1<<CS00)
	#define TIMER0_TOP			(F_CPU/64)/1000
#else
	#error F_CPU does not supported.
#endif

void millis_setup();
void millis_end();
uint64_t millis();

#endif //MILLIS_H
