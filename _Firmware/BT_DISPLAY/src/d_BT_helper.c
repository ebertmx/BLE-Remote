#include <zephyr/types.h>
#include <stddef.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/util.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>

bool start_scan();
bool stop_scan();
bool start_adv();
bool stop_adv();
extern void scan_cb(const bt_addr_le_t *addr, int8_t rssi, uint8_t adv_type,
			 struct net_buf_simple *buf);
int bluetooth_init(void);



static struct bt_le_scan_param scan_param = {
	.type = BT_HCI_LE_SCAN_PASSIVE,
	.options = BT_LE_SCAN_OPT_NONE,
	.interval = 0x00a0,
	.window = 0x0080,
};

bool start_scan()
{
	int err = bt_le_scan_start(&scan_param, scan_cb);
	if (err)
	{
		printk("Starting scanning failed (err %d)\n", err);
		return false;
	}
	return true;
}

bool stop_scan()
{
	int err = bt_le_scan_stop();
	if (err)
	{
		printk("Starting scanning failed (err %d)\n", err);
		return false;
	}
	return true;
}

bool start_adv(const struct bt_data *ad)
{
	int err = bt_le_adv_start(BT_LE_ADV_NCONN, ad, 1,
							  NULL, 0);
	if (err)
	{
		printk("Advertising failed to start (err %d)\n", err);
		return false;
	}
	return true;
}

bool stop_adv()
{
	int err = bt_le_adv_stop();
	if (err)
	{
		printk("Advertising failed to stop (err %d)\n", err);
		return false;
	}
	return true;
}



int bluetooth_init(void)
{
	int err;

	// printk("Starting Scanner/Advertiser Demo\n");

	/* Initialize the Bluetooth Subsystem */
	err = bt_enable(NULL);
	if (err)
	{
		printk("Bluetooth init failed (err %d)\n", err);
		return 0;
	}

	printk("Bluetooth initialized\n");

	return 1;
}