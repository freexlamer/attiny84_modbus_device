// (C) freexlamer@github.com

#include <stdbool.h>
#include <inttypes.h>
#include <stddef.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>  

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

char m90e26_serial_port_num;

size_t (*m90e26_SerialWrite)(char, uint8_t);
int (*m90e26_SerialRead)(char);

bool m90e26_read_reg(unsigned char address, unsigned int *data);
bool m90e26_write_reg(unsigned char address, unsigned int data);
void wdt_int_enable(unsigned char timeout);
