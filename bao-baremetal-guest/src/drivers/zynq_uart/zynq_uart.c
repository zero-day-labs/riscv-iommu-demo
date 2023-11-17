/**
 * Bao, a Lightweight Static Partitioning Hypervisor
 *
 * Copyright (c) Bao Project (www.bao-project.org), 2019-
 *
 * Authors:
 *      Jose Martins <jose.martins@bao-project.org>
 *      Sandro Pinto <sandro.pinto@bao-project.org>
 *
 * Bao is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License version 2 as published by the Free
 * Software Foundation, with a special exception exempting guest code from such
 * license. See the COPYING file in the top-level directory for details.
 *
 */

#include <zynq_uart.h>

bool xil_uart_init(Xil_Uart* uart)
{
    uint32_t ret;

    ret = xil_uart_set_baud_rate(uart, UART_BAUD_RATE);
    if (ret == false) {
        return false;
    }

    /* Set the level of the RxFIFO trigger level */
    uart->rx_fifo_trig = UART_RX_TRIGGER_LVL;
    /* Program the Receiver Timeout Mechanism (Disabled) */
    uart->rx_timeout = UART_RX_TIMEOUT_DIS;

    /* Clear all the interrupts in Interrupt Status Register */
    uart->isr_status = 0xFFFFFFFF;
    /* Enable RxFIFO Trigger Interrupt */
    uart->isr_en = UART_ISR_EN_RTRIG;

    /** Enable (closer to Reset) the Controller */
    uart->control |=
        (UART_CONTROL_STPBRK | UART_CONTROL_RXRES | UART_CONTROL_TXRES);

    return true;
}

void xil_uart_enable(Xil_Uart* uart)
{
    uint32_t ctrl_reg = uart->control;

    ctrl_reg = (UART_CONTROL_STPBRK | UART_CONTROL_TXEN | UART_CONTROL_RXEN |
                UART_CONTROL_RXRES | UART_CONTROL_TXRES);

    uart->control = ctrl_reg;
}

void xil_uart_disable(Xil_Uart* uart)
{
    uint32_t ctrl_reg = uart->control;

    ctrl_reg = (UART_CONTROL_STPBRK | UART_CONTROL_TXDIS | UART_CONTROL_RXDIS);

    uart->control = ctrl_reg;
}

bool xil_uart_set_baud_rate(Xil_Uart* uart, uint32_t baud_rate)
{
    // uint32_t sel_clk =  UART_FREQ_CLK;
    uint8_t bdiv = 0;
    uint16_t cd_calc = 0;

    /** Handling corner case */
    if (baud_rate == 0) {
        baud_rate = UART_BAUD_RATE;
    }

    /* baud_rate = sel_clk / (CD * (BDIV+1))
     * baud_rate -> Baud Rate
     * sel_clk -> Selected Clock
     * CD -> Baud Rate Generator
     * BDIV -> Baud Rate Divider
     */
    // TODO - Add support for auto Baud Rate generation */
    bdiv = UART_BDIV_115200;
    cd_calc = UART_CD_115200;

    /** Configure the Baud Rate */
    /* Disable the Rx and Tx path */
    uart->control = (UART_CONTROL_RXDIS | UART_CONTROL_TXDIS);
    /* Write the calculated CD value */
    uart->br_gen = cd_calc;
    /* Write the calculated BDIV value */
    uart->br_div = bdiv;
    /* Reset Tx and Rx paths */
    uart->control = (UART_CONTROL_TXRES | UART_CONTROL_RXRES);
    /* Enable the Rx and Tx path */
    uart->control = (UART_CONTROL_TXEN | UART_CONTROL_RXEN);

    return true;
}

uint32_t xil_uart_getc(Xil_Uart* uart)
{
    uint32_t data = 0;

    // Chose one of the following: (Trigger Level or Not Empty)
    /* Wait until RxFIFO is filled up to the trigger level */
    while (!uart->ch_status & UART_CH_STATUS_RTRIG)
        ;
    /* Wait until RxFIFO is not empty */
    // while(!uart->ch_status & UART_CH_STATUS_REMPTY);

    data = uart->tx_rx_fifo;

    return data;
}

void xil_uart_putc(Xil_Uart* uart, int8_t c)
{
    /* Wait until txFIFO is not full */
    while (uart->ch_status & UART_CH_STATUS_TFUL)
        ;

    uart->tx_rx_fifo = c;
}

void xil_uart_puts(Xil_Uart* uart, const char* s)
{
    while (*s) {
        xil_uart_putc(uart, *s++);
    }
}

void xil_uart_enable_irq(Xil_Uart* uart, uint32_t irq){
	uart->isr_en = irq;
	uart->isr_mask |= irq;
}

void xil_uart_clear_irq(Xil_Uart* uart, uint32_t irq){
	uart->isr_status = irq;
}

void xil_uart_clear_rxbuf(Xil_Uart* uart){
	while(uart->ch_status & UART_CH_STATUS_RTRIG){
		(void)xil_uart_getc(uart);
	}
}
