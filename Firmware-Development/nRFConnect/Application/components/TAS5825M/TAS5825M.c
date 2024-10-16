/*
 * Copyright (c) 2020 Antmicro <www.antmicro.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/drivers/i2s.h>
#include <zephyr/logging/log.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "TAS5825M.h"

LOG_MODULE_REGISTER(tas5825m, LOG_LEVEL_INF); // Initialize logging module
static const struct device *i2s_dev_tx = DEVICE_DT_GET(DT_ALIAS(speaker));

// #define I2S_TX_NODE  DT_NODELABEL(i2s_rxtx)

#define SAMPLE_FREQUENCY 44100
#define SAMPLE_BIT_WIDTH 32
#define BYTES_PER_SAMPLE sizeof(int32_t)
#define NUMBER_OF_CHANNELS 1
/* Such block length provides an echo with the delay of 100 ms. */
#define SAMPLES_PER_BLOCK ((SAMPLE_FREQUENCY / 10) * NUMBER_OF_CHANNELS)
#define INITIAL_BLOCKS 2
#define TIMEOUT 1000

#define BLOCK_SIZE (BYTES_PER_SAMPLE * SAMPLES_PER_BLOCK)
#define BLOCK_COUNT (INITIAL_BLOCKS + 10)
K_MEM_SLAB_DEFINE_STATIC(mem_slab, BLOCK_SIZE, BLOCK_COUNT, BYTES_PER_SAMPLE);

#define NUM_SAMPLES 32
/* Sine Wave Sample */
static int16_t data_frame[NUM_SAMPLES] = {
    6392/4,
    12539/4,
    18204/4,
    30272/4,
    32137/4,
    32137/4,
    30272/4,
    27244/4,
    23169/4,
    18204/4,
    12539/4,
    6392/4,
    0/4,
    -6393/4,
    -12540/4,
    -18205/4,
    -23170/4,
    -27245/4,
    -30273/4,
    -32138/4,
    -32767/4,
    -32138/4,
    -30273/4,
    -27245/4,
    -23170/4,
    -18205/4,
    -12540/4,
    -6393/4,
    -1/4,
};

/* Define a new Memory Slab which consistes of NUM_BLOCKS blocks
   __________________________________________________________________________
  |    Block 0   |    Block 1   |    Block 2   |    Block 3   |    Block 4   |
  |    0...31    |    0...31    |    0...31    |    0...31    |    0...31    |
  |______________|______________|______________|______________|______________|
*/

static void process_block_data(void *mem_block, uint32_t number_of_samples)
{
    #define PI 3.14159265358979323846
    #define SINE_FREQUENCY 1378 
    #define AMPLITUDE 32767 
    int16_t *sample = &((int16_t *)mem_block)[0];
    for (int i = 0; i < number_of_samples; i++) {
        sample[i] = data_frame[i % NUM_SAMPLES];
        // Calculate the sine wave value for each sample
        // float t = (float)i / SAMPLE_FREQUENCY; // Time in seconds for the current sample
        // float sine_value = sinf(2 * PI * SINE_FREQUENCY * t); // Generate sine wave value
        // sample[i] = (int16_t)(AMPLITUDE * sine_value); // Scale to 16-bit range and store in the buffer
    }
}

static bool configure_streams(const struct device *i2s_dev_tx,
			      const struct i2s_config *config)
{
	int ret;

    ret = i2s_configure(i2s_dev_tx, I2S_DIR_BOTH, config);
    if (ret == 0) {
        return true;
    }
    /* -ENOSYS means that the RX and TX streams need to be
        * configured separately.
        */
    if (ret != -ENOSYS) {
        printk("Failed to configure streams: %d\n", ret);
        return false;
    }

	return true;
}

static bool prepare_transfer(const struct device *i2s_dev_tx)
{
	int ret;

	for (int i = 0; i < INITIAL_BLOCKS; ++i) {
		void *mem_block;

		ret = k_mem_slab_alloc(&mem_slab, &mem_block, K_NO_WAIT);
		if (ret < 0) {
			printk("Failed to allocate TX block %d: %d\n", i, ret);
			return false;
		}

		memset(mem_block, 0, BLOCK_SIZE);

		ret = i2s_write(i2s_dev_tx, mem_block, BLOCK_SIZE);
		if (ret < 0) {
			printk("Failed to write block %d: %d\n", i, ret);
			return false;
		}
	}

	return true;
}

static bool trigger_command(const struct device *i2s_dev_tx,
			    enum i2s_trigger_cmd cmd)
{
	int ret;

    ret = i2s_trigger(i2s_dev_tx, I2S_DIR_BOTH, cmd);
    if (ret == 0) {
        return true;
    }
    /* -ENOSYS means that commands for the RX and TX streams need
        * to be triggered separately.
        */
    if (ret != -ENOSYS) {
        printk("Failed to trigger command %d: %d\n", cmd, ret);
        return false;
    }

	return true;
}

DeviceStatus TAS5825M_init(void)
{
    printk("I2S echo sample\n");

    if (!device_is_ready(i2s_dev_tx))
    {
        printk("%s is not ready\n", i2s_dev_tx->name);
        return DEV_ERROR;
    }

    struct i2s_config config = {0};
    config.word_size = SAMPLE_BIT_WIDTH;
    config.channels = NUMBER_OF_CHANNELS;
    config.format = I2S_FMT_DATA_FORMAT_I2S;
    config.options = I2S_OPT_BIT_CLK_MASTER | I2S_OPT_FRAME_CLK_MASTER;
    config.frame_clk_freq = SAMPLE_FREQUENCY;
    config.mem_slab = &mem_slab;
    config.block_size = BLOCK_SIZE;
    config.timeout = TIMEOUT;
    if (!configure_streams(i2s_dev_tx, &config))
    {
        return DEV_ERROR;
    }

    if (!prepare_transfer(i2s_dev_tx))
    {
        return DEV_ERROR;
    }

    if (!trigger_command(i2s_dev_tx, I2S_TRIGGER_START))
    {
        return DEV_ERROR;
    }

    return DEV_OK;
}

DeviceStatus TAS5825M_loop(void)
{  
    void *mem_block;
    uint32_t block_size;
    int ret;

    ret = i2s_read(i2s_dev_tx, &mem_block, &block_size);
    if (ret < 0) {
        LOG_ERR("Failed to read data: %d\n", ret);
        return DEV_ERROR;
    }
    // LOG_INF("%d\n", block_size);
    // ret = k_mem_slab_alloc(&mem_slab, &mem_block, K_NO_WAIT);
    // if (ret < 0) {
    //     printk("Failed to allocate TX block %d\n", ret);
    //     return DEV_ERROR;
    // }
    // LOG_INF("%p\n", mem_block);
    // process_block_data(mem_block, SAMPLES_PER_BLOCK);
    // int32_t *sample = &((int32_t *)mem_block)[0];
    // for (size_t i = 0; i < SAMPLES_PER_BLOCK; i++)
    // {
    //     printk("%d\n", sample[i]);
    // }
    // memset(mem_block, 0, block_size * BYTES_PER_SAMPLE);
    
    ret = i2s_write(i2s_dev_tx, mem_block, block_size);
    if (ret < 0) {
        LOG_ERR("Failed to write data: %d\n", ret);
        return DEV_ERROR;
    }

    // if (!trigger_command(i2s_dev_rx, i2s_dev_tx,
    // 		     I2S_TRIGGER_DROP)) {
    // 	return 0;
    // }

    return DEV_OK;
}