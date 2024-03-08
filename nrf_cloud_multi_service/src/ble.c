/*
 * Copyright (c) 2018 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/kernel.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/uuid.h>
#include <bluetooth/gatt_dm.h>
#include <bluetooth/scan.h>

#include <dk_buttons_and_leds.h>
#include <zephyr/sys/byteorder.h>

#include <net/nrf_cloud.h>
#include <zephyr/logging/log.h>
#include "aggregator.h"
#include "application.h"


#include <date_time.h>


extern struct subscribed_peer subscribed_peers[CONFIG_BT_MAX_PAIRED];

bt_addr_le_t set_addr = {
	.type = 0,
	.a =  { {0xE9, 0xD6, 0xAF, 0x57, 0xD4, 0xEC} } ,
};

bt_addr_le_t addr = {
		.type		= BT_ADDR_LE_RANDOM,
		.a.val		= {0xba, 0xde, 0xba, 0x11, 0xca, 0xfe},
	};

LOG_MODULE_REGISTER(lte_ble_gw, CONFIG_MQTT_MULTI_SERVICE_LOG_LEVEL);



/* Thinghy advertisement UUID */
#define BT_UUID_THINGY_VAL \
	BT_UUID_128_ENCODE(0xef680100, 0x9b35, 0x4933, 0x9b10, 0x52ffa9740042)

#define BT_UUID_THINGY \
	BT_UUID_DECLARE_128(BT_UUID_THINGY_VAL)
/* Thingy service UUID */
#define BT_UUID_TMS_VAL \
	BT_UUID_128_ENCODE(0xef680400, 0x9b35, 0x4933, 0x9b10, 0x52ffa9740042)

#define BT_UUID_TMS \
	BT_UUID_DECLARE_128(BT_UUID_TMS_VAL)
		/* Thingy characteristic UUID */
#define BT_UUID_TOC_VAL \
	BT_UUID_128_ENCODE(0xef680403, 0x9b35, 0x4933, 0x9b10, 0x52ffa9740042)

#define BT_UUID_TOC \
	BT_UUID_DECLARE_128(BT_UUID_TOC_VAL)
extern void alarm(void);

static uint8_t on_received(struct bt_conn *conn,
			struct bt_gatt_subscribe_params *params,
			const void *data, uint16_t length)
			{
				return BT_GATT_ITER_CONTINUE;
			}

static void reset_subscribers(void)
{
	for (size_t i = 1; i < 20; i++) {
		bt_addr_le_copy(&subscribed_peers[i].addr, BT_ADDR_LE_NONE);
	}

}
static void discovery_completed(struct bt_gatt_dm *disc, void *ctx)
{
	int err;

	static struct bt_gatt_subscribe_params param = {
		.notify = on_received,
		.value = BT_GATT_CCC_NOTIFY,
	};

	const struct bt_gatt_dm_attr *chrc;
	const struct bt_gatt_dm_attr *desc;

	chrc = bt_gatt_dm_char_by_uuid(disc, BT_UUID_TOC);
	if (!chrc) {
		LOG_ERR("Missing Thingy orientation characteristic");
		goto release;
	}

	desc = bt_gatt_dm_desc_by_uuid(disc, chrc, BT_UUID_TOC);
	if (!desc) {
		LOG_ERR("Missing Thingy orientation char value descriptor");
		goto release;
	}

	param.value_handle = desc->handle,

	desc = bt_gatt_dm_desc_by_uuid(disc, chrc, BT_UUID_GATT_CCC);
	if (!desc) {
		LOG_ERR("Missing Thingy orientation char CCC descriptor");
		goto release;
	}

	param.ccc_handle = desc->handle;

	err = bt_gatt_subscribe(bt_gatt_dm_conn_get(disc), &param);
	if (err) {
		LOG_ERR("Subscribe failed (err %d)", err);
	}

release:
	err = bt_gatt_dm_data_release(disc);
	if (err) {
		LOG_ERR("Could not release discovery data, err: %d", err);
	}
}

static void discovery_service_not_found(struct bt_conn *conn, void *ctx)
{
	LOG_ERR("Thingy orientation service not found!");
}

static void discovery_error_found(struct bt_conn *conn, int err, void *ctx)
{
	LOG_ERR("The discovery procedure failed, err %d", err);
}

static struct bt_gatt_dm_cb discovery_cb = {
	.completed = discovery_completed,
	.service_not_found = discovery_service_not_found,
	.error_found = discovery_error_found,
};

