/* main.c - Application main entry point */

/*
 * Copyright (c) 2015-2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/types.h>
#include <stddef.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/util.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>

volatile uint8_t count;

static uint8_t mfg_data[] = {'R', 'E', 'M', '0', '1', 'x', 9};
static uint8_t conn_dev[] = {'D', 'I', 'S', '0', '1', 'x'};
uint8_t S_CONN_DEV = sizeof(conn_dev);

static const struct bt_data ad[] = {
	BT_DATA(BT_DATA_MANUFACTURER_DATA, mfg_data, 7),
};

bool start_scan()
{
}

bool stop_scan()
{
}

bool start_adv()
{
}

bool stop_adv()
{
}

static void scan_cb(const bt_addr_le_t *addr, int8_t rssi, uint8_t adv_type,
					struct net_buf_simple *buf)
{

	bool read_dev = false;
	int i = 0;
	// check for DIS device
	for (i; i < buf->len; i++)
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
		// display count
		count = buf->data[i];
		printk("(REMOTE):");
		printk(" dev = %s  ", device);
		printk(" count = %d\n", mfg_data[6]);
		mfg_data[6] = count;
	}
	else
	{
		return;
	}
	// ACK reciept of count to REM device
}

int bluetooth_init(void)
{
	struct bt_le_scan_param scan_param = {
		.type = BT_HCI_LE_SCAN_PASSIVE,
		.options = BT_LE_SCAN_OPT_NONE,
		.interval = 0x00a0,
		.window = 0x0080,
	};
	int err;

	printk("Starting Scanner/Advertiser Demo\n");

	/* Initialize the Bluetooth Subsystem */
	err = bt_enable(NULL);
	if (err)
	{
		printk("Bluetooth init failed (err %d)\n", err);
		return 0;
	}

	printk("Bluetooth initialized\n");

	err = bt_le_scan_start(&scan_param, scan_cb);
	if (err)
	{
		printk("Starting scanning failed (err %d)\n", err);
		return 0;
	}
	do
	{
		k_sleep(K_MSEC(500));
		/* Start advertising */
		err = bt_le_adv_start(BT_LE_ADV_NCONN, ad, ARRAY_SIZE(ad),
							  NULL, 0);
		if (err)
		{
			printk("Advertising failed to start (err %d)\n", err);
			return 0;
		}

		k_sleep(K_MSEC(500));

		err = bt_le_adv_stop();
		if (err)
		{
			printk("Advertising failed to stop (err %d)\n", err);
			return 0;
		}

		mfg_data[6]--;
		if (mfg_data[6] == 0)
		{
			mfg_data[6] = 9;
		}
		// printk("count = %d\n", mfg_data[6]);
	} while (1);
	return 0;
}