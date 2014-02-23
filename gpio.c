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

volatile unsigned *gpio;	// I/O access

/**
 * Set GPIO high
 *
 * @param g GPIO
 */
void gpio_set(unsigned int g) {
	GPIO_SET = g;
}

/**
 * Set GPIO low
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
 * Set GPIO as output
 *
 * @param g GPIO
 */
void gpio_output(unsigned int g) {
	OUT_GPIO(g);
}

/**
 * Read status of GPIO
 *
 * @param g GPIO
 * @return Status of GPIO
 */
unsigned char gpio_read(unsigned int g) {
	return g & ~(*(gpio + 13));
}

/**
 * Read status of all GPIOs
 *
 * @return Status of all GPIOs
 */
unsigned int gpio_read_all() {
	return ~(*(gpio + 13));;
}
