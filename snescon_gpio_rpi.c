/*
 * NES, SNES, gamepad driver for Raspberry Pi
 *
 *  Copyright (c) 2014	Christian Isaksson
 *  Copyright (c) 2014	Karl Thoren <karl.h.thoren@gmail.com>
 */

/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
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

#include "pads.h"

MODULE_AUTHOR("Christian Isaksson, Karl Thoren <karl.h.thoren@gmail.com>");
MODULE_DESCRIPTION("NES, SNES, gamepad driver for Raspberry Pi");
MODULE_LICENSE("GPL");

#define BCM2708_PERI_BASE        0x20000000
#define GPIO_BASE                (BCM2708_PERI_BASE + 0x200000) /* GPIO controller */


/**
 * Init function for the driver.
 */
static int __init snescon_init(void) {
	/* Set up gpio pointer for direct register access */
	if ((gpio = ioremap(GPIO_BASE, 0xB0)) == NULL) {
		pr_err("io remap failed\n");
		return -EBUSY;
	}
    
	return 0;
}


/**
 * Exit function for the driver.
 */
static void __exit snescon_exit(void) {
	iounmap(gpio);
}

module_init(snescon_init);
module_exit(snescon_exit);

