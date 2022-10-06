/*
 * uart_communication.c
 *
 *  Created on: 21 Feb 2022
 *      Author: AEAscher
 */

#include <stdio.h>
#include <string.h>
#include <device.h>
#include <drivers/uart.h>
#include <zephyr.h>
//#include <sys/ring_buffer.h>

#include <usb/usb_device.h>
#include <logging/log.h>
#include <uart_communication.h>
#include <sys/reboot.h>

#define UART_BUF_SIZE 8192

static K_FIFO_DEFINE(usb_0_tx_fifo);
static K_FIFO_DEFINE(uart_1_tx_fifo);

struct uart_data {
	void *fifo_reserved;
	uint8_t buffer[UART_BUF_SIZE];
	uint16_t len;
}uart_data_t;

static struct serial_dev {
	const struct device *dev;
	void *peer;
	struct k_fifo *fifo;
	struct k_sem sem;
	struct uart_data *rx;
} devs[2];

/* Frees data for incoming transmission on device blocked by full heap. */
static int oom_free(struct serial_dev *sd)
{
	struct serial_dev *peer_sd = (struct serial_dev *)sd->peer;
	struct uart_data *buf;

	/* First, try to free from FIFO of peer device (blocked stream) */
	buf = k_fifo_get(peer_sd->fifo, K_NO_WAIT);
	if (buf) {
		k_free(buf);
		return 0;
	}

	/* Then, try FIFO of the receiving device (reverse of blocked stream) */
	buf = k_fifo_get(sd->fifo, K_NO_WAIT);
	if (buf) {
		k_free(buf);
		return 0;
	}

	/* Finally, try all of them */
	for (int i = 0; i < ARRAY_SIZE(devs); i++) {
		buf = k_fifo_get(sd->fifo, K_NO_WAIT);
		if (buf) {
			k_free(buf);
			return 0;
		}
	}

	return -1; /* Was not able to free any heap memory */
}

static void uart_interrupt_handler(const struct device *int_dev, void *user_data)
{
	struct serial_dev *sd = user_data;
	struct device *dev = sd->dev;
	struct serial_dev *peer_sd = (struct serial_dev *)sd->peer;

	uart_irq_update(dev);

	while (uart_irq_rx_ready(dev)) {
		int data_length;

		while (!sd->rx) {
			sd->rx = k_malloc(sizeof(*sd->rx));
			if (sd->rx) {
				sd->rx->len = 0;
			} else {
				int err = oom_free(sd);

				if (err) {
					printk("Could not free memory. Rebooting.\n");
					sys_reboot(SYS_REBOOT_COLD);
				}
			}
		}

		data_length = uart_fifo_read(dev, &sd->rx->buffer[sd->rx->len],
					   UART_BUF_SIZE - sd->rx->len);
		sd->rx->len += data_length;

		if (sd->rx->len > 0) {
			// if ((sd->rx->len == UART_BUF_SIZE) ||
			//    (sd->rx->buffer[sd->rx->len - 1] == '\n') ||
			//    (sd->rx->buffer[sd->rx->len - 1] == '\r') ||
			//    (sd->rx->buffer[sd->rx->len - 1] == '\0')) {
			if ((sd->rx->len == UART_BUF_SIZE) ||
			   (sd->rx->buffer[sd->rx->len - 1] == '\n')) {
				if(sd->rx->buffer[sd->rx->len - 1] == '\n' && sd->rx->buffer[sd->rx->len - 2] != '\r')
				{
					sd->rx->buffer[sd->rx->len - 1] = '\r';
					sd->rx->buffer[sd->rx->len] = '\n';
					sd->rx->len++;
				}
				k_fifo_put(peer_sd->fifo, sd->rx);
				k_sem_give(&peer_sd->sem);

				sd->rx = NULL;
			}
		}
	}

	if (uart_irq_tx_ready(dev)) {
		struct uart_data *buf = k_fifo_get(sd->fifo, K_NO_WAIT);
		uint16_t written = 0;

		/* Nothing in the FIFO, nothing to send */
		if (!buf) {
			uart_irq_tx_disable(dev);
			return;
		}

		while (buf->len > written) {
			written += uart_fifo_fill(dev,
						  &buf->buffer[written],
						  buf->len - written);
		}

		while (!uart_irq_tx_complete(dev)) {
			/* Wait for the last byte to get
			 * shifted out of the module
			 */
		}

		if (k_fifo_is_empty(sd->fifo)) {
			uart_irq_tx_disable(dev);
		}

		k_free(buf);
	}
}

void usb_2_uart_init()
{
    int ret;

    struct serial_dev *usb_0_sd = &devs[0];
    struct serial_dev *uart_1_sd =&devs[1];
    const struct device *usb_0_dev;
    const struct device *uart_1_dev;
    
    usb_0_dev = device_get_binding("CDC_ACM_0");
    if (!usb_0_dev) {
            printk("CDC ACM device not found\n");
            return;
    }

    uart_1_dev = device_get_binding("UART_1");
    if (!uart_1_dev) {
            printk("UART 1 init failed\n");
    }

    usb_0_sd->dev = usb_0_dev;
    usb_0_sd->fifo = &usb_0_tx_fifo;
    usb_0_sd->peer = uart_1_sd;

    uart_1_sd->dev = uart_1_dev;
    uart_1_sd->fifo = &uart_1_tx_fifo;
    uart_1_sd->peer = usb_0_sd;

    k_sem_init(&usb_0_sd->sem, 0, 1);
    k_sem_init(&uart_1_sd->sem, 0, 1);


    uart_irq_callback_user_data_set(usb_0_dev, uart_interrupt_handler, usb_0_sd);
    uart_irq_callback_user_data_set(uart_1_dev, uart_interrupt_handler, uart_1_sd);

    ret = usb_enable(NULL);
    if (ret != 0) {
            printk("Failed to enable USB\n");
            return;
    }

    uart_irq_rx_enable(usb_0_dev);
    uart_irq_rx_enable(uart_1_dev);

    printk("USB <--> UART bridge is now initialized\n");

    struct k_poll_event events[2] = {
            K_POLL_EVENT_STATIC_INITIALIZER(K_POLL_TYPE_SEM_AVAILABLE,
                                            K_POLL_MODE_NOTIFY_ONLY,
                                            &usb_0_sd->sem, 0),
            K_POLL_EVENT_STATIC_INITIALIZER(K_POLL_TYPE_SEM_AVAILABLE,
                                            K_POLL_MODE_NOTIFY_ONLY,
                                            &uart_1_sd->sem, 0),
    };

    while (1) 
    {
        ret = k_poll(events, ARRAY_SIZE(events), K_FOREVER);
        if (ret != 0) 
        {
                continue;
        }

        if (events[0].state == K_POLL_TYPE_SEM_AVAILABLE) 
        {
                events[0].state = K_POLL_STATE_NOT_READY;
                k_sem_take(&usb_0_sd->sem, K_NO_WAIT);
                uart_irq_tx_enable(usb_0_dev);
        }
        else if (events[1].state == K_POLL_TYPE_SEM_AVAILABLE) 
        {
                events[1].state = K_POLL_STATE_NOT_READY;
                k_sem_take(&uart_1_sd->sem, K_NO_WAIT);
                uart_irq_tx_enable(uart_1_dev);
        }
    }
}