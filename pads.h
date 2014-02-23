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

extern volatile unsigned *gpio;	///< I/O access

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

