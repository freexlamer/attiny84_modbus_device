// (C) freexlamer@github.com

#include <stdbool.h>
#include <inttypes.h>
#include <stddef.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>

#include "SoftwareSerial.h"

#define M90E26_START_MARKER 0xFE

#define TIMEOUT_IOREG GPIOR0
#define TIMEOUT_FLAG 0
#define TIMEOUT_READ_WDTO WDTO_500MS
#define TIMEOUT_WRITE_WDTO WDTO_120MS


#define set_timeout_flag() \
TIMEOUT_IOREG |= _BV(TIMEOUT_FLAG);

#define clear_timeout_flag() \
TIMEOUT_IOREG &= ~(_BV(TIMEOUT_FLAG));

#define check_timeout_flag() ((TIMEOUT_IOREG & _BV(TIMEOUT_FLAG))==0)

Uart *m90e26_serial_port;

size_t (*m90e26_SerialWrite)(uint8_t b, Uart *p);
int (*m90e26_SerialRead)(Uart *p);

bool m90e26_read_reg(unsigned char address, unsigned int *data);
bool m90e26_write_reg(unsigned char address, unsigned int data);
void wdt_int_enable(unsigned char timeout);
