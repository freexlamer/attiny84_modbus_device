#include "osc_calibration.h"


bool update_osccal_from_eeprom() {
	uint8_t osccal_new = eeprom_read_byte((uint8_t*)OSCCAL_EEADDR);
	uint8_t osccal_old = OSCCAL;
	uint8_t i;

	// blank eeprom filled up by 0xFF
	if (osccal_new!=0xFF) {
		if (osccal_new>osccal_old) {
			for(i = osccal_old+1; i <= osccal_new; i++) {
				OSCCAL = i;
			}
		}
		else if (osccal_new<osccal_old) {
			for(i = osccal_old-1; i >= osccal_new; i--) {
				OSCCAL = i;
			}

		}
		return true;
	}
	else
		return false;
}


/*
bool update_osccal_from_eeprom() {
	uint8_t osccal_new = eeprom_read_byte(OSCCAL_EEADDR);
	if (osccal_new!=0xFF) {
		OSCCAL = osccal_new;
		return true;
	}
	else
		return false;
}
*/

void write_from_osccal_to_eeprom() {
	eeprom_write_byte((uint8_t*)OSCCAL_EEADDR, OSCCAL);
}

int perform_calibration() {
	uint8_t osctmp = OSCCAL;
	bool success = false;
	unsigned int i;

	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {

		// setup (turn off) the timer1
		TCCR1A = 0;
		TCCR1B = 0;

	    while (1) {
			TCNT1 = 0;

			__asm volatile(
			"calibrationWaitStart: \n\t"
			" sbic %[uart_port]-2, %[uart_pin] \n\t" // wait for start edge
			" rjmp calibrationWaitStart \n\t"

			" ldi r18, %[timer_start] \n\t"
			" out %[tccrb], r18 \n\t"

			"calibrationWaitStop: \n\t"
			" sbis %[uart_port]-2, %[uart_pin] \n\t"
			" rjmp calibrationWaitStop \n\t"

			" ldi r18, %[timer_stop] \n\t"
			" out %[tccrb], r18 \n\t"

			:: [uart_port] "I" (_SFR_IO_ADDR(UART0_PORT)),
			[uart_pin] "I" (UART0_RX_PIN),
			[timer_start] "I" (TIMER_START),
			[timer_stop] "I" (TIMER_STOP),
			[tccrb] "I" (_SFR_IO_ADDR(TCCR1B))
			: "r0","r18"
			);

			i = TCNT1;

			if (i > (TIMER_REFERENCE+REFERENCE_HYSTERESIS/2)) {
				osctmp--;
		    }
		    else if (i < (TIMER_REFERENCE-REFERENCE_HYSTERESIS/2)) {
				osctmp++;
			}
			else {
				success = true;
				break;
		    }

			OSCCAL = osctmp;
			(*osc_calibration_SerialWrite)(osctmp, osc_calibration_serial_port);

			for (i=0; i<5; i++) {
				(*osc_calibration_toggle_led)();
				_delay_ms(150);
		    }
	    }

	}

	if (success)
		return osctmp;
	else
		return -1;
}
