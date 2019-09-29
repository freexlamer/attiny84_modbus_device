#include "settings.h"

#define    UART_RX_ENABLED        (1) // Enable UART RX
#define    UART_TX_ENABLED        (1) // Enable UART TX

#if defined(UART_TX_ENABLED) && !defined(UART_TX)
# define        UART_TX         PA6
#endif  /* !UART_TX */

#if defined(UART_RX_ENABLED) && !defined(UART_RX)
# define        UART_RX         PA5
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


//********** uart Prototypes **********//

char uart_getc();
void uart_putc(char c);
void uart_puts(const char *s);
