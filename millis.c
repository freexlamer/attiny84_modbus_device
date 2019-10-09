/*
Based on https://github.com/sourceperl/millis

*/

#include "millis.h"

uint64_t _millis = 0;

ISR(TIM0_OVF_vect) {
  _millis++;
}

void millis_setup() {
  /* interrup setup */
  OCR0A = TIMER0_TOP;
  // (F_CPU/TIMER_PRESCALE)/1000

  TCCR0A = (1<<WGM01)|(1<<WGM00);
  TCCR0B = TIMER0_PRESCALER|(1<<WGM02);
  // enable timer overflow interrupt
  TIMSK0  |= 1<<TOIE0;
}

void millis_end() {
  TIMSK0  &= ~(1<<TOIE0);

  OCR0A = 0;
  TCCR0A = 0;
  TCCR0B = 0;
}

uint64_t millis() {
  uint64_t m;

  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    m = _millis;
  }
  return m;
}