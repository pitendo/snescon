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
	int driver_usage_cnt;
};

/**
 * Timer that read and update all pads.
 * 
 * @param ptr The pointer to the snescon_config structure
 */
static void snescon_timer(unsigned long ptr) {
	struct snescon_config* cfg = (void *) ptr;
	pads_update(&(cfg->pads_cfg));
	mod_timer(&cfg->timer, jiffies + REFRESH_TIME);
}

/**
 * Module global parameter variable.
 *
 */
static struct snescon_config snescon_config = {
	.pads_cfg.gpio = {2, 3, 4, 7, 10, 11}, // Default values for the GPIOs.
	.pads_cfg.gpio_cnt = NUMBER_OF_GPIOS
};

/**
 * @brief Definition of module parameter gpio. This parameter are readable from the sysfs.
 */
module_param_array_named(gpio, snescon_config.pads_cfg.gpio, uint, &(snescon_config.pads_cfg.gpio_cnt), S_IRUGO);
MODULE_PARM_DESC(gpio, "Mapping of the 6 gpio for the driver are as following. <clk, latch, port1_d0 (data1), port2_d0 (data2), port2_d1 (data4), port2_pp (data6)>");

static int snescon_open(struct input_dev *dev) {
	struct snescon_config* cfg = input_get_drvdata(dev);
	int status;
    
	status = mutex_lock_interruptible(&cfg->mutex);
	if (status) {
		return status;
	}
    
	cfg->driver_usage_cnt++;
	if (cfg->driver_usage_cnt > 0) {
		mod_timer(&cfg->timer, jiffies + REFRESH_TIME);
	}
    
	mutex_unlock(&cfg->mutex);
	return 0;
}

static void snescon_close(struct input_dev *dev) {
	struct snescon_config* cfg = input_get_drvdata(dev);
    
	mutex_lock(&cfg->mutex);
	cfg->driver_usage_cnt--;
	if (cfg->driver_usage_cnt <= 0) {
		del_timer_sync(&cfg->timer);
	}
	mutex_unlock(&cfg->mutex);
}


/**
 * Init function for the driver.
 */
static int __init snescon_init(void) {
	/* Check if the supplied GPIO setting are useful. All GPIOs must be set for the configuration to be prevalid. */
	if (snescon_config.pads_cfg.gpio_cnt != NUMBER_OF_GPIOS) {
		pr_err("Number of GPIO pins in gpio configuration is not correct. Expected %i, actual %i\n", NUMBER_OF_GPIOS, snescon_config.pads_cfg.gpio_cnt);
		return -EINVAL;
	}
	/** @todo Add check so the provided GPIO setting are valid */

	/* Set up the gpio handler. */	
	if (gpio_init() != 0) {
		pr_err("Setup of the gpio handler failed\n");
		return -EBUSY;
	}

	/* Initiate the mutex and the timer */
        mutex_init(&snescon_config.mutex);
        setup_timer(&snescon_config.timer, snescon_timer, (long) &snescon_config);

	if (pads_setup(&snescon_config.pads_cfg)) {
		pr_err("Setup of input_device failed!\n");

		/* Cleanup allocated resourses */
		del_timer(&snescon_config.timer);
		mutex_destroy(&snescon_config.mutex);
		gpio_exit();

		return -ENODEV;
	}

	pr_info("Loaded driver\n");
    
	return 0;
}


/**
 * Exit function for the driver.
 */
static void __exit snescon_exit(void) {
	del_timer(&snescon_config.timer);
	pads_remove(&snescon_config.pads_cfg);
	mutex_destroy(&snescon_config.mutex);
	gpio_exit();

	pr_info("driver exit\n");
}

module_init(snescon_init);
module_exit(snescon_exit);

