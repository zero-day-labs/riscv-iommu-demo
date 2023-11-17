#include <uart.h>
#include <8250_uart.h>

#define CVA6_DEFAULT_UART_ADDR		    0x10000000
#define CVA6_DEFAULT_UART_FREQ		    50000000
#define CVA6_DEFAULT_UART_BAUDRATE		115200
#define CVA6_DEFAULT_UART_REG_SHIFT	    2
#define CVA6_DEFAULT_UART_REG_WIDTH	    4


void uart_init(){
    uart8250_init(CVA6_DEFAULT_UART_ADDR,
                  CVA6_DEFAULT_UART_FREQ,
			      CVA6_DEFAULT_UART_BAUDRATE,
                  CVA6_DEFAULT_UART_REG_SHIFT,
                  CVA6_DEFAULT_UART_REG_WIDTH);
}

void uart_putc(char c)
{
    uart8250_putc(c);
}

char uart_getchar(void)
{
    return uart8250_getc();
}

void uart_enable_rxirq()
{
    uart8250_enable_rx_int();
}

void uart_clear_rxirq()
{
    uart8250_interrupt_handler(); 
}
