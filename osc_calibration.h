#ifndef OSC_CALIBRATION_H
#define OSC_CALIBRATION_H

#include <stdbool.h>
#include <avr/io.h>
#include <avr/eeprom.h>
#include <avr/interrupt.h>
#include <util/atomic.h>
#include <util/delay.h>

#include "settings.h"
#include "SoftwareSerial.h"

#define OSCCAL_EEADDR 0xFF

#define TIMER_STOP 0
#define TIMER_START (1<<CS10)

#if F_CPU == 1000000
	#define TIMER_REFERENCE 937 //(9*1000000/9600 = 937.5)
	//(count_of_zero_bits*F_CPU/baud)
	// (start bit + data bits = 9 bits)
	#define REFERENCE_HYSTERESIS 6
#elif F_CPU == 8000000
	#define TIMER_REFERENCE 7500 //(9*8000000/9600)
	#define REFERENCE_HYSTERESIS 40
#else
	#error F_CPU does not supported.
#endif

Uart *osc_calibration_serial_port;

void (*osc_calibration_toggle_led)();
size_t (*osc_calibration_SerialWrite)(uint8_t b, Uart *p);

bool update_osccal_from_eeprom();
void write_from_osccal_to_eeprom();
int perform_calibration();

#endif //OSC_CALIBRATION_H
