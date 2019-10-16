/*
Based on ideas of https://github.com/angeloc/simplemodbusng

The crc calculation is based on the work published 
 by jpmzometa at 
 http://sites.google.com/site/jpmzometa/arduino-mbrt
 
 By Juan Bester : bester.juan@gmail.com
*/

// (C) freexlamer@github.com

#ifndef TINY_MODBUS_RTU_SLAVE_H
#define TINY_MODBUS_RTU_SLAVE_H

#include <stdbool.h>
#include <inttypes.h>
#include <stddef.h>
#include <util/delay.h>

#include "settings.h"
#include "SoftwareSerial.h"
#include "millis.h"

//#define MODBUS_ERROR_BUFFER_OVERFLOV_ENABLED	1

#define MODBUS_BUFFER_SIZE 32
// (1/BAUD)*PACKET_SIZE*3.5*1000 = timeout in ms
// (1/9600)*10*3.5*1000 = near 3
#define MODBUS_COMMAND_TIMEOUT UINT64_C(1000*10*3.5/UART0_BAUDRATE)

#define MODBUS_BROADCAST_ID 0x00

#define MODBUS_ERROR_MARKER 0x80
#define MODBUS_ERROR_ILLEGAL_FUNCTION 0x01
#define MODBUS_ERROR_ILLEGAL_DATA_ADDRESS 0x02
#define MODBUS_ERROR_ILLEGAL_DATA_VALUE 0x03
#define MODBUS_ERROR_SLAVE_DEVICE_FAILURE 0x04
#define MODBUS_ERROR_CRC 0x10
#define MODBUS_ERROR_BUFFER_OVERFLOW 0x11

#define MODBUS_CRC_SIZE				2
#define MODBUS_HEADER_SIZE			2
#define MODBUS_4_BYTES_PDU_SIZE		4

#define MODBUS_FUNCTION_READ_DO			0x01
#define MODBUS_FUNCTION_READ_DI			0x02
#define MODBUS_FUNCTION_READ_AO			0x03
#define MODBUS_FUNCTION_READ_AI			0x04
#define MODBUS_FUNCTION_WRITE_DO		0x05
#define MODBUS_FUNCTION_WRITE_AO		0x06
#define MODBUS_FUNCTION_WRITE_AO_RESPONSE_SIZE 8
#define MODBUS_FUNCTION_WRITE_MANY_DO	0x0F
#define MODBUS_FUNCTION_WRITE_MANY_AO	0x10

#define M90E26_START_ADDRESS 	0x0000
#define M90E26_END_ADDRESS 		0x006F

#define RELAY_REG_START_ADDRESS 0x0080
#define RELAY_REG_END_ADDRESS 	0x0081

#define DS18B20_START_ADDRESS 	0x0084
#define DS18B20_END_ADDRESS 	0x0085

#define ERRORS_START_ADDRESS 	0x0088
#define ERRORS_M90E26_READ 		ERRORS_START_ADDRESS
#define ERRORS_M90E26_WRITE 	ERRORS_START_ADDRESS+1
#define ERRORS_MODBUS_ALL 		ERRORS_START_ADDRESS+2
#define ERRORS_MODBUS_CRC 		ERRORS_START_ADDRESS+3
#define ERRORS_END_ADDRESS 		0x0091

#define DEBUG_REGS_START_ADDRESS 	0x0090
#define DEBUG_REGS_END_ADDRESS 		0x00A0


unsigned char slaveID;
Uart *modbus_serial_port;
unsigned int modbus_crc_errors;
unsigned char modbus_error_count;

size_t (*modbus_SerialWrite)(uint8_t b, Uart *p);
bool (*modbus_read_reg)(unsigned int address, unsigned int *data);
bool (*modbus_write_reg)(unsigned int address, unsigned int data);
void (*modbus_led)(bool);

void modbus_init();
unsigned char pull_port(int c);

#endif //TINY_MODBUS_RTU_SLAVE_H
