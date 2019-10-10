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

#include "settings.h"



//
// Defines
//
#define HIGH 1
#define LOW 0

//
// Definitions
//
#define _SS_MAX_RX_BUFF 16 // RX buffer size, must be (1<<n)
#define _SS_RX_BUFF_MASK (_SS_MAX_RX_BUFF-1)
#ifndef GCC_VERSION
#define GCC_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#endif


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

}Uart;

//
// Types
//
typedef uint8_t byte;

// public variables

Uart *serial_0;
Uart *serial_1;
uint8_t FLAG;


//
// public methods
//

void softSerialBegin(Uart *p);
void softSerialEnd();
bool softSerialOverflow(Uart *p);
int softSerialPeek(Uart *p);
size_t softSerialWrite(uint8_t b, Uart *p);
int softSerialRead(Uart *p);
int softSerialAvailable(Uart *p);
void softSerialFlush(Uart *p);

void handler(Uart *p);

#ifdef OSCCAL_FORCE_CALIBRATION
char isCalibDataReady();
char getImpulsWidth();
#endif


#endif

