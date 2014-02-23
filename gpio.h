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

// GPIO setup macros. Always use INP_GPIO(x) before using OUT_GPIO(x)
#define INP_GPIO(g) *(gpio+((g)/10)) &= ~(7<<(((g)%10)*3))	// Set GPIO as input.
#define OUT_GPIO(g) *(gpio+((g)/10)) |=  (1<<(((g)%10)*3))	// Set GPIO as output.

#define GPIO_SET *(gpio + 7)	// Sets bits which are 1 and ignores bits which are 0.
#define GPIO_CLR *(gpio + 10)	// Clears bits which are 1 and ignores bits which are 0.

void gpio_set(unsigned int);
void gpio_clear(unsigned int);
void gpio_input(unsigned int);
void gpio_output(unsigned int);
unsigned char gpio_read(unsigned int);
unsigned int gpio_read_all(void);
