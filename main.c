// (C) freexlamer@github.com

#include <stdbool.h>
#include <util/delay.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>

#include "settings.h"
#include "soft_uart.h"
#include "tiny_modbus_rtu_slave.h"
#include "m90e26.h"
#include "one_wire.h"
#include "osc_calibration.h"

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


int main(void)
{
    uint8_t tmp;
    // Ports initialization
    // Status led pin
    STATUS_LED_DDR = (1 << STATUS_LED_PIN);

    while(1) {
        perform_main_loop_exit = false;

        if (perform_forced_calibration || !update_osccal_from_eeprom()) {
            osc_calibration_toggle_led = &led_toggle;
            osc_calibration_uart_putc =  &uart_putc;
            uart_init_tx_pin();
            uart_init_rx_pin();

            tmp = perform_calibration();
            perform_forced_calibration = false;

            uart_init_tx_pin();
            uart_putc(0xFF);
            write_from_osccal_to_eeprom();
        }

        update_osccal_from_eeprom();
        
        // Relays pins
        relay_init();

        // UARTs initialization
        uart_init_rx_pin();
        uart_init_tx_pin();

        uart2_init_rx_pin();
        uart2_init_tx_pin();

        // MODBUS initialization
        slaveID = MODBUS_SELF_ADDRESS;

        modbus_uart_putc = &uart_putc;
        modbus_read_reg = &read_reg;
        modbus_write_reg = &write_reg;
        modbus_led = &led_set;

        // M90E26 initialization
        m90e26_uart_putc = &uart2_putc;
        m90e26_uart_getc = &uart2_getc;

        /* loop */
        while (1) {
            if (perform_main_loop_exit) 
                break;
            pull_port(uart_getc());
        }
    }
    
}

