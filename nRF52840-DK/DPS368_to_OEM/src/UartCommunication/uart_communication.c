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
const struct device *usb;

struct k_sem uart_sem;

static K_MEM_SLAB_DEFINE(uart_slab, BUF_SIZE, 2, 4);
static K_MEM_SLAB_DEFINE(usb_slab, BUF_SIZE, 2, 4);

uint8_t *buf_oem;
uint8_t *buf_usb;

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
                LOG_INF("OEM UART_TX_DONE");
                break;

        case UART_TX_ABORTED:
                LOG_INF("OEM UART_TX_ABORTED");
                break;

        case UART_RX_RDY:
                // Save it to uart receive buffer
                LOG_INF("OEM UART_RX_RDY");
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
            LOG_INF("OEM UART_RX_BUF_REQUEST");
            uint8_t *buf_oem;

            err = k_mem_slab_alloc(&uart_slab, (void **)&buf_oem, K_NO_WAIT);
            __ASSERT(err == 0, "Failed to allocate slab");

            err = k_mem_slab_alloc(&usb_slab, (void **)&buf_usb, K_NO_WAIT);
            __ASSERT(err == 0, "Failed to allocate slab");

            err = uart_rx_buf_rsp(uart, buf_oem, BUF_SIZE);
            __ASSERT(err == 0, "Failed to provide new buffer for OEM communication");

            err = uart_rx_buf_rsp(usb, buf_usb, BUF_SIZE);
            __ASSERT(err == 0, "Failed to provide new buffer for USB communication");
            break;
        }

        case UART_RX_BUF_RELEASED:
            LOG_INF("OEM UART_RX_BUF_RELEASED");
            k_mem_slab_free(&uart_slab, (void **)&evt->data.rx_buf.buf);
            break;

        case UART_RX_DISABLED:
            LOG_INF("OEM UART_RX_DISABLED");
            break;

        case UART_RX_STOPPED:
            LOG_INF("OEM UART_RX_STOPPED");
            break;
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// usb callback
void usb_callback(const struct device *dev,
                   struct uart_event *evt,
                   void *user_data)
{

    switch (evt->type)
    {
        case UART_TX_DONE:
            LOG_INF("USB UART_TX_DONE");
            break;

        case UART_TX_ABORTED:
            LOG_INF("USB UART_TX_ABORTED");
            break;

        case UART_RX_RDY:
            LOG_INF("USB UART_RX_RDY");
            break;

        case UART_RX_BUF_REQUEST:
            LOG_INF("USB UART_RX_BUF_REQUEST");
            break;

        case UART_RX_BUF_RELEASED:
            LOG_INF("USB UART_RX_BUF_RELEASED");
            break;

        case UART_RX_DISABLED:
            LOG_INF("USB UART_RX_DISABLED");
            break;

        case UART_RX_STOPPED:
            LOG_INF("USB UART_RX_STOPPED");
            break;
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// init uart
void uart_init(void)
{
    int err = 0;

    uart = device_get_binding("UART_1");
    __ASSERT(uart, "Failed to get the UART_1 device");

    usb = device_get_binding("CDC_ACM_0");
    __ASSERT(usb == 0, "Failed to get the CDC_ACM_0 (USB) device");

    err = k_mem_slab_alloc(&uart_slab, (void **)&buf_oem, K_NO_WAIT);
    __ASSERT(err == 0, "Failed to alloc slab");

    err = k_mem_slab_alloc(&usb_slab, (void **)&buf_usb, K_NO_WAIT);
    __ASSERT(err == 0, "Failed to alloc slab");

    err = uart_callback_set(uart, uart_callback, (void *)uart);   // set callback
    __ASSERT(err == 0, "Failed to set OEM callback");

    err = uart_callback_set(usb, usb_callback, (void *)usb);   // set callback
    __ASSERT(err == 0, "Failed to set USB callback");

    err = uart_rx_enable(uart, buf_oem, BUF_SIZE, 100);    // enable receiving
    __ASSERT(err == 0, "Failed to enable RX OEM"); 

    err = uart_rx_enable(usb, buf_usb, BUF_SIZE, 100);    // enable receiving
    __ASSERT(err == 0, "Failed to enable RX USB"); 
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// reinit uart
void uart_reinit(void)
{
    int err = 0;

    err = uart_rx_enable(uart, buf_oem, BUF_SIZE, 100);    // enable receiving
    __ASSERT(err == 0, "Failed to enable RX");

    k_sleep(K_MSEC(10)); 
}