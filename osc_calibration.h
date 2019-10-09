#include <stdbool.h>
#include <avr/io.h>
#include <avr/eeprom.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "settings.h"
#include "SoftwareSerial.h"

#define OSCCAL_EEADDR 0xFF

#define TIMER_STOP 0
#define TIMER_START (1<<CS10)
//#define TIMER_REFERENCE 1041
//#define TIMER_REFERENCE 937
#define TIMER_REFERENCE 7500
//#define REFERENCE_HYSTERESIS 18
//#define REFERENCE_HYSTERESIS 10
//#define REFERENCE_HYSTERESIS 6
#define REFERENCE_HYSTERESIS 30

Uart *osc_calibration_serial_port;

void (*osc_calibration_toggle_led)();
size_t (*osc_calibration_SerialWrite)(uint8_t b, Uart *p);

bool update_osccal_from_eeprom();
void write_from_osccal_to_eeprom();
int perform_calibration();
