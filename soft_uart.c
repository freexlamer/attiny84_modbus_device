/**
 * Copyright (c) 2017, Łukasz Marcin Podkalicki <lpodkalicki@gmail.com>
 * Software UART for ATtiny13
 */

// (C) freexlamer@github.com


#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "soft_uart.h"
#include "m90e26.h"


void uart_init_rx_pin(){
	#ifdef    UART_RX_ENABLED
	UART_RX_PORT_REG &= ~(1 << UART_RX_PIN);
	UART_RX_DDR_REG &= ~(1 << UART_RX_PIN);
	#endif /* !UART_RX_ENABLED */
}

void uart_init_tx_pin(){
	#ifdef    UART_TX_ENABLED
	UART_TX_PORT_REG |= 1 << UART_TX_PIN;
	UART_TX_DDR_REG |= 1 << UART_TX_PIN;
	#endif /* !UART_TX_ENABLED */
}

unsigned char uart_getc(void)
{
    #ifdef    UART_RX_ENABLED
    unsigned char c;
    uint8_t sreg;

    sreg = SREG;
    cli();
    __asm volatile(
    " ldi r18, %[rxdelay2] \n\t" // 1.5 bit delay
    " ldi %0, 0x80 \n\t" // bit shift counter
    "WaitStart: \n\t"
    " sbic %[uart_port]-2, %[uart_pin] \n\t" // wait for start edge
    " rjmp WaitStart \n\t"
    "RxBit: \n\t"
    // 6 cycle loop + delay - total = 5 + 3*r22
    // delay (3 cycle * r18) -1 and clear carry with subi
    " subi r18, 1 \n\t"
    " brne RxBit \n\t"
    " ldi r18, %[rxdelay] \n\t"
    " sbic %[uart_port]-2, %[uart_pin] \n\t" // check UART PIN
    " sec \n\t"
    " ror %0 \n\t"
    " brcc RxBit \n\t"
    "StopBit: \n\t"
    " dec r18 \n\t"
    " brne StopBit \n\t"
    : "=r" (c)
    : [uart_port] "I" (_SFR_IO_ADDR(UART_RX_PORT_REG)),
    [uart_pin] "I" (UART_RX_PIN),
    [rxdelay] "I" (RXDELAY),
    [rxdelay2] "I" (RXDELAY2)
    : "r0","r18","r19"
    );
    SREG = sreg;
    sei();
    return c;
    #else
    return (0);
    #endif /* !UART_RX_ENABLED */
}

void
uart_putc(unsigned char c)
{
    #ifdef    UART_TX_ENABLED
    uint8_t sreg;

    sreg = SREG;
    cli();
    asm volatile(
    " cbi %[uart_port], %[uart_pin] \n\t" // start bit
    " in r0, %[uart_port] \n\t"
    " ldi r30, 3 \n\t" // stop bit + idle state
    " ldi r28, %[txdelay] \n\t"
    "TxLoop: \n\t"
    // 8 cycle loop + delay - total = 7 + 3*r22
    " mov r29, r28 \n\t"
    "TxDelay: \n\t"
    // delay (3 cycle * delayCount) - 1
    " dec r29 \n\t"
    " brne TxDelay \n\t"
    " bst %[ch], 0 \n\t"
    " bld r0, %[uart_pin] \n\t"
    " lsr r30 \n\t"
    " ror %[ch] \n\t"
    " out %[uart_port], r0 \n\t"
    " brne TxLoop \n\t"
    :
    : [uart_port] "I" (_SFR_IO_ADDR(UART_TX_PORT_REG)),
    [uart_pin] "I" (UART_TX_PIN),
    [txdelay] "I" (TXDELAY),
    [ch] "r" (c)
    : "r0","r28","r29","r30"
    );
    SREG = sreg;
    sei();
    #endif /* !UART_TX_ENABLED */
}

void
uart_puts(const char *s)
{
    while (*s) uart_putc(*(s++));
}

