/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "pico/stdlib.h"
#include "hardware/i2c.h"

#include "display/display.h"
#include "properties.h"

int main() 
	{
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
   
   	for (int x=0; x<10; x++)	
	    for (int i=0; i<5; i++)
    		dpy.setPixel(x+i,x*5+i);
    	
    // Send buffer to the display
    dpy.update();
   }

