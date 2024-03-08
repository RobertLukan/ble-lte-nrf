/* Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef _APPLICATION_H_
#define _APPLICATION_H_

/**
 * @brief Main application -- Wait for valid connection, start location tracking, and then
 * periodically sample sensors and send them to nRF Cloud.
 */
#include <zephyr/bluetooth/bluetooth.h>

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

//struct subscribed_peer subscribed_peers;


void main_application_thread_fn(void);



#endif /* _APPLICATION_H_ */
