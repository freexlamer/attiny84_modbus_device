// (C) freexlamer@github.com

#include "m90e26.h"

unsigned char m90e26_frame[6];

unsigned char m90e26_buffer = 0;
bool m90e26_overflow = false;

unsigned char m90e26_calculate_chksum(unsigned char buffer_start, unsigned char buffer_end);
void m90e26_send_packet(unsigned char bufferSize);
void wdt_int_enable(unsigned char timeout);

bool m90e26_write_reg(unsigned char address, unsigned int data) {
	m90e26_frame[0] = M90E26_START_MARKER;
	m90e26_frame[1] = address & 0x7F; // address and read
	m90e26_frame[2] = data >> 8; // DATA_MSB
	m90e26_frame[3] = data & 0x00FF; // DATA_LSB
	m90e26_frame[4] = m90e26_calculate_chksum(1, 4);

	m90e26_send_packet(5);

	wdt_int_enable(TIMEOUT_WRITE_WDTO);
	unsigned char ack = (*m90e26_uart_getc)();
	wdt_disable();

	if (check_timeout_flag())
		return (m90e26_frame[4] == ack);
	else
		return false;
}

bool m90e26_read_reg(unsigned char address, unsigned int *data) {
	m90e26_frame[0] = M90E26_START_MARKER;
	m90e26_frame[1] = address | 0x80; // address and write
	m90e26_frame[2] = m90e26_frame[1];

	m90e26_send_packet(3);

	wdt_int_enable(TIMEOUT_READ_WDTO);
	m90e26_frame[0] = (*m90e26_uart_getc)();
	m90e26_frame[1] = (*m90e26_uart_getc)();
	m90e26_frame[2] = (*m90e26_uart_getc)();
	wdt_disable();

	/*
	if ((TIMEOUT_IOREG & _BV(TIMEOUT_FLAG))!=0)
	//if (check_timeout_flag())
		return false;
	*/
	if ((TIMEOUT_IOREG & _BV(TIMEOUT_FLAG))==0) {
		if (m90e26_frame[2] == m90e26_calculate_chksum(0,2)) {
			*data = m90e26_frame[1] | m90e26_frame[0] << 8;
			return true;
		}
		else
			return false;
	}
	else
		return false;
	

}

unsigned char m90e26_calculate_chksum(unsigned char buffer_start, unsigned char buffer_end) {
	unsigned char temp=0;
	for (unsigned char i = buffer_start; i < buffer_end; i++) {
		temp += m90e26_frame[i];
	}
	return temp;
}

void m90e26_send_packet(unsigned char bufferSize)
{
  
  for (unsigned char i = 0; i < bufferSize; i++)
    (*m90e26_uart_putc)(m90e26_frame[i]);

}

void wdt_int_enable(unsigned char timeout) {
    cli();
    WDTCSR |= (_BV(WDCE) | _BV(WDE));
    WDTCSR = _BV(WDIE) | timeout; // Enable WDT Interrupt
    sei();
    clear_timeout_flag();
}

ISR(WDT_vect)
{
    set_timeout_flag();
}
