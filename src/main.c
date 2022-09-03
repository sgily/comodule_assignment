/*
 * Copyright (c) 2022 Libre Solar Technologies GmbH
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <device.h>
#include <drivers/uart.h>
#include <string.h>
#include <parser.h>
#include <bt.h>

/* change this to any other UART peripheral if desired */
#define UART_DEVICE_NODE DT_CHOSEN(zephyr_shell_uart)

#define MSG_SIZE 40 // Max input length

/* queue to store up to 10 messages (aligned to 4-byte boundary) */
K_MSGQ_DEFINE(uart_msgq, MSG_SIZE, 10, 4);

static const struct device *uart_dev = DEVICE_DT_GET(UART_DEVICE_NODE);

/* receive buffer used in UART ISR callback */
static char rx_buf[MSG_SIZE];
static int rx_buf_pos;


/*
 * Read characters from UART until line end is detected. Afterwards push the
 * data to the message queue.
 */
void serial_cb(const struct device *dev, void *user_data)
{
	uint8_t c;

	if (!uart_irq_update(uart_dev)) {
		return;
	}

	while (uart_irq_rx_ready(uart_dev)) {

		uart_fifo_read(uart_dev, &c, 1);

		if ((c == '\n' || c == '\r') && rx_buf_pos > 0) {
			/* terminate string */
			rx_buf[rx_buf_pos] = '\0';

			/* if queue is full, message is silently dropped */
			k_msgq_put(&uart_msgq, &rx_buf, K_NO_WAIT);

			/* reset the buffer (it was copied to the msgq) */
			rx_buf_pos = 0;
		} else if (rx_buf_pos < (sizeof(rx_buf) - 1)) {
			rx_buf[rx_buf_pos++] = c;
		}
		/* else: characters beyond buffer size are dropped */
	}
}

/*
 * Print a null-terminated string character by character to the UART interface
 */
void print_uart(char *buf)
{
	int msg_len = strlen(buf);

	for (int i = 0; i < msg_len; i++) {
		uart_poll_out(uart_dev, buf[i]);
	}
}

static void print_help() {
	print_uart("Supported format is Decimal Degrees\r\n");
	print_uart("e.g. 123.456789, 12.3456789\r\n");
	print_uart("If you see error, then check your input and try again\r\n");
}

static void flush(char* buf, size_t length) {
	for (int i=0; i<length; ++i) {
		buf[i] = 0;
	}
}

void main(void)
{
	char tx_buf[MSG_SIZE];
	int err;
	if (!device_is_ready(uart_dev)) {
		printk("UART device not found!");
		return;
	}
	/* configure interrupt and callback to receive data */
	uart_irq_callback_user_data_set(uart_dev, serial_cb, NULL);
	uart_irq_rx_enable(uart_dev);

	/* Enable bluetooth*/
	err = start_bt();
	if (err) {
		printk("Bluetooth initialization failed");
	}
	
	print_uart("Please type in GPS coordinates\r\n");
	print_help();

	/* indefinitely wait for input from the user */
	while (k_msgq_get(&uart_msgq, &tx_buf, K_FOREVER) == 0) {
		// parse buffer
		if (!strcmp("help", tx_buf)) {
			print_help();
		} else {
			// add comodule company id to the beginning
			// of advertising data
			char out_buf[26] = {0x0f, 0x02};
			err = format_coordinates(tx_buf, MSG_SIZE, &out_buf[2], ADV_SIZE-2);
			// if error, print error and wait again
			if (err) {
				switch (err)
				{
				case -EINVAL:
					print_uart("Bad input format, check for typing errors\r\n");
					break;
				case -ERANGE:
					print_uart("GPS data out of range\r\n");
					break;
				default:
					print_uart("Input parsing error\r\n");
					break;
				}
			} else {
				err = update_bt_data(out_buf, ARRAY_SIZE(out_buf));
				if (err)
					print_uart("Failed to update adv data\r\n");
				else
					print_uart("GPS location data updated\r\n");
			}
		}
		flush(tx_buf, ARRAY_SIZE(tx_buf));
		print_uart("\r\n");
	}
}
