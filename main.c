// (C) freexlamer@github.com


#include <stdbool.h>
#include <util/delay.h>
#include <avr/io.h>

#include "settings.h"
#include "soft_uart.h"
#include "tiny_modbus_rtu_slave.h"
#include "m90e26.h"
#include "one_wire.h"

#define RELAY_R1 PA2
#define RELAY_R2 PA3
#define RELAY_REG_R1 0x0001
#define RELAY_REG_R2 0x0001
#define STATUS_LED PB2
#define MODBUS_SELF_ADDRESS 0x10

unsigned int relay_r1_reg, relay_r2_reg;
unsigned int ds18b20_cfg;

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
        return m90e26_read_reg(address, data);
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
    else 
        return false;
}


void led_set(bool stat) {
    if (stat) {
        PORTB |= (1 << STATUS_LED);
    }
    else {
        PORTB &= ~(1 << STATUS_LED);
    }
}


void relay_init(){
    update_relay_r1(0);
    update_relay_r2(0);
    DDRA |= (1 << RELAY_R1 | 1 << RELAY_R2);
}


int
main(void)
{
    DDRB = (1 << STATUS_LED);
    relay_init();

    slaveID = MODBUS_SELF_ADDRESS;

    modbus_uart_putc = &uart_putc;
    modbus_read_reg = &read_reg;
    modbus_write_reg = &write_reg;
    modbus_led = &led_set;

    m90e26_uart_putc = &uart2_putc;
    m90e26_uart_getc = &uart2_getc;

    /* loop */
    while (1) {
        pull_port(uart_getc());
    }
}

