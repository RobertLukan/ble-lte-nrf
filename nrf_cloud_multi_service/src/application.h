/* Copyright (c) 2023 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */
#include <zephyr/bluetooth/bluetooth.h>

#ifndef _APPLICATION_H_
#define _APPLICATION_H_

#if defined(CONFIG_NRF_CLOUD_MQTT)
#define MSG_OBJ_DEFINE(_obj_name) \
	NRF_CLOUD_OBJ_JSON_DEFINE(_obj_name);
#elif defined(CONFIG_NRF_CLOUD_COAP)
#define MSG_OBJ_DEFINE(_obj_name) \
	NRF_CLOUD_OBJ_COAP_CBOR_DEFINE(_obj_name);
#endif


struct TextMessage
  {
    //   char request_Type[100];
       unsigned int SenderId;                       /* unique client identifier */
       unsigned int RecipientId;                       /* unique client identifier */
	   double temperatureLocal;
	   unsigned int battLocal;                       /* unique client identifier */
   
       char message[42];                           /* text message */
	//   uint8_t m_hash[10];
  };


struct subscriber_data {
	uint8_t conn_count;
	uint8_t peer_count;
};

struct subscribed_peer {
	bt_addr_le_t addr;
	uint8_t protocol_version;
	bool has_illuminance;
	uint8_t counter;
	uint16_t battery_voltage;
	float temp_celsius;
	float humidity_percent;
	float moisture_percent;
	float illuminance;
	float rssi_value;
	bool has_timestamp;
	int64_t timestamp;
	float spare1;      
	float spare2;      
};


/**
 * @brief Main application -- Wait for valid connection, start location tracking, and then
 * periodically sample sensors and send them to nRF Cloud.
 */
void main_application_thread_fn(void);

#endif /* _APPLICATION_H_ */
