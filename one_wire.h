#ifndef ONE_WIRE_H
#define ONE_WIRE_H

#include "settings.h"

#include <stdbool.h>
#include <avr/io.h>
#include <util/delay.h>

#define DS18B20_PORT PORTA
#define DS18B20_DDR DDRA
#define DS18B20_PIN PINA
#define DS18B20_DQ PA7

bool DS18B20_init(void);
unsigned char DS18B20_read(void);
void DS18B20_write(unsigned char data);
bool DS18B20_get(unsigned int *data, unsigned int *cfg);

#endif //ONE_WIRE_H
