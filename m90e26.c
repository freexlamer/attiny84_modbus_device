// (C) freexlamer@github.com

#include "m90e26.h"

unsigned char m90e26_frame[6];

unsigned char m90e26_buffer = 0;
bool m90e26_overflow = false;

uint64_t start_time;

unsigned char m90e26_calculate_chksum(unsigned char buffer_start, unsigned char buffer_end);
void m90e26_send_packet(unsigned char bufferSize);
void wdt_int_enable(unsigned char timeout);

void m90e26_init() {
    m90e26_read_errors = 0;
    m90e26_write_errors = 0;
}

bool m90e26_write_reg(unsigned char address, unsigned int data) {
	//int rx_data;

	m90e26_frame[0] = M90E26_START_MARKER;
	m90e26_frame[1] = address & 0x7F; // address and read
	m90e26_frame[2] = data >> 8; // DATA_MSB
	m90e26_frame[3] = data & 0x00FF; // DATA_LSB
	m90e26_frame[4] = m90e26_calculate_chksum(1, 4);

	m90e26_send_packet(5);

	start_time = millis();
	while (!softSerialAvailable(m90e26_serial_port)) {
		_delay_ms(ANSWER_DELAY_MS);
		if ((millis() - start_time) > TIMEOUT_READ_WRITE_MS) {
			m90e26_write_errors++;
			return false;
		}
	}

	unsigned char ack = (unsigned char)(*m90e26_SerialRead)(m90e26_serial_port);
	if (m90e26_frame[4] == ack) {
		return true;
	}
	else {
		m90e26_write_errors++;
		return false;
	}

}

bool m90e26_read_reg(unsigned char address, unsigned int *data) {
	int rx_data;
	char i;

	m90e26_frame[0] = M90E26_START_MARKER;
	m90e26_frame[1] = address | 0x80; // address and write
	m90e26_frame[2] = m90e26_frame[1];

	m90e26_send_packet(3);

	//wdt_int_enable(TIMEOUT_READ_WDTO);
	for (unsigned char i = 0; i < 3; i++) {
		start_time = millis();
		while (!softSerialAvailable(m90e26_serial_port)) {
			_delay_ms(ANSWER_DELAY_MS);
			if (millis() - start_time > TIMEOUT_READ_WRITE_MS) {
				m90e26_read_errors++;
				return false;
			}
		}

		m90e26_frame[i] = (unsigned char)(*m90e26_SerialRead)(m90e26_serial_port);

	}


	if (m90e26_frame[2] == m90e26_calculate_chksum(0,2)) {
		*data = m90e26_frame[1] | (m90e26_frame[0] << 8);
		return true;
	}
	else {
		m90e26_read_errors++;
		return false;
	}
	

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

  for (unsigned char i = 0; i < bufferSize; i++) {
    (*m90e26_SerialWrite)(m90e26_frame[i], m90e26_serial_port);
  }

}