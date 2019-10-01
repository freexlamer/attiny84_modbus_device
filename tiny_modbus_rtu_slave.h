/*
Based on ideas of https://github.com/angeloc/simplemodbusng

The crc calculation is based on the work published 
 by jpmzometa at 
 http://sites.google.com/site/jpmzometa/arduino-mbrt
 
 By Juan Bester : bester.juan@gmail.com
*/

// (C) freexlamer@github.com


#include <stdbool.h>
#include <util/delay.h>

#define BUFFER_SIZE 16

#define MODBUS_BROADCAST_ID 0x00

#define MODBUS_ERROR_MARKER 0x80
#define MODBUS_ERROR_ILLEGAL_FUNCTION 0x01
#define MODBUS_ERROR_ILLEGAL_DATA_ADDRESS 0x02
#define MODBUS_ERROR_ILLEGAL_DATA_VALUE 0x03
#define MODBUS_ERROR_SLAVE_DEVICE_FAILURE 0x04
#define MODBUS_ERROR_CRC 0x10

#define MODBUS_FUNCTION_READ_AO 0x03
#define MODBUS_FUNCTION_WRITE_AO 0x06
#define MODBUS_FUNCTION_WRITE_AO_RESPONSE_SIZE 8

#define M90E26_START_ADDRESS 0x0000
#define M90E26_END_ADDRESS 0x006F
#define RELAY_REG_START_ADDRESS 0x0080
#define RELAY_REG_END_ADDRESS 0x0081
#define DS18B20_START_ADDRESS 0x0084
#define DS18B20_END_ADDRESS 0x0085
#define ERRORS_START_ADDRESS 0x0088
#define ERRORS_END_ADDRESS 0x0088

unsigned char slaveID;
void (*modbus_uart_putc)(unsigned char);
bool (*modbus_read_reg)(unsigned int address, unsigned int *data);
bool (*modbus_write_reg)(unsigned int address, unsigned int data);
void (*modbus_led)(bool state);

unsigned char pull_port(unsigned char c);

