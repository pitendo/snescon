/*
 * NES, SNES gamepad functionality
 *
 * Copyright (c) 2014 Christian Isaksson
 * Copyright (c) 2014 Karl Thoren <karl.h.thoren@gmail.com>
 */

/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/mutex.h>
#include <linux/slab.h>

#include <linux/ioport.h>
#include <asm/io.h>

#include "pads.h"

// GPIO setup macros. Always use INP_GPIO(x) before using OUT_GPIO(x)
#define INP_GPIO(g) *(gpio+((g)/10)) &= ~(7<<(((g)%10)*3))	// Set GPIO as input.
#define OUT_GPIO(g) *(gpio+((g)/10)) |=  (1<<(((g)%10)*3))	// Set GPIO as output.

#define GPIO_SET *(gpio + 7)	// Sets bits which are 1 and ignores bits which are 0.
#define GPIO_CLR *(gpio + 10)	// Clears bits which are 1 and ignores bits which are 0.

#define DELAY 6
#define BUFFER_SIZE 34

volatile unsigned *gpio;	// I/O access

// The number of bits reported by each supported type of gamepad and accessory.
const unsigned char bit_length[] = { 8, 24, 12, 24, 32 };

// Buttons found on the SNES gamepad.
const short btn_label[] = { BTN_B, BTN_Y, BTN_SELECT, BTN_START, BTN_A, BTN_X, BTN_TL, BTN_TR };

// The order that the buttons of the SNES gamepad are stored in the byte string.
const unsigned char btn_index[] = { 0, 1, 2, 3, 8, 9, 10, 11 };

/**
 * Read the data pins of all connected devices.
 *
 * @param cfg The configuration
 * @param data Array to store the read data in
 */
void read_pads(struct config *cfg, unsigned int *data) {
	int i;
	unsigned int clk, latch, pp;
	
	clk = cfg->gpio[0];
	latch = cfg->gpio[1];
	pp = cfg->gpio[5];
    
	GPIO_SET = clk | latch;
	udelay(DELAY * 2);
	GPIO_CLR = latch;
	
	for(i = 0; i < BUFFER_SIZE / 2; i++) {
		udelay(DELAY);
		GPIO_CLR = clk;
		data[i] = ~(*(gpio + 13));
		udelay(DELAY);
		GPIO_SET = clk;
	}
	
	// Set PP low
	GPIO_CLR = pp;
	
	for(; i < BUFFER_SIZE; i++) {
		udelay(DELAY);
		GPIO_CLR = clk;
		data[i] = ~(*(gpio + 13));
		udelay(DELAY);
		GPIO_SET = clk;
	}
	
	// Set PP high
	GPIO_SET = pp;
}

/**
 * Check if a SNES Multitap is connected.
 *
 * @param cfg The configuration
 * @return 1 if a SNES Multitap is connected, otherwise 0
 */
int multitap_connected(struct config *cfg) {
	int i;
	unsigned char byte = 0;
	unsigned int clk, latch, d0, d1;
	
	// Store GPIOs in variables
	clk = cfg->gpio[0];
	latch = cfg->gpio[1];
	d0 = cfg->gpio[3];
	d1 = cfg->gpio[4];
	
	// Set D0 to output
	INP_GPIO(d0);
	OUT_GPIO(d0);
	
	// Set D0 high
	GPIO_SET = d0;
	GPIO_SET = clk;
	udelay(DELAY);
	
	// Read D1 eight times
	for(i = 0; i < 8; i++) {
		udelay(DELAY);
		GPIO_CLR = clk;
		
		// Check if D1 is low
		if(!(d1 & ~(*(gpio + 13)))) {
			return 0;
		}
		udelay(DELAY);
		GPIO_SET = clk;
	}

	// Set D0 low
	GPIO_CLR = d0;
	
	// Read D1 eight times
	for(i = 0; i < 8; i++) {
		udelay(DELAY);
		GPIO_CLR = clk;
		
		// Check if D1 is high
		if(d1 & ~(*(gpio + 13))) {
			byte += 1;
		}
		byte <<= 1;
		udelay(DELAY);
		GPIO_SET = clk;
	}
	
	// Set D0 to input
	INP_GPIO(d0);
	
	if(byte == 0xFF) {
		return 0;
	}
	return 1;
}

/**
 * Check if a NES Four Score is connected.
 *
 * @param cfg The configuration
 * @return 1 if a NES Four Score is connected, otherwise 0
 */
int fourscore_connected(struct config *cfg, unsigned int *data) {
	return !(cfg->gpio[2] & data[16]) &&
	       !(cfg->gpio[2] & data[17]) &&
	       !(cfg->gpio[2] & data[18]) &&
	        (cfg->gpio[2] & data[19]) &&
	       !(cfg->gpio[2] & data[20]) &&
	       !(cfg->gpio[2] & data[21]) &&
	       !(cfg->gpio[2] & data[22]) &&
	       !(cfg->gpio[2] & data[23]) &&
	       !(cfg->gpio[3] & data[16]) &&
	       !(cfg->gpio[3] & data[17]) &&
	        (cfg->gpio[3] & data[18]) &&
	       !(cfg->gpio[3] & data[19]) &&
	       !(cfg->gpio[3] & data[20]) &&
	       !(cfg->gpio[3] & data[21]) &&
	       !(cfg->gpio[3] & data[22]) &&
	       !(cfg->gpio[3] & data[23]);
}


