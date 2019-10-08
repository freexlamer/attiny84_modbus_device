/*

BASED ON: https://github.com/shimniok/ATtinySoftSerial
For: Attiny84A 8 MHz

Added:
-Support of multiple UARTS.
-Measurement of long impulse width (longer than selected baud rate byte).

For correct work you have to manually configure PCINT interrupts. As example if receiver or uart are on PA0 you have to detect low level
when interrupt occurs and call a function handler() with a Uart struct created before for this interface before as param.

Measured width stored on pWidth variable and can be taken by calling getImpulsWidth. To check if data about width is ready you can call
isCalibDataReady function and it will return 1 if data ready.

This code configures:

-Timer 0, CHANNEL A COMPARE INTERRUPT.
-PCINT interrupts.

*/


#ifndef SoftwareSerial_h
#define SoftwareSerial_h

#include <avr/io.h>
#include <inttypes.h>
#include <stddef.h>
#include <stdbool.h>


//
// Defines
//
//#define true 1
//#define false 0
#define HIGH 1
#define LOW 0

//
// Definitions
//
#define UARTS_COUNT		2
#define SOFT_UART0	0
#define SOFT_UART1	1
#define _SS_MAX_RX_BUFF 16 // RX buffer size, must be (1<<n)
#define _SS_RX_BUFF_MASK (_SS_MAX_RX_BUFF-1)
#ifndef GCC_VERSION
#define GCC_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#endif


//
// Types
//
typedef uint8_t byte;

typedef volatile struct{
	volatile long _SPEED;
	char* _DDR;
	char* _PORT;
	char* _PIN;
	char _RX_PIN_NUM;
	char _TX_PIN_NUM;
	char _receive_buffer[_SS_MAX_RX_BUFF];
	uint8_t _receive_buffer_tail;
	uint8_t _receive_buffer_head;
	uint16_t _buffer_overflow;
    uint16_t _rx_delay_intrabit;
	uint16_t _rx_delay_centering;
	uint16_t _rx_delay_stopbit;
	uint16_t _tx_delay;

} Uart;

//
// public methods
//

Uart serials[UARTS_COUNT];

Uart *serial_0;
Uart *serial_1;

void softSerialBegin(char port_num, long speed, char *ddr, char *port, 
					 char *pin, char rx_pin, char tx_pin);
void softSerialEnd(char port_num);
bool softSerialOverflow(char port_num);
int softSerialPeek(char port_num);
size_t softSerialWrite(char port_num, uint8_t b);
int softSerialRead(char port_num);
int softSerialAvailable(char port_num);
void softSerialFlush(char port_num);

void handler(char port_num);

char isCalibDataReady();
char getImpulsWidth();


#endif

