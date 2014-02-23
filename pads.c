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

#include "gpio.h"
#include "pads.h"

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

	gpio_set(clk | latch);
	udelay(DELAY * 2);
	gpio_clear(latch);

	for (i = 0; i < BUFFER_SIZE / 2; i++) {
		udelay (DELAY);
		gpio_clear(clk);
		data[i] = gpio_read_all();
		udelay(DELAY);
		gpio_set(clk);
	}

	// Set PP low
	gpio_clear(pp);

	for (; i < BUFFER_SIZE; i++) {
		udelay (DELAY);
		gpio_clear(clk);
		data[i] = gpio_read_all();
		udelay(DELAY);
		Ggpio_set(clk);
	}

	// Set PP high
	gpio_set(pp);
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
	gpio_input(d0);
	gpio_output(d0);

	// Set D0 high
	gpio_set(d0);
	gpio_set(clk);
	udelay (DELAY);

	// Read D1 eight times
	for (i = 0; i < 8; i++) {
		udelay(DELAY);
		gpio_clear(clk);

		// Check if D1 is low
		if (!gpio_read(d1)) {
			return 0;
		}
		udelay(DELAY);
		gpio_set(clk);
	}

	// Set D0 low
	gpio_clear(d0);

	// Read D1 eight times
	for (i = 0; i < 8; i++) {
		udelay(DELAY);
		gpio_clear(clk);

		// Check if D1 is high
		if (gpio_read(d1)) {
			byte += 1;
		}
		byte <<= 1;
		udelay(DELAY);
		gpio_set(clk);
	}

	// Set D0 to input
	gpio_input(d0);

	if (byte == 0xFF) {
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
	unsigned char i, j;
	struct input_dev *dev;

	read_pads(cfg, data);

	if (multitap_connected(cfg)) {
		// SNES Multitap

		// Set 5 player mode
		cfg->player_mode = 5;

		// Player 1
		dev = cfg->pad[0];
		g = cfg->gpio[2];

		for (j = 0; j < 8; j++) {
			input_report_key(dev, btn_label[j], g & data[btn_index[j]]);
		}
		input_report_abs(dev, ABS_X, !(g & data[6]) - !(g & data[7]));
		input_report_abs(dev, ABS_Y, !(g & data[4]) - !(g & data[5]));
		input_sync(dev);

		// Player 2
		dev = cfg->pad[1];
		g = cfg->gpio[3];

		for (j = 0; j < 8; j++) {
			input_report_key(dev, btn_label[j], g & data[btn_index[j]]);
		}
		input_report_abs(dev, ABS_X, !(g & data[6]) - !(g & data[7]));
		input_report_abs(dev, ABS_Y, !(g & data[4]) - !(g & data[5]));
		input_sync(dev);

		// Player 3
		dev = cfg->pad[2];
		g = cfg->gpio[4];

		for (j = 0; j < 8; j++) {
			input_report_key(dev, btn_label[j], g & data[btn_index[j]]);
		}
		input_report_abs(dev, ABS_X, !(g & data[6]) - !(g & data[7]));
		input_report_abs(dev, ABS_Y, !(g & data[4]) - !(g & data[5]));
		input_sync(dev);

		// Player 4
		dev = cfg->pad[3];
		g = cfg->gpio[3];

		for (j = 0; j < 8; j++) {
			input_report_key(dev, btn_label[j], g & data[btn_index[j] + 17]);
		}
		input_report_abs(dev, ABS_X, !(g & data[23]) - !(g & data[24]));
		input_report_abs(dev, ABS_Y, !(g & data[21]) - !(g & data[22]));
		input_sync(dev);

		// Player 5
		dev = cfg->pad[4];
		g = cfg->gpio[4];

		for (j = 0; j < 8; j++) {
			input_report_key(dev, btn_label[j], g & data[btn_index[j] + 17]);
		}
		input_report_abs(dev, ABS_X, !(g & data[23]) - !(g & data[24]));
		input_report_abs(dev, ABS_Y, !(g & data[21]) - !(g & data[22]));
		input_sync(dev);

	} else if (fourscore_connected(cfg, data)) {
		// NES Four Score

		// Player 1 and 2
		for (i = 0; i < 2; i++) {
			dev = cfg->pad[i];
			g = cfg->gpio[i + 2];

			for (j = 0; j < 4; j++) {
				input_report_key(dev, btn_label[j], g & data[btn_index[j]]);
			}
			input_report_abs(dev, ABS_X, !(g & data[6]) - !(g & data[7]));
			input_report_abs(dev, ABS_Y, !(g & data[4]) - !(g & data[5]));
			input_sync(dev);
		}

		// Player 3 and 4
		for (i = 2; i < 4; i++) {
			dev = cfg->pad[i];
			g = cfg->gpio[i];

			for (j = 0; j < 4; j++) {
				input_report_key(dev, btn_label[j], g & data[btn_index[j] + 8]);
			}
			input_report_abs(dev, ABS_X, !(g & data[14]) - !(g & data[15]));
			input_report_abs(dev, ABS_Y, !(g & data[12]) - !(g & data[13]));
			input_sync(dev);
		}
		
		// Check if virtual device 5 should be cleared and if player_mode should be changed to 4 player mode
		if (cfg->player_mode > 4) {
			cfg->player_mode = 4;
			clear_devices(cfg, 1);
		} else if (cfg->player_mode < 4) {
			cfg->player_mode = 4;
		}
	} else {
		// NES or SNES gamepad

		// Player 1 and 2
		for (i = 0; i < 2; i++) {
			dev = cfg->pad[i];
			g = cfg->gpio[i + 2];

			for (j = 0; j < 8; j++) {
				input_report_key(dev, btn_label[j], g & data[btn_index[j]]);
			}
			input_report_abs(dev, ABS_X, !(g & data[6]) - !(g & data[7]));
			input_report_abs(dev, ABS_Y, !(g & data[4]) - !(g & data[5]));
			input_sync(dev);
		}

		// Check if virtual devices 3, 4 and 5 should be cleared and player_mode should be changed to 2 player mode
		if (cfg->player_mode > 2) {
			cfg->player_mode = 2;
			clear_devices(cfg, 3);
		}
	}
}

/**
 * Clear status of buttons and axises of virtual devices
 * 
 * @param cfg The configuration
 * @param n_devs Number of devices to have all buttons and axises cleared
 */
*/
void clear_devices(struct config *cfg, unsigned char n_devs) {
	int i;
	for(i = 0; i < n_pads; i++) {
		dev = cfg->pad[5 - i];
		for (j = 0; j < 8; j++) {
			input_report_key(dev, btn_label[j], 0);
		}
		input_report_abs(dev, ABS_X, 0);
		input_report_abs(dev, ABS_Y, 0);
		input_sync(dev);
	}
}

