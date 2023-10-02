/*
 * Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/sys/util.h>
#include "d_UART_helper.c"
#include "d_BT_helper.c"

K_SEM_DEFINE(acking, 0, 1);
uint8_t dev_data[] = {'D', 'I', 'S', '0', '1', 'x', 9};

static uint8_t conn_dev[] = {'R', 'E', 'M', '0', '1', 'x'};

uint8_t S_CONN_DEV = sizeof(conn_dev);

struct bt_data adv_data[] = {
	BT_DATA(BT_DATA_MANUFACTURER_DATA, dev_data, 7)};
volatile uint8_t count;

int main(void)
{
	uart_init();
	bluetooth_init();
	start_scan();
	while (1)
	{

		//k_sem_take(&acking, K_FOREVER);
		//stop_adv();
		//dev_data[6] = count;
		//start_adv(&adv_data);
		k_sleep(K_MSEC(100));
	}

	return 0;
}

void scan_cb(const bt_addr_le_t *addr, int8_t rssi, uint8_t adv_type,
			 struct net_buf_simple *buf)
{

	if (k_sem_count_get(&acking) != 0)
	{
		return;
	}
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

		if (buf->data[i] != count)
		{
			printk("(REMOTE):");
			printk(" dev = %s  ", device);
			printk(" count = %d\n", buf->data[i]);
			count = buf->data[i];
			k_sem_give(&acking);
		}
	}
	else
	{
		return;
	}
	// ACK reciept of count to REM device
}