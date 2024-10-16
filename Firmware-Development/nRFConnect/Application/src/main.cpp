#include <zephyr/kernel.h>
#include "Aled.hpp"
#include "TAS5825M.h"

int main(void)
{
    printk("Ezlo Part 2\n");
    // ALED aled;
    // aled.init();
    TAS5825M_init();

    while (1)
    {
        TAS5825M_loop();
    }

    return 0;
}