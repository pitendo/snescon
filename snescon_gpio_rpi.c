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

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/mutex.h>
#include <linux/slab.h>

#include "pads.h"
#include "gpio.h"

#define REFRESH_TIME	HZ/100

MODULE_AUTHOR("Christian Isaksson");
MODULE_AUTHOR("Karl Thoren <karl.h.thoren@gmail.com>");
MODULE_DESCRIPTION("NES, SNES, gamepad driver for Raspberry Pi");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.0.0");

/*
 * Structure that contain pad configuration, timer and mutex.
 */
struct snescon_config {
	struct pads_config pads_cfg;
	struct timer_list timer;
	struct mutex mutex;
};

/**
 * Timer that read and update all pads.
 * 
 * @param ptr The pointer to the snescon_config structure
 */
static void snescon_timer(unsigned long ptr) {
	struct snescon_config cfg = (void *) ptr;
	pads_update(cfg->pads_cfg);
	mod_timer(&cfg->timer, jiffies + REFRESH_TIME);
}

struct config snescon_config = {
	.gpio = {2, 3, 4, 7, 10, 11}, // Default values.
	.gpio_cnt = NUMBER_OF_GPIOS
};

/**
 * @brief Definition of module parameter gpio. This parameter are readable from the sysfs.
 */
module_param_array_named(gpio, snescon_config.gpio, uint, &(snescon_config.gpio_cnt), S_IRUGO);
MODULE_PARM_DESC(gpio, "Mapping of the 6 gpio for the driver are as following. <clk, latch, port1_d0 (data1), port2_d0 (data2), port2_d1 (data4), port2_pp (data6)>");

/**
 * Init function for the driver.
 */
static int __init snescon_init(void) {

	/* Check if the used supplied a GPIO setting. All GPIOs must be set for the configuration to be prevalid. */
	if (snescon_config.gpio_cnt != NUMBER_OF_GPIOS) {
		pr_err("Number of GPIO pins in gpio configuration is not correct. Expected %i, actual %i\n", NUMBER_OF_GPIOS, snescon_config.gpio_cnt);
		return -EINVAL;
	}

	/* Set up the gpio handler. */	
	if (gpio_init() != 0) {
		pr_err("Setup of the gpio handler failed\n");
		return -EBUSY;
	}

	pr_info("Loaded driver\n");
    
	return 0;
}


/**
 * Exit function for the driver.
 */
static void __exit snescon_exit(void) {
	gpio_exit();
}

module_init(snescon_init);
module_exit(snescon_exit);

