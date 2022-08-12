/**
 * @file  serial.c
 * @brief Handle communication with UART interface
 *
 * @author Saint-Genest Gwenael <gwen@cowlab.fr>
 * @copyright Cowlab (c) 2022
 *
 * @page License
 * This firmware is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3 as published
 * by the Free Software Foundation. You should have received a copy of the
 * GNU General Public License along with this program, see LICENSE.md file
 * for more details.
 * This program is distributed WITHOUT ANY WARRANTY.
 */
#include "pico/stdlib.h"
#include "hardware/irq.h"
#include "hardware/uart.h"
#include "serial.h"

static void serial_irq(void);

static uint8_t rx_buffer[SERIAL_RX_SZ];
static volatile int rx_index_r, rx_index_w;

/**
 * @brief Initialize the "serial" module
 *
 * This function initialize the serial module and configure UART interface. For
 * this driver to work properly, this function must be called before any other
 * serial functions.
 */
void serial_init(void)
{
	/* Initialize indexes of buffers */
	rx_index_r = 0;
	rx_index_w = 0;

	uart_init(uart1, 115200);

	gpio_set_function(8, GPIO_FUNC_UART);
	gpio_set_function(9, GPIO_FUNC_UART);

	/* Set default/initial UART configuration */
	uart_set_hw_flow(uart1, false, false);
	uart_set_format (uart1, 8, 1, UART_PARITY_NONE);
	uart_set_fifo_enabled(uart1, false);

	/* Configure interrupts */
	irq_set_exclusive_handler(UART1_IRQ, serial_irq);
	irq_set_enabled(UART1_IRQ, true);
	uart_set_irq_enables(uart1, true, false);
}

/**
 * @brief Read bytes from rx buffer
 *
 * Bytes received from UART are handled by interrupt (see serial_irq) and put
 * into a buffer. This function allow to extract bytes from this rx buffer.
 *
 * @param buffer Pointer to a buffer where to put data
 * @param len    Maximum number of bytes to read
 * @return integer Number of readed bytes
 */
int serial_read(char *buffer, int len)
{
	int count = 0;

	/* Sanity check */
	if (buffer == 0)
		return(0);

	while (count < len)
	{
		/* If the rx buffer is empty, nothing more to do */
		if (rx_index_r == rx_index_w)
			break;

		/* Read one byte from rx buffer */
		*buffer++ = rx_buffer[rx_index_r];
		count++;
		/* Update index */
		rx_index_r++;
		if (rx_index_r == SERIAL_RX_SZ)
			rx_index_r = 0;
	}
	return(count);
}

/**
 * @brief Get the number of bytes available into receive buffer
 *
 * @return integer Number of received bytes
 */
int serial_rx_avail(void)
{
	int r_copy, w_copy;
	int len;

	if (rx_index_r == rx_index_w)
		return(0);

	/* Save a copy of indexes */
	r_copy = rx_index_r;
	w_copy = rx_index_w;

	if (w_copy > r_copy)
		len = (w_copy - r_copy);
	else
		len = (SERIAL_RX_SZ - r_copy) + w_copy;

	return(len);
}

/**
 * @brief Send bytes to UART
 *
 * @param data Pointer to a buffer with data to send
 * @param len  Number of bytes to send
 */
void serial_write(uint8_t *data, int len)
{
	int i;

	/* Sanity check */
	if (data == 0)
		return;

	for (i = 0; i < len; i++)
		uart_putc_raw(uart1, data[i]);
}

/**
 * @brief UART interrupt handler
 *
 * This function is called when an interrupt is raised by the UART peripheral
 * (mainly on received byte event)
 */
static void serial_irq(void)
{
	uart_hw_t *dev;
	uint32_t ris;
	uint8_t c;
	int idx_next;

	dev = uart_get_hw(uart1);

	/* Get the Raw Interrupt Status */
	ris = dev->ris;

	/* Receive Timeout is not really used, disable it */
	if (ris & (1 << 6))
		dev->imsc &= ~(1 << 6);

	/* If one byte has been received */
	if (ris & UART_UARTRIS_RXRIS_BITS)
	{
		c = (dev->dr & 0xFF);
		rx_buffer[rx_index_w] = c;

		/* Compute index of next byte into circular buffer */
		idx_next = rx_index_w + 1;
		if (idx_next == SERIAL_RX_SZ)
			idx_next = 0;
		/* If buffer is not full, update write index */
		if (idx_next != rx_index_r)
			rx_index_w = idx_next;
	}
}
/* EOF */
