/*
 * GPIO functionality for Raspberry Pi
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

volatile unsigned *gpio;	// I/O access.

// GPIO setup macros. Always use INP_GPIO(x) before using OUT_GPIO(x)
#define INP_GPIO(g) *(gpio+((g)/10)) &= ~(7<<(((g)%10)*3))	// Set GPIO as input.
#define OUT_GPIO(g) *(gpio+((g)/10)) |=  (1<<(((g)%10)*3))	// Set GPIO as output.

#define GPIO_SET *(gpio + 7)	// Sets bits which are 1 and ignores bits which are 0.
#define GPIO_CLR *(gpio + 10)	// Clears bits which are 1 and ignores bits which are 0.

#define BCM2708_PERI_BASE        0x20000000
#define GPIO_BASE                (BCM2708_PERI_BASE + 0x200000) // GPIO controller.

/*
 * All GPIOs found on the Raspberry Pi P1 Header and how many they are in total.
 */
#define N_VALID_GPIO 20;
const unsigned char all_valid_gpio[] = { 0, 1, 2, 3, 4, 7, 8, 9, 10, 11, 14, 15, 17, 18, 21, 22, 23, 24, 25, 27 };

/**
 * Set GPIO high.
 *
 * @param g GPIO
 */
void gpio_set(unsigned int g) {
	GPIO_SET = g;
}

/**
 * Set GPIO low.
 *
 * @param g GPIO
 */
void gpio_clear(unsigned int g) {
	GPIO_CLR = g;
}

/**
 * Set GPIO as input
 *
 * @param g GPIO
 */
void gpio_input(unsigned int g) {
	INP_GPIO(g);
}

/**
 * Set GPIO as output.
 *
 * @param g GPIO
 */
void gpio_output(unsigned int g) {
	OUT_GPIO(g);
}

/**
 * Read status of GPIO.
 *
 * @param g GPIO
 * @return Status of GPIO
 */
unsigned char gpio_read(unsigned int g) {
	return g & *(gpio + 13);
}

/**
 * Read and negate status of all GPIOs.
 *
 * @return Negated status of all GPIOs
 */
unsigned int gpio_read_all() {
	return ~(*(gpio + 13));;
}

/**
 * Init function for the gpio part of the driver.
 *
 * @return Result of the init operation
 */
int __init gpio_init(void) {
	/* Set up gpio pointer for direct register access */
	if ((gpio = ioremap(GPIO_BASE, 0xB0)) == NULL) {
		pr_err("io remap failed\n");
		return -EBUSY;
	}

	return 0;
}

/**
 * Exit function for the gpio part of the driver.
 */
void gpio_exit(void) {
	iounmap(gpio);
}

/**
 * Check if a GPIO number is valid.
 * 
 * @param g GPIO number to test validness of
 * @return 1 if g is valid, otherwise 0
 */
unsigned char gpio_valid(unsigned char g) {
	int i;
	for(i = 0; i < N_VALID_GPIO; i++) {
		if(g == all_valid_gpio[i]) {
			return 1;
		}
	}
	return 0;
}

/**
 * Calculate the bit in the GPIO register that a specific GPIO number corresponds to .
 * 
 * @param g The GPIO number
 * @return The bit that GPIO g corresponds to in the GPIO register
 */
unsigned int gpio_get_bit(unsigned char g) {
	int i, res;
	if(g == 0) {
		return 1;
	}
	res = 2;
	for(i = 1; i < g; i++) {
		res *= 2;
	}
	return res
}
