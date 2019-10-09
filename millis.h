/*
Based on https://github.com/sourceperl/millis

*/

#include <avr/io.h>
#include <inttypes.h>
#include <avr/interrupt.h>


void millis_setup();
void millis_end();
uint64_t millis();

