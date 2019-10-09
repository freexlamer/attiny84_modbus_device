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
  // prescale timer0 to 1/64th the clock rate
  // overflow timer0 every 125 tacts = 1 ms (at 8MHz)
  //OCR0A = 125;
  OCR0A = 125;
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
  uint8_t sreg;
  uint64_t m;
  sreg = SREG;

  cli();
  m = _millis;
  
  SREG = sreg;
  return m;
}