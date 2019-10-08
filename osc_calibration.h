#include <stdbool.h>
#include <avr/io.h>
#include <avr/eeprom.h>
#include <avr/interrupt.h>
#include "soft_uart.h"
#include "SoftwareSerial.h"

#define OSCCAL_EEADDR 0xFF

#define TIMER_STOP 0
#define TIMER_START (1<<CS10)
//#define TIMER_REFERENCE 1041
#define TIMER_REFERENCE 937
//#define REFERENCE_HYSTERESIS 18
//#define REFERENCE_HYSTERESIS 10
//#define REFERENCE_HYSTERESIS 6
#define REFERENCE_HYSTERESIS 4

char osc_calibration_serial_port_num;

void (*osc_calibration_toggle_led)();
size_t (*osc_calibration_SerialWrite)(char, uint8_t);

bool update_osccal_from_eeprom();
void write_from_osccal_to_eeprom();
int perform_calibration();