static void connected(struct bt_conn *conn, uint8_t conn_err)
{
	int err;
	char addr[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

	if (conn_err) {
		LOG_ERR("Failed to connect to %s (%u)", addr, conn_err);
		return;
	}

	LOG_INF("Connected: %s", addr);

	err = bt_gatt_dm_start(conn, BT_UUID_TMS, &discovery_cb, NULL);
	if (err) {
		LOG_ERR("Could not start service discovery, err %d", err);
	}
}

static struct bt_conn_cb conn_callbacks = {
	.connected = connected,
};

bool find_adv_data(struct bt_data *data, int *user_data)
{
    if(data->type == 22)
	{

		int  subscriber  = (int)user_data;
		int oldCounter = subscribed_peers[subscriber].counter;
  		const uint8_t protocol_version = data->data[2] >> 4;
		subscribed_peers[subscriber].protocol_version = protocol_version;


  		uint8_t counter = data->data[3] & 0x0f;
		if(oldCounter==counter)
		{
			return true;
		}

		LOG_INF("Counter: %u", counter);
		subscribed_peers[subscriber].counter = counter;


// Some b-parasite versions have an (optional) illuminance sensor.
  		bool has_illuminance = data->data[2] & 0x1;
		subscribed_peers[subscriber].has_illuminance = has_illuminance;

		LOG_INF("Illuminance support: %u", has_illuminance);
  // Counter for deduplicating messages.

	// Battery voltage in millivolts.
  		uint16_t battery_millivolt = data->data[4] << 8 | data->data[5];
  		float battery_voltage = battery_millivolt / 1000.0f;
		LOG_INF("Battery voltage: %.2f", battery_voltage);
		subscribed_peers[subscriber].battery_voltage = battery_millivolt;

// Temperature in 1000 * Celsius (protocol v1) or 100 * Celsius (protocol v2).
  		float temp_celsius;
	//	uint16_t temp_millicelsius;
  		if (protocol_version == 1) {
    		uint16_t temp_millicelsius = data->data[6] << 8 | data->data[7];
    		temp_celsius = temp_millicelsius / 1000.0f;
  		} else {
    		int16_t temp_centicelsius = data->data[6] << 8 | data->data[7];
    		temp_celsius = temp_centicelsius / 100.0f;
  		}
		LOG_INF("Temp: %.2f", temp_celsius);
		subscribed_peers[subscriber].temp_celsius = temp_celsius;

// Relative air humidity in the range [0, 2^16).
  		uint16_t humidity = data->data[8] << 8 | data->data[9];
  		float humidity_percent = (100.0f * humidity) / (1 << 16);
		LOG_INF("Humidity: %.2f", humidity_percent);
		subscribed_peers[subscriber].humidity_percent = humidity_percent;

  // Relative soil moisture in [0 - 2^16).
  		uint16_t soil_moisture = data->data[10] << 8 | data->data[11];
  		float moisture_percent = (100.0f * soil_moisture) / (1 << 16);
		LOG_INF("Moisture: %.2f", moisture_percent);
		subscribed_peers[subscriber].moisture_percent = moisture_percent;

  // Ambient light in lux.
  		float illuminance = has_illuminance ? data->data[18] << 8 | data->data[19] : 0.0f;
		LOG_INF("Illuminance: %.2f", illuminance);
		subscribed_peers[subscriber].illuminance = illuminance;


		if(date_time_is_valid())
		{
			int64_t timestamp;
			date_time_now(&timestamp);
			LOG_INF("DateTime valid");
			subscribed_peers[subscriber].has_timestamp = true;
			subscribed_peers[subscriber].timestamp = timestamp;
		}
	
	}
    return true; // keep parsing
}


void scan_filter_match(struct bt_scan_device_info *device_info,
		       struct bt_scan_filter_match *filter_match,
		       bool connectable)
{
	char addr[BT_ADDR_LE_STR_LEN];

	bt_addr_le_to_str(device_info->recv_info->addr, addr, sizeof(addr));
	int counter = 0;
	LOG_INF("Device found: %s", addr);

	for (int i = 0; i < 20; i++) {
			/* Already saved. */
			if (!bt_addr_le_cmp(&subscribed_peers[i].addr,device_info->recv_info->addr)) 
				  {
						counter = i;
						subscribed_peers[counter].rssi_value = device_info->recv_info->rssi;
					//	LOG_INF("Device already subscribed: %s", addr);
						break;
			}
			/* Save data about new subscriber. */
			if (!bt_addr_le_cmp(&subscribed_peers[i].addr,BT_ADDR_LE_NONE)) 
			{
				bt_addr_le_copy(&subscribed_peers[i].addr,device_info->recv_info->addr);

			//	LOG_INF("Device not yet subscribed: %s", addr);
				counter = i;
				subscribed_peers[counter].rssi_value = device_info->recv_info->rssi;
				break;
			}
		}


    bt_data_parse(device_info->adv_data, find_adv_data, counter);

}


void scan_connecting_error(struct bt_scan_device_info *device_info)
{
	LOG_ERR("Connection to peer failed!");
}

BT_SCAN_CB_INIT(scan_cb, scan_filter_match, NULL, scan_connecting_error, NULL);

static void scan_start(void)
{
	int err;
	reset_subscribers();

	struct bt_le_scan_param scan_param = {
		.type = BT_LE_SCAN_TYPE_PASSIVE,
//		.options = BT_LE_SCAN_OPT_FILTER_ACCEPT_LIST, 
//		.options = BT_LE_SCAN_OPT_FILTER_DUPLICATE,
		.interval = 0x0010,
		.window = 0x0010,
		.timeout = 0,
	};

	struct bt_scan_init_param scan_init = {
		.connect_if_match = 0,
		.scan_param = &scan_param,
		.conn_param = BT_LE_CONN_PARAM_DEFAULT,
	};

	bt_scan_init(&scan_init);
	bt_scan_cb_register(&scan_cb);
	bt_scan_filter_remove_all();



    struct bt_scan_short_name ble_shrt_name;
	ble_shrt_name.name = "prst";
	ble_shrt_name.min_len = 4;

	err = bt_scan_filter_add(BT_SCAN_FILTER_TYPE_SHORT_NAME, &ble_shrt_name);
	if (err) {
		LOG_ERR("Scanning filters cannot be set (err %d)", err);
		return;
	}
	err = bt_scan_filter_enable(BT_SCAN_SHORT_NAME_FILTER, false);
	if (err) {
		LOG_ERR("Filters cannot be turned on");
	}

	err = bt_scan_start(BT_SCAN_TYPE_SCAN_ACTIVE);
	if (err) {
		LOG_ERR("Scanning failed to start, err %d", err);
	}

	LOG_INF("Scanning...");
}

static void ble_ready(int err)
{
	LOG_INF("Bluetooth ready");

	scan_start();
}

void ble_init()
{
	int err;
	LOG_INF("Initializing Bluetooth..");
	err = bt_enable(ble_ready);
	if (err) {
		LOG_ERR("Bluetooth init failed (err %d)", err);
		return;
	}
}
