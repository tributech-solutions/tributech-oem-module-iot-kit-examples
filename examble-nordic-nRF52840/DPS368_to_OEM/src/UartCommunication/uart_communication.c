/*
 * uart_communication.c
 *
 *  Created on: 26 May 2022
 *      Author: A.Ascher
 */


#include <uart_communication.h>
#include <logging/log.h>
#include <parse_and_send.h>

const struct device *uart;
struct k_sem uart_sem;

static K_MEM_SLAB_DEFINE(uart_slab, BUF_SIZE, 2, 4);
uint8_t *buf;

LOG_MODULE_REGISTER(uart_communication,LOG_LEVEL_INF);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// uart callback
void uart_callback(const struct device *dev,
                   struct uart_event *evt,
                   void *user_data)
{
    struct device *uart = user_data;
    int err;
    uint32_t first_bracket_offset = 0;

    switch (evt->type)
    {
        case UART_TX_DONE:
                LOG_INF("UART_TX_DONE");
                break;

        case UART_TX_ABORTED:
                LOG_INF("UART_TX_ABORTED");
                break;

        case UART_RX_RDY:
                // Save it to uart receive buffer
                LOG_INF("UART_RX_RDY");
                uart_rx_disable(uart);

                while(first_bracket_offset < 4096 && evt->data.rx.buf[first_bracket_offset] != 0x7B)
                {
                    first_bracket_offset++;
                }
                if(evt->data.rx.buf[first_bracket_offset] == 0x7B && evt->data.rx.len > 10)
                { 
                    // The response of a successful ProvideValue command is {"TransactionNr": X,"ErrorCode": 0}
                    // where X is the corresponding transaction number send with the command.
                    LOG_INF("%s\n\r", evt->data.rx.buf);
                    //uart_reinit();
                }
                //else
                //{
                //    uart_reinit();
                //}
                break;

        case UART_RX_BUF_REQUEST:
        {
            LOG_INF("UART_RX_BUF_REQUEST");
            uint8_t *buf;

            err = k_mem_slab_alloc(&uart_slab, (void **)&buf, K_NO_WAIT);
            __ASSERT(err == 0, "Failed to allocate slab");

            err = uart_rx_buf_rsp(uart, buf, BUF_SIZE);
            __ASSERT(err == 0, "Failed to provide new buffer");
            break;
        }

        case UART_RX_BUF_RELEASED:
            LOG_INF("UART_RX_BUF_RELEASED");
            k_mem_slab_free(&uart_slab, (void **)&evt->data.rx_buf.buf);
            break;

        case UART_RX_DISABLED:
            LOG_INF("UART_RX_DISABLED");
            break;

        case UART_RX_STOPPED:
            LOG_INF("UART_RX_STOPPED");
            break;
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// init uart
void uart_init(void)
{
    int err = 0;

    uart = device_get_binding("UART_1");
    __ASSERT(uart, "Failed to get the device");

    err = k_mem_slab_alloc(&uart_slab, (void **)&buf, K_NO_WAIT);
    __ASSERT(err == 0, "Failed to alloc slab");

    err = uart_callback_set(uart, uart_callback, (void *)uart);   // set callback
    __ASSERT(err == 0, "Failed to set callback");

    err = uart_rx_enable(uart, buf, BUF_SIZE, 100);    // enable receiving
    __ASSERT(err == 0, "Failed to enable RX"); 
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// reinit uart
void uart_reinit(void)
{
    int err = 0;

    err = uart_rx_enable(uart, buf, BUF_SIZE, 100);    // enable receiving
    __ASSERT(err == 0, "Failed to enable RX");

    k_sleep(K_MSEC(10)); 
}