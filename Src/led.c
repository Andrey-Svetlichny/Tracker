#include "led.h"

QUEUE_DECLARATION(led_queue, uint8_t, 8);
QUEUE_DEFINITION(led_queue, uint8_t);
struct led_queue led_queue;

void led_init()
{
    led_queue_init(&led_queue);
}

void led(uint8_t x)
{
    led_queue_enqueue(&led_queue, &x);
}

uint8_t led_dequeue()
{
    uint8_t x;
    led_queue_dequeue(&led_queue, &x);
    return x;
}

bool led_is_empty()
{
    return led_queue_is_empty(&led_queue);
}

void led_hello()
{
    led(LED_RGB | LED_L);
    led(LED_R);
    led(LED_G);
    led(LED_B);
}

void led_low_battery()
{
    led(LED_RGB);
    led(LED_NONE);
    led(LED_RGB);
    led(LED_NONE);
    led(LED_RGB);
    led(LED_NONE);
}