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

#define DELAY 6
#define BUFFER_SIZE 34


/*
 * Structure that contain the configuration.
 *
 * Structuring of the gpio and gamepad arrays:
 * gpio: <clk, latch, port1_d0 (data1), port2_d0 (data2), port2_d1 (data4), port2_pp (data6)>
 * pad: <pad 1, pad 2, pad 3, pad 4, pad 5>
 */
struct config {
	unsigned int gpio[6];
	struct input_dev *pad[5];
	struct timer_list timer;
	struct mutex mutex;
	unsigned char player_mode;
};

/**
 * Read the data pins of all connected devices.
 *
 * @param cfg The configuration
 * @param data Array to store the read data in
 */
void read_pads(struct config *cfg, unsigned int *data);

/**
 * Check if a SNES Multitap is connected.
 *
 * @param cfg The configuration
 * @return 1 if a SNES Multitap is connected, otherwise 0
 */
int multitap_connected(struct config *cfg);

/**
 * Check if a NES Four Score is connected.
 *
 * @param cfg The configuration
 * @return 1 if a NES Four Score is connected, otherwise 0
 */
int fourscore_connected(struct config *cfg, unsigned int *data);

/**
 * Update the status of all connected devices.
 *
 * @param cfg The configuration
 */
void update_pads(struct config *cfg);

/**
 * Clear status of buttons and axises of virtual devices
 * 
 * @param cfg The configuration
 * @param n_devs Number of devices to have all buttons and axises cleared
 */
*/
void clear_devices(struct config *cfg, unsigned char n_devs);

