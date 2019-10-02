/**
 * Copyright (c) 2017, Łukasz Marcin Podkalicki <lpodkalicki@gmail.com>
 * Software UART for ATtiny13
 */

// (C) freexlamer@github.com


#include "settings.h"

#define    UART_RX_ENABLED        (1) // Enable UART RX
#define    UART_TX_ENABLED        (1) // Enable UART TX

#if defined(UART_TX_ENABLED) && !defined(UART_TX)
# define        UART_TX_PIN         PA6
# define		UART_TX_DDR_REG		DDRA
# define		UART_TX_PORT_REG	PORTA
#endif  /* !UART_TX */

#if defined(UART_RX_ENABLED) && !defined(UART_RX)
# define        UART_RX_PIN         PA5
# define		UART_RX_DDR_REG		DDRA
# define		UART_RX_PORT_REG	PORTA
#endif  /* !UART_RX */

#if (defined(UART_TX_ENABLED) || defined(UART_RX_ENABLED)) && !defined(UART_BAUDRATE)
# define        UART_BAUDRATE   (9600)
#endif  /* !UART_BAUDRATE */

#define TXDELAY             (int)(((F_CPU/UART_BAUDRATE)-7 +1.5)/3)
#define RXDELAY             (int)(((F_CPU/UART_BAUDRATE)-5 +1.5)/3)
#define RXDELAY2            (int)((RXDELAY*1.5)-2.5)
#define RXROUNDED           (((F_CPU/UART_BAUDRATE)-5 +2)/3)
#if RXROUNDED > 127
# error Low baud rates unsupported - use higher UART_BAUDRATE
#endif

#define    UART2_RX_ENABLED        (1) // Enable UART RX
#define    UART2_TX_ENABLED        (1) // Enable UART TX

#if defined(UART2_TX_ENABLED) && !defined(UART2_TX)
# define        UART2_TX_PIN		PA1
# define		UART2_TX_DDR_REG	DDRA
# define		UART2_TX_PORT_REG	PORTA
#endif  /* !UART2_TX */

#if defined(UART2_RX_ENABLED) && !defined(UART2_RX)
# define		UART2_RX_PIN		PA0
# define		UART2_RX_DDR_REG	DDRA
# define		UART2_RX_PORT_REG	PORTA
#endif  /* !UART2_RX */

#if (defined(UART2_TX_ENABLED) || defined(UART2_RX_ENABLED)) && !defined(UART2_BAUDRATE)
# define        UART2_BAUDRATE   (9600)
#endif  /* !UART2_BAUDRATE */

#define UART2_TXDELAY             (int)(((F_CPU/UART2_BAUDRATE)-7 +1.5)/3)
#define UART2_RXDELAY             (int)(((F_CPU/UART2_BAUDRATE)-5 +1.5)/3)
#define UART2_RXDELAY2            (int)((UART2_RXDELAY*1.5)-2.5)
#define UART2_RXROUNDED           (((F_CPU/UART2_BAUDRATE)-5 +2)/3)
#if UART2_RXROUNDED > 127
# error Low baud rates unsupported - use higher UART2_BAUDRATE
#endif

//********** uart Prototypes **********//
void uart_init_rx_pin();
void uart_init_tx_pin();
unsigned char uart_getc();
void uart_putc(unsigned char c);
void uart_puts(const char *s);

void uart2_init_rx_pin();
void uart2_init_tx_pin();
unsigned char uart2_getc();
void uart2_putc(unsigned char c);
