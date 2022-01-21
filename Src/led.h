#include "queue.h"

#define LED_NONE 0x0
#define LED_R 0x1
#define LED_G 0x2
#define LED_B 0x4
#define LED_RGB 0x7
#define LED_L 0x8

void led_init();
void led(uint8_t x);
uint8_t led_dequeue();
bool led_is_empty();

void led_hello();
void led_low_battery();