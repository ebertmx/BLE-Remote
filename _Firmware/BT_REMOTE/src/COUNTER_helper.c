
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/util.h>
#include <zephyr/types.h>
#include <zephyr/sys/printk.h>
#include <inttypes.h>

// GLOABAL
int16_t d_count = 10;

// TIMER
struct k_timer my_timer;
void reset_pushed(struct k_timer *timer_id);
void countdown(struct k_timer *timer_id);
void at_zero();

K_TIMER_DEFINE(my_timer, countdown, reset_pushed);

void reset_pushed(struct k_timer *timer_id)
{
	d_count = 10;
	printk("\nreset count = %d", d_count);
	k_timer_start(&my_timer, K_SECONDS(1), K_SECONDS(1));
}

void countdown(struct k_timer *timer_id)
{
	if(d_count>0)
	{
		d_count--;
	}
	if (d_count <= 0)
	{
		at_zero();
	}

	printk("\ncount = %d", d_count);
}

void at_zero()
{
	printk("\nOUT OF TIME");
}

void timer_init(int period)
{
	d_count = 10;
	k_timer_start(&my_timer, K_SECONDS(1), K_SECONDS(1));
}
// TIMER

// BUTTON
#define SLEEP_TIME_MS 1

#define SW0_NODE DT_ALIAS(sw0)

static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET_OR(SW0_NODE, gpios,
															  {0});
static struct gpio_callback button_cb_data;

void button_pressed(const struct device *dev, struct gpio_callback *cb,
					uint32_t pins)
{
	printk("\nButton pressed at %" PRIu32 "\n", k_cycle_get_32());
	k_timer_stop(&my_timer);
}

int button_init(void)
{
	int ret;

	if (!gpio_is_ready_dt(&button))
	{
		printk("Error: button device %s is not ready\n",
			   button.port->name);
		return 0;
	}

	ret = gpio_pin_configure_dt(&button, GPIO_INPUT);
	if (ret != 0)
	{
		printk("Error %d: failed to configure %s pin %d\n",
			   ret, button.port->name, button.pin);
		return 0;
	}

	ret = gpio_pin_interrupt_configure_dt(&button,
										  GPIO_INT_EDGE_TO_ACTIVE);
	if (ret != 0)
	{
		printk("Error %d: failed to configure interrupt on %s pin %d\n",
			   ret, button.port->name, button.pin);
		return 0;
	}

	gpio_init_callback(&button_cb_data, button_pressed, BIT(button.pin));
	gpio_add_callback(button.port, &button_cb_data);
	// printk("Set up button at %s pin %d\n", button.port->name, button.pin);

	return 1;
}

// BUTTON
