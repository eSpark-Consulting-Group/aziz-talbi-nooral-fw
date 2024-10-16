// #include <zephyr/kernel.h>
// #include <zephyr/drivers/led_strip.h>
// #include <zephyr/device.h>
// #include <zephyr/drivers/spi.h>
// #include <zephyr/sys/util.h>

// #include "Aled.hpp"

// #define STRIP_NODE		DT_ALIAS(led_strip)
// #define STRIP_NUM_PIXELS	DT_PROP(DT_ALIAS(led_strip), chain_length)

// #define RGB(_r, _g, _b) { .r = (_r), .g = (_g), .b = (_b) }

// static const struct led_rgb colors[] = {
// 	RGB(0x00, 0x00, 0x00), /* Black/off */
// 	RGB(0x0f, 0x00, 0x00), /* red */
// 	RGB(0x00, 0x0f, 0x00), /* green */
// 	RGB(0x00, 0x00, 0x0f), /* blue */
// 	RGB(0x0f, 0x0f, 0x0f), /* white */
// };

// struct led_rgb pixels[STRIP_NUM_PIXELS];

// static const struct device *strip = DEVICE_DT_GET(STRIP_NODE);

// #define STACK_SIZE 1024
// #define PRIORITY 5

// // Define stack and control structure for your thread
// K_THREAD_STACK_DEFINE(my_stack, STACK_SIZE);
// struct k_thread aled_thread_data;

// void setColorSingle(int color, int pixel_index)
// {
//     memcpy(&pixels[pixel_index], &colors[color], sizeof(struct led_rgb));
// }

// // Define your thread function
// void aled_thread(void * arg1, void * arg2, void * arg3) {
    
//     ALED* aled = (ALED*) arg1;
// 	printk("RGB Thread Started!\n");

//     while (1) {
//     	printk("RGB Thread Running!\n");
// 		aled->setColor(RED, STRIP_NUM_PIXELS);
// 		k_sleep(ALED_RGB_DELAY_TIME);

// 		aled->setColor(GREEN, STRIP_NUM_PIXELS);
// 		k_sleep(ALED_RGB_DELAY_TIME);

// 		aled->setColor(BLUE, STRIP_NUM_PIXELS);
// 		k_sleep(ALED_RGB_DELAY_TIME);

// 		aled->setColor(WHITE, STRIP_NUM_PIXELS);
// 		k_sleep(ALED_RGB_DELAY_TIME);

// 		aled->setColor(BLACK, STRIP_NUM_PIXELS);
// 		k_sleep(ALED_RGB_DELAY_TIME);
//     }
// }

// void ALED::init()
// {
// 	if (device_is_ready(strip)) {
// 		printk("Found LED strip device %s", strip->name);
// 	} else {
	
// 		printk("LED strip device %s is not ready", strip->name);
// 	}
// 	setColorSingle(WHITE, STRIP_NUM_PIXELS);


// 	#if 1
//     /* LED Thread intialize */
//     // Initialize the thread
//     k_thread_create(&aled_thread_data, my_stack, STACK_SIZE,
//                     aled_thread, this, NULL, NULL,
//                     PRIORITY, 0, K_NO_WAIT);

//     // Start the thread
//     k_thread_start(&aled_thread_data);
// 	#endif

// }

// void ALED::setColor(int color, int n)
// {
//     for(int i = 0; i < n; i++)
//     {
//         setColorSingle(color, i);
//     }
//     led_strip_update_rgb(strip, pixels, STRIP_NUM_PIXELS);
// }