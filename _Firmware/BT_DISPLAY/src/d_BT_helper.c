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

static uint8_t mfg_data[] = {'D', 'I', 'S', '0', '1', 'x', 9};
static uint8_t conn_dev[] = {'R', 'E', 'M', '0', '1', 'x'};
uint8_t S_CONN_DEV = sizeof(conn_dev);
static const struct bt_data ad[] = {
	BT_DATA(BT_DATA_MANUFACTURER_DATA, mfg_data, 14),
};

volatile uint8_t count = 0;
bool open_tx = false;

static void scan_cb(const bt_addr_le_t *addr, int8_t rssi, uint8_t adv_type,
					struct net_buf_simple *buf)
{
	bool read_dev = false;
	int i = 0;
	//printk("Scan\n");
	// check for REM device
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
			// printk("read_dev = false\n");
			read_dev = false;
			// device is not a REM
			return;
		}
	}

	// device is a REM
	if (read_dev)
	{
		// display count
		count = buf->data[i];
		printk("(DISPLAY):");
		printk(" dev = %s  ", device);
		printk(" count = %d\n", mfg_data[6]);
		mfg_data[6] = count;
		open_tx = true;
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
		.interval = 0x0200,
		.window = 0x00f0,
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

	while (1)
	{
		//printk("Waiting...\n");
		k_sleep(K_MSEC(1000));
		if (open_tx)
		{
			printk("ACK REM\n");
			bool err = bt_le_adv_start(BT_LE_ADV_NCONN, ad, ARRAY_SIZE(ad),
									   NULL, 0);
			if (err)
			{
				printk("Advertising failed to start (err %d)\n", err);
				return 0;
			}

			k_sleep(K_MSEC(100));

			err = bt_le_adv_stop();
			if (err)
			{
				printk("Advertising failed to stop (err %d)\n", err);
				return 0;
			}
			open_tx = false;
		}
	}

	return 0;
}