void uart2_init_rx_pin(){
	#ifdef    UART2_RX_ENABLED
	UART2_RX_PORT_REG &= ~(1 << UART2_RX_PIN);
	UART2_RX_DDR_REG &= ~(1 << UART2_RX_PIN);
	#endif /* !UART2_RX_ENABLED */
}

void uart2_init_tx_pin(){
	#ifdef    UART2_TX_ENABLED
	UART2_TX_PORT_REG |= 1 << UART2_TX_PIN;
	UART2_TX_DDR_REG |= 1 << UART2_TX_PIN;
	#endif /* !UART2_TX_ENABLED */
}

unsigned char
uart2_getc(void)
{
    #ifdef    UART2_RX_ENABLED
    unsigned char c;
    uint8_t sreg;

    sreg = SREG;
    //cli();
    __asm volatile(
    " ldi r18, %[uart2_rxdelay2] \n\t" // 1.5 bit delay
    " ldi %0, 0x80 \n\t" // bit shift counter
    "uart2_WaitStart: \n\t"
    " sbic %[timeout_ioreg], %[timeout_flag] \n\t"
    " rjmp uart2_RxExit \n\t"
    " sbic %[uart2_port]-2, %[uart2_pin] \n\t" // wait for start edge
    " rjmp uart2_WaitStart \n\t"
    "uart2_RxBit: \n\t"
    // 6 cycle loop + delay - total = 5 + 3*r22
    // delay (3 cycle * r18) -1 and clear carry with subi
    " subi r18, 1 \n\t"
    " brne uart2_RxBit \n\t"
    " ldi r18, %[uart2_rxdelay] \n\t"
    " sbic %[uart2_port]-2, %[uart2_pin] \n\t" // check UART PIN
    " sec \n\t"
    " ror %0 \n\t"
    " brcc uart2_RxBit \n\t"
    "uart2_StopBit: \n\t"
    " dec r18 \n\t"
    " brne uart2_StopBit \n\t"
    "uart2_RxExit: \n\t"
    : "=r" (c)
    : [uart2_port] "I" (_SFR_IO_ADDR(UART2_RX_PORT_REG)),
    [uart2_pin] "I" (UART2_RX_PIN),
    [uart2_rxdelay] "I" (UART2_RXDELAY),
    [uart2_rxdelay2] "I" (UART2_RXDELAY2),
    [timeout_ioreg] "I" (_SFR_IO_ADDR(TIMEOUT_IOREG)),
    [timeout_flag] "I" (TIMEOUT_FLAG)
    : "r0","r18","r19"
    );
    SREG = sreg;
    return c;
    #else
    return (0);
    #endif /* !UART2_RX_ENABLED */
}

void
uart2_putc(unsigned char c)
{
    #ifdef    UART2_TX_ENABLED
    uint8_t sreg;

    sreg = SREG;
    cli();
    asm volatile(
    " cbi %[uart2_port], %[uart2_pin] \n\t" // start bit
    " in r0, %[uart2_port] \n\t"
    " ldi r30, 3 \n\t" // stop bit + idle state
    " ldi r28, %[uart2_txdelay] \n\t"
    "uart2_TxLoop: \n\t"
    // 8 cycle loop + delay - total = 7 + 3*r22
    " mov r29, r28 \n\t"
    "uart2_TxDelay: \n\t"
    // delay (3 cycle * delayCount) - 1
    " dec r29 \n\t"
    " brne uart2_TxDelay \n\t"
    " bst %[ch], 0 \n\t"
    " bld r0, %[uart2_pin] \n\t"
    " lsr r30 \n\t"
    " ror %[ch] \n\t"
    " out %[uart2_port], r0 \n\t"
    " brne uart2_TxLoop \n\t"
    :
    : [uart2_port] "I" (_SFR_IO_ADDR(UART2_TX_PORT_REG)),
    [uart2_pin] "I" (UART2_TX_PIN),
    [uart2_txdelay] "I" (UART2_TXDELAY),
    [ch] "r" (c)
    : "r0","r28","r29","r30"
    );
    SREG = sreg;
    sei();
    #endif /* !UART2_TX_ENABLED */
}