/**
 * Update the status of all connected devices.
 *
 * @param cfg The configuration
 */
void update_pads(struct config *cfg) {
	unsigned int g, data[BUFFER_SIZE];
	unsigned char i, j, multitap;
	struct input_dev *dev;
	
	read_pads(cfg, data);
	
	if(multitap_connected(cfg)) {
		// SNES Multitap
		
		// Player 1
		dev = cfg->pad[0];
		g = cfg->gpio[2];
		
		for (j = 0; j < 8; j++) {
			input_report_key(dev, btn_label[j], g & data[btn_index[j]]);
		}
		input_report_abs(dev, ABS_X, !(g & data[6]) - !(g & data[7]));
		input_report_abs(dev, ABS_X, !(g & data[4]) - !(g & data[5]));
		input_sync(dev);
		
		// Player 2
		dev = cfg->pad[1];
		g = cfg->gpio[3];
		
		for (j = 0; j < 8; j++) {
			input_report_key(dev, btn_label[j], g & data[btn_index[j]]);
		}
		input_report_abs(dev, ABS_X, !(g & data[6]) - !(g & data[7]));
		input_report_abs(dev, ABS_X, !(g & data[4]) - !(g & data[5]));
		input_sync(dev);
		
		// Player 3
		dev = cfg->pad[2];
		g = cfg->gpio[4];
		
		for (j = 0; j < 8; j++) {
			input_report_key(dev, btn_label[j], g & data[btn_index[j]]);
		}
		input_report_abs(dev, ABS_X, !(g & data[6]) - !(g & data[7]));
		input_report_abs(dev, ABS_X, !(g & data[4]) - !(g & data[5]));
		input_sync(dev);
		
		// Player 4
		dev = cfg->pad[3];
		g = cfg->gpio[3];
		
		for (j = 0; j < 8; j++) {
			input_report_key(dev, btn_label[j], g & data[btn_index[j] + 17]);
		}
		input_report_abs(dev, ABS_X, !(g & data[23]) - !(g & data[24]));
		input_report_abs(dev, ABS_X, !(g & data[21]) - !(g & data[22]));
		input_sync(dev);
		
		// Player 5
		dev = cfg->pad[4];
		g = cfg->gpio[4];
		
		for (j = 0; j < 8; j++) {
			input_report_key(dev, btn_label[j], g & data[btn_index[j] + 17]);
		}
		input_report_abs(dev, ABS_X, !(g & data[23]) - !(g & data[24]));
		input_report_abs(dev, ABS_X, !(g & data[21]) - !(g & data[22]));
		input_sync(dev);
		
	} else if(fourscore_connected(cfg, data)) {
		// NES Four Score
		
		// Player 1
		dev = cfg->pad[0];
		g = cfg->gpio[2];
		
		for (j = 0; j < 4; j++) {
			input_report_key(dev, btn_label[j], g & data[btn_index[j]]);
		}
		input_report_abs(dev, ABS_X, !(g & data[6]) - !(g & data[7]));
		input_report_abs(dev, ABS_X, !(g & data[4]) - !(g & data[5]));
		input_sync(dev);
		
		// Player 2
		dev = cfg->pad[1];
		g = cfg->gpio[3];
		
		for (j = 0; j < 4; j++) {
			input_report_key(dev, btn_label[j], g & data[btn_index[j]]);
		}
		input_report_abs(dev, ABS_X, !(g & data[6]) - !(g & data[7]));
		input_report_abs(dev, ABS_X, !(g & data[4]) - !(g & data[5]));
		input_sync(dev);
		
		// Player 3
		dev = cfg->pad[2];
		g = cfg->gpio[2];
		
		for (j = 0; j < 4; j++) {
			input_report_key(dev, btn_label[j], g & data[btn_index[j] + 8]);
		}
		input_report_abs(dev, ABS_X, !(g & data[14]) - !(g & data[15]));
		input_report_abs(dev, ABS_Y, !(g & data[12]) - !(g & data[13]));
		input_sync(dev);
		
		// Player 4
		dev = cfg->pad[2];
		g = cfg->gpio[3];
		
		for (j = 0; j < 4; j++) {
			input_report_key(dev, btn_label[j], g & data[btn_index[j] + 8]);
		}
		input_report_abs(dev, ABS_X, !(g & data[14]) - !(g & data[15]));
		input_report_abs(dev, ABS_Y, !(g & data[12]) - !(g & data[13]));
		input_sync(dev);
	} else {
		// NES or SNES gamepad
		
		// Player 1
		dev = cfg->pad[0];
		g = cfg->gpio[2];
		
		for (j = 0; j < 8; j++) {
			input_report_key(dev, btn_label[j], g & data[btn_index[j]]);
		}
		input_report_abs(dev, ABS_X, !(g & data[6]) - !(g & data[7]));
		input_report_abs(dev, ABS_X, !(g & data[4]) - !(g & data[5]));
		input_sync(dev);
		
		// Player 2
		dev = cfg->pad[1];
		g = cfg->gpio[3];
		
		for (j = 0; j < 8; j++) {
			input_report_key(dev, btn_label[j], g & data[btn_index[j]]);
		}
		input_report_abs(dev, ABS_X, !(g & data[6]) - !(g & data[7]));
		input_report_abs(dev, ABS_X, !(g & data[4]) - !(g & data[5]));
		input_sync(dev);
	}
}

