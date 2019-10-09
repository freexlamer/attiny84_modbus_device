// (C) freexlamer@github.com

#ifndef M90E26_H
#define M90E26_H

#include <stdbool.h>
#include <inttypes.h>
#include <stddef.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "SoftwareSerial.h"
#include "millis.h"

#define M90E26_START_MARKER 0xFE

#define TIMEOUT_READ_WRITE_MS 10
#define ANSWER_DELAY_MS 1

Uart *m90e26_serial_port;

unsigned int m90e26_read_errors;
unsigned int m90e26_write_errors;

size_t (*m90e26_SerialWrite)(uint8_t b, Uart *p);
int (*m90e26_SerialRead)(Uart *p);

void m90e26_init();
bool m90e26_read_reg(unsigned char address, unsigned int *data);
bool m90e26_write_reg(unsigned char address, unsigned int data);

#endif //M90E26_H
