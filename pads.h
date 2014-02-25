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

#ifndef _PADS_H_
#define _PADS_H_

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
#define NUMBER_OF_GPIOS 6
#define NUMBER_OF_INPUT_DEVICES 5


/*
 * Structure that contain the configuration.
 *
 * Structuring of the gpio and gamepad arrays:
 * gpio: <clk, latch, port1_d0 (data1), port2_d0 (data2), port2_d1 (data4), port2_pp (data6)>
 * pad: <pad 1, pad 2, pad 3, pad 4, pad 5>
 *
 */
struct pads_config {
	unsigned int gpio[NUMBER_OF_GPIOS];
	struct input_dev *pad[NUMBER_OF_INPUT_DEVICES];
	unsigned char player_mode;
	char *device_name;
	int (* open) (struct input_dev *dev);
	void (* close) (struct input_dev *dev);
	bool enable_multitap;
	bool enable_fourscore;
};

extern void pads_update(struct pads_config *);
extern int  pads_setup(struct pads_config *) __init;
extern void pads_remove(struct pads_config *) __exit;

#endif /* _PADS_H_ */
