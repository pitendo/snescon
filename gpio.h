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

#ifndef _GPIO_H_
#define _GPIO_H_

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <asm/io.h>

void gpio_set(unsigned int);
void gpio_clear(unsigned int);
void gpio_input(unsigned int);
void gpio_output(unsigned int);
unsigned char gpio_read(unsigned int);
unsigned int gpio_read_all(void);
extern int gpio_init(void) __init;
extern void gpio_exit(void);
unsigned char gpio_valid(unsigned char);
unsigned char gpio_list_valid(unsigned char *, unsigned char);
unsigned int gpio_get_bit(unsigned char);
void gpio_enable_pull_up(unsigned int);

#endif // _GPIO_H_
