// (C) freexlamer@github.com

#include <stdbool.h>
#include <util/delay.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>

#include "settings.h"
#include "SoftwareSerial.h"
#include "tiny_modbus_rtu_slave.h"
#include "m90e26.h"
#include "one_wire.h"
#include "osc_calibration.h"
#include "millis.h"

#define RELAY_R1 PA2
#define RELAY_R2 PA3
#define RELAY_REG_R1 0x0001
#define RELAY_REG_R2 0x0001
#define STATUS_LED_PIN PB2
#define STATUS_LED_PORT PORTB
#define STATUS_LED_DDR DDRB
#define MODBUS_SELF_ADDRESS 0x10

unsigned int relay_r1_reg, relay_r2_reg;
unsigned int ds18b20_cfg;

//volatile 

bool perform_main_loop_exit = false;
bool perform_forced_calibration = false;

void led_toggle();

bool update_relay_r1(unsigned int data){
    relay_r1_reg = data;
    if ((relay_r1_reg & RELAY_REG_R1)>0) {
        PORTA |= (1 << RELAY_R1);
    }
    else {
        PORTA &= ~(1 << RELAY_R1);
    }

    return true;
}

bool update_relay_r2(unsigned int data){
    relay_r2_reg = data;
    if ((relay_r2_reg & RELAY_REG_R2)>0) {
        PORTA |= (1 << RELAY_R2);
    }
    else {
        PORTA &= ~(1 << RELAY_R2);
    }
    return true;
}

bool get_temperature_sensor(unsigned int address, unsigned int *data) {
    if (address == DS18B20_START_ADDRESS) {
        return DS18B20_get(data, &ds18b20_cfg);
    }
    else if (address == DS18B20_END_ADDRESS) {
        *data = ds18b20_cfg;
        return true;
    }
    else
        return false;
}

bool read_reg(unsigned int address, unsigned int *data){
    //bool status;

    if ((address>=M90E26_START_ADDRESS) && (address<=M90E26_END_ADDRESS)) {
        // *data = 1;
        //return true;
        //return m90e26_read_reg(address, data);
        if (m90e26_read_reg(address, data)) {
            return true;
        }
        else {
            led_toggle();
            return false;
        }
    } 
    else if (address == RELAY_REG_START_ADDRESS) {
        *data = relay_r1_reg;
        return true;
    }
    else if (address == RELAY_REG_END_ADDRESS) {
        *data = relay_r2_reg;
        return true;
    }
    else if ((address>=DS18B20_START_ADDRESS) && (address<=DS18B20_END_ADDRESS)) {
        return get_temperature_sensor(address, data);
    }
    else if ((address>=ERRORS_START_ADDRESS) && (address<=ERRORS_END_ADDRESS)) {
        *data = 4;
        return true;
    }
    else if (address == DEBUG_REGS_START_ADDRESS) {
        *data = OSCCAL;
    }
    else if (address == DEBUG_REGS_START_ADDRESS+1) {
        *data = eeprom_read_byte(OSCCAL_EEADDR);
    }
    else 
        return false;
}


bool write_reg(unsigned int address, unsigned int data){
    if ((address>=M90E26_START_ADDRESS) && (address<=M90E26_END_ADDRESS)) {
        //return true;
        return m90e26_write_reg(address, data);;
    } 
    else if (address == RELAY_REG_START_ADDRESS) {
        return update_relay_r1(data);
    }
    else if (address == RELAY_REG_END_ADDRESS) {
        return update_relay_r2(data);
    }
    else if ((address>=DS18B20_START_ADDRESS) && (address<=DS18B20_END_ADDRESS)) {
        return false;
    }
    else if ((address>=ERRORS_START_ADDRESS) && (address<=ERRORS_END_ADDRESS)) {
        return true;
    }
    else if (address == DEBUG_REGS_START_ADDRESS+1) {
        eeprom_write_byte(OSCCAL_EEADDR, data);
        return true;
    }
    else 
        return false;
}


void led_set(bool stat) {
    if (stat) {
        STATUS_LED_PORT |= (1 << STATUS_LED_PIN);
    }
    else {
        STATUS_LED_PORT &= ~(1 << STATUS_LED_PIN);
    }
}

void led_toggle() {
    STATUS_LED_PORT ^= (1 << STATUS_LED_PIN);

}


void relay_init(){
    /* Выводим на пины 0, т.е. отключаем
    ** Потом переводим пины в режим вывода
    */

    update_relay_r1(0);
    update_relay_r2(0);
    DDRA |= (1 << RELAY_R1 | 1 << RELAY_R2);
}

//Uart serial0;
//Uart serial1;


int main(void)
{
    uint8_t tmp;

    // Ports initialization
    // Status led pin
    STATUS_LED_DDR = (1 << STATUS_LED_PIN);

    // internal chronometer initialization
    millis_setup();

    while(1) {
        perform_main_loop_exit = false;

        Uart serial0 = {9600, &DDRA, &PORTA, &PINA, PA5, PA6};
        Uart serial1 = {9600, &DDRA, &PORTA, &PINA, PA0, PA1};

        serial_0 = &serial0;
        serial_1 = &serial1;

        if (perform_forced_calibration || !update_osccal_from_eeprom()) {
            for (int i=0; i<10; i++) {
                led_toggle();
                _delay_ms(100);
                led_toggle();
                _delay_ms(250);
            }
            osc_calibration_toggle_led = &led_toggle;
            osc_calibration_serial_port = serial_0;
            osc_calibration_SerialWrite =  &softSerialWrite;
            softSerialBegin(serial_0);

            tmp = perform_calibration();

            //softSerialBegin(serial_0);
            softSerialWrite(0xFF, serial_0);
            softSerialWrite(tmp, serial_0);

            write_from_osccal_to_eeprom();

            softSerialEnd();
            perform_forced_calibration = false;
        }

        // Relays pins
        relay_init();

        // UARTs initialization
        //softSerialBegin(SOFT_UART0, 9600, &DDRA, &PORTA, &PINA, PA5, PA6);
        //softSerialBegin(SOFT_UART1, 9600, &DDRA, &PORTA, &PINA, PA0, PA1);
       

        softSerialBegin(serial_0);
        softSerialBegin(serial_1);

        // MODBUS initialization
        slaveID = MODBUS_SELF_ADDRESS;

        modbus_serial_port = serial_0;
        modbus_SerialWrite = &softSerialWrite;
        modbus_read_reg = &read_reg;
        modbus_write_reg = &write_reg;
        modbus_led = &led_set;

        // M90E26 initialization
        m90e26_serial_port = serial_1;
        m90e26_SerialWrite = &softSerialWrite;
        m90e26_SerialRead = &softSerialRead;

        led_toggle();

        sei();

        /* loop */
        while (1) {
            if (perform_main_loop_exit) 
                break;
          
            if (softSerialAvailable(serial_0)) {
                led_toggle();
                pull_port(softSerialRead(serial_0));
            }

            if (FLAG) {
                perform_main_loop_exit = true;
                perform_forced_calibration = true;
                FLAG = 0;
            }

        }

        for (int i=0; i<10; i++) {
            led_toggle();
            _delay_ms(150);
        }
        

        softSerialEnd();
        millis_end();
    }
    
}

