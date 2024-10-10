#include <zephyr/kernel.h>
#include "Aled.hpp"

int main(void)
{
    printk("Ezlo Part 2\n");
    ALED aled;
    aled.init();

    return 0;
}
