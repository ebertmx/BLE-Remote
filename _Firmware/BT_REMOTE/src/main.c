#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/types.h>
#include <stddef.h>

#include <zephyr/drivers/gpio.h>

#include <zephyr/device.h>
#include <zephyr/drivers/counter.h>

#include "BT_helper.c"
#include "UART_helper.c"
#include "BUTTON_helper.c"

// THREADS
void dodge_clock(void);
K_THREAD_DEFINE(d_clock, 1024, dodge_clock, NULL, NULL, NULL, 1, 0, 0);

// TIMERS
void shot_timer_count_cb(struct k_timer *timer_id);
void shot_timer_end_cb(struct k_timer *timer_id);
struct k_timer shot_timer;
K_TIMER_DEFINE(shot_timer, shot_timer_count_cb, shot_timer_end_cb);
K_TIMER_DEFINE(shot_timer_test, NULL, NULL);
// SEMAPHORES
K_SEM_DEFINE(scanning, 0, 1);

// GLOBALS
volatile uint8_t count;
volatile uint8_t dis_count;
volatile bool display_ack = false;

// CONSTANTS
#define COUNT_RATE 1
#define CANCEL_ACK_DELAY 500

uint8_t dev_data[] = {'R', 'E', 'M', '0', '1', 'x', 9};
struct bt_data adv_data[] = {
    BT_DATA(BT_DATA_MANUFACTURER_DATA, dev_data, 8),
};

static uint8_t conn_dev[] = {'D', 'I', 'S', '0', '1', 'x', 9};
uint8_t S_CONN_DEV = sizeof(conn_dev);

// INTERRUPTS
void button_pressed(const struct device *dev, struct gpio_callback *cb,
                    uint32_t pins);

void scan_cb(const bt_addr_le_t *addr, int8_t rssi, uint8_t adv_type,
             struct net_buf_simple *buf);

int main(void)
{
        uart_init();
        button_init();
        bluetooth_init();
        count = 10;
        printk("Main Sleep\n");
        k_sleep(K_FOREVER);

        return 0;
}

void dodge_clock()
{
        // start shot timer
        k_timer_start(&shot_timer, K_SECONDS(COUNT_RATE), K_SECONDS(COUNT_RATE));

        printk("Dodge Clock START\n");
        k_sleep(K_MSEC(1000));

        while (1)
        {

                k_timer_status_sync(&shot_timer); // wait until timer expires

                // send out count
                dev_data[6] = count;
                start_adv(adv_data);

                start_scan(); // scan for acknoledgement

                int ret = k_sem_take(&scanning, K_MSEC(CANCEL_ACK_DELAY));
                stop_adv();  // stop advertising
                stop_scan(); // stop scanning
                printk("\nret: %d    Displayed = %d\n", ret, dis_count);
        }
}

void button_pressed(const struct device *dev, struct gpio_callback *cb,
                    uint32_t pins)
{
        printk("Button pressed at %" PRIu32 "\n", k_cycle_get_32());
        k_timer_start(&shot_timer, K_SECONDS(COUNT_RATE), K_SECONDS(COUNT_RATE));
        count = 10;
}

void shot_timer_count_cb(struct k_timer *timer_id)
{
        count--;
        if (count <= 0)
        {
                count = 10;
                // k_timer_stop(&shot_timer);
        }
        printk("Timer count = %d\n", count);
}

void shot_timer_end_cb(struct k_timer *timer_id)
{
        printk("Timer end\n");
}

void scan_cb(const bt_addr_le_t *addr, int8_t rssi, uint8_t adv_type,
             struct net_buf_simple *buf)
{
        bool read_dev = false;
        int i = 0;

        // check for DIS device
        for (i = 0; i < buf->len; i++)
        {
                // iterate through device name
                if (buf->data[i] == conn_dev[0])
                {
                        read_dev = true;
                        break;
                }
        }
        if (read_dev == false)
        {
                // device is NOT a REM
                return;
        }

        char device[6];
        int k = 0;

        // Confirm device REM
        while (k < S_CONN_DEV)
        {
                if (buf->data[i] == conn_dev[k])
                {
                        device[k] = buf->data[i];
                        i++;
                        k++;
                }
                else
                {
                        read_dev = false;
                        // device is not a DIS
                        return;
                }
        }
        // device is a DIS
        if (read_dev)
        {

                // printk("(REMOTE):");
                // printk(" dev = %s  ", device);
                // printk(" count = %d\n", buf->data[i]);
                dis_count = buf->data[i];
                k_sem_give(&scanning);
        }
        else
        {
                return;
        }
        // ACK reciept of count to REM device
}