/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>

#include "can2040.h"
#include "pico/stdlib.h"
#include "hardware/i2c.h"

#include "display/display.h"
#include "properties.h"

#include "RP2040.h" // hw_set_bits
#include "irq.h"

static struct can2040 cbus;

static void can2040_cb(struct can2040 *cd, uint32_t notify, struct can2040_msg *msg)
	{
	const char * remoteFrame = (msg->id & CAN2040_ID_RTR) ? ("[RMT]") : ("     ");
	const char * extended    = (msg->id & CAN2040_ID_EFF) ? ("[EXT]") : ("     ");
	
    printf("CAN %x %s %s : ", remoteFrame, extended);
    for (int i=0; i<msg->dlc; i++)
    	printf(" %02x", msg->data[i]);
    printf("\n");
	}

static void PIOx_IRQHandler(void)
	{
    can2040_pio_irq_handler(&cbus);
	}

void canbus_setup(void)
	{
    uint32_t pio_num = 0;
    uint32_t sys_clock = 125000000, bitrate = 1000000;
    uint32_t gpio_rx = 28, gpio_tx = 29;

    // Setup canbus
    can2040_setup(&cbus, pio_num);
    can2040_callback_config(&cbus, can2040_cb);

    // Enable irqs
    irq_set_exclusive_handler(PIO0_IRQ_0_IRQn, PIOx_IRQHandler);
    NVIC_SetPriority(PIO0_IRQ_0_IRQn, 1);
    NVIC_EnableIRQ(PIO0_IRQ_0_IRQn);

    // Start canbus
    can2040_start(&cbus, sys_clock, bitrate, gpio_rx, gpio_tx);
	}


int main() 
	{
	stdio_init_all();
	
	// Initialise the I2C controller
    i2c_init(i2c1, 1000000);
 
     // Set up pins 26 and 27
    gpio_set_function(26, GPIO_FUNC_I2C);
    gpio_set_function(27, GPIO_FUNC_I2C);
    gpio_pull_up(26);
    gpio_pull_up(27);
    
    // If you don't do anything before initializing a display pi pico is too fast and starts sending
    // commands before the screen controller had time to set itself up, so we add an artificial delay for
    // ssd1306 to set itself up
    sleep_ms(250);
    
    
    Display dpy(i2c1, 0x3c, Display::Size::W128xH64);
    dpy.flip(false);
   
	dpy.rect(0,0,127,15);
	//dpy.rect(10, 20, 117, 30);
	
    // Send buffer to the display
    dpy.update();
   
   	canbus_setup();

   	int count = 0;
   	while (1)
   		{
  		sleep_ms(1000);
   		}
   	}

