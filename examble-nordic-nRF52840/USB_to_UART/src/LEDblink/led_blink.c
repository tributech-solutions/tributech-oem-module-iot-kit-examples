/*
 * led_blink.c
 *
 *  Created on: 21 Feb 2022
 *      Author: AEAscher
 */

 #include <led_blink.h>
 #include <zephyr.h>
 #include <drivers/gpio.h>

 /* The devicetree node identifier for the "led0" alias. */
#define LED0_NODE DT_ALIAS(led0)

#if DT_NODE_HAS_STATUS(LED0_NODE, okay)
#define LED0	DT_GPIO_LABEL(LED0_NODE, gpios)
#define PIN	DT_GPIO_PIN(LED0_NODE, gpios)
#define FLAGS	DT_GPIO_FLAGS(LED0_NODE, gpios)
#else
/* A build error here means your board isn't set up to blink an LED. */
#error "Unsupported board: led0 devicetree alias is not defined"
#define LED0	""
#define PIN	0
#define FLAGS	0
#endif

const struct device *led;
bool led_is_on = true;
struct k_timer led_timer;

static void led_timer_handler(struct k_timer *dummy)
{
    toggle_led();
}

K_TIMER_DEFINE(led_timer, led_timer_handler, NULL);

void toggle_led()
{
    gpio_pin_set(led, PIN, (int)led_is_on);
    led_is_on = !led_is_on;
}

int init_led()
{
    int ret;
    led = device_get_binding(LED0);
    if (led == NULL) {
            return -1;
    }

    ret = gpio_pin_configure(led, PIN, GPIO_OUTPUT_ACTIVE | FLAGS);
    return ret;
}

void start_timer()
{
    k_timer_start(&led_timer, K_MSEC(1000), K_MSEC(1000));
}

