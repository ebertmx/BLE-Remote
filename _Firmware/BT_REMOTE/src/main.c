#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/types.h>
#include <stddef.h>

#include <zephyr/drivers/gpio.h>

#include <zephyr/device.h>
#include <zephyr/drivers/counter.h>

#include "BT_helper.c"
#include "UART_helper.c"
#include "COUNTER_helper.c"

// GPIO
#define LED0_NODE DT_ALIAS(led0)
// static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
uint8_t adv_data[] = {'R', 'E', 'M', '0', '1', 'x', 9};


int main(void)
{

        const struct device *const dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_console));
        uint32_t dtr = 0;

#if defined(CONFIG_USB_DEVICE_STACK_NEXT)
        if (enable_usb_device_next())
        {
                return 0;
        }
#else
        if (usb_enable(NULL))
        {
                return 0;
        }
#endif

        /* Poll if the DTR flag was set */
        while (!dtr)
        {
                uart_line_ctrl_get(dev, UART_LINE_CTRL_DTR, &dtr);
                /* Give CPU resources to low priority threads. */
                k_sleep(K_MSEC(100));
        }


        bluetooth_init();

        static const struct bt_data ad[] = {
            BT_DATA(BT_DATA_MANUFACTURER_DATA, adv_data, 7),
        };

        while (1)
        {
                start_adv(ad);
                start_scan();
                k_sleep(K_MSEC(500));
                stop_adv();
                stop_scan();
                adv_data[6]--;
                k_sleep(K_MSEC(500));
        }


        while (1)
        {
                k_sleep(K_FOREVER);
        }
        return 0;
}
