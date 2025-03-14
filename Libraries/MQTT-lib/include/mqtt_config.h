/*************************************************************************
 *
 * Copyright 2023 ETH Zurich and University of Bologna
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * SPDX-License-Identifier: Apache-2.0
 * Author: Giovanni Bambini (gv.bambini@gmail.com)
 *
 **************************************************************************/

#ifndef _MQTT_CONFIG_
#define _MQTT_CONFIG_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

#include "mosquitto.h"

#define MQTT_KEEPALIVE 60
#define MQTT_QOS 0 // Quality of Service

#define TOPIC_STRING_DIM 64
#define MEASSAGE_OF_NUMBER 10
#define MAX_DATA_LENGTH 72 * 4

#define WAIT_SEC_CALLBACK 10

// typedef int mqtt_message_type;

/**
 * Struct collector_val - Struct containing the state of the metric
 * @mqtt_topic:           MQTT topic to subscribe
 * @mosq:                 (INTERNAL USE) ptr to the mosquitto object
 * @flag_en:              (INTERNAL USE) MQTT Broker port
 * @sum_val:              (INTERNAL USE) Sum monitored samples
 * @count_mean:           (INTERNAL USE) Number of monitored samples
 * @mean_val:             Mean of the monitored samples
 * @start:                Timestamp start
 * @end:                  Timestamp end
 *
 * Returns:           0 on Success, 1 on Failure.
 **/
struct mqtt_instance {
    struct mosquitto *mosq;
    bool flag_en;
    struct timeval start;
    struct timeval end;
    char *broker_ip;
    int broker_port;
    char *username;
    char *passwd;

    char *mqtt_topic;

    // int connected;
    int message_received;
    void *message;
    // void* of_message;
    // int topics_number;
    // int topics_counter;
};

struct mqtt_collector_message {
    uint32_t data[MAX_DATA_LENGTH];
    char topic[TOPIC_STRING_DIM];
    int mid;
    int length;
    void *lock;
    void *signal;
};

struct mqtt_publisher_message {
    int length;
    void *address;
    char *topic;
    int mid;
    // unsigned int cycleID;
};

/*
struct mosquitto_message{
        int mid;
        char *topic;
        void *payload;
        int payloadlen;
        int qos;
        bool retain;
};
*/
/*
enum mosq_err_t {
        MOSQ_ERR_AUTH_CONTINUE = -4,
        MOSQ_ERR_NO_SUBSCRIBERS = -3,
        MOSQ_ERR_SUB_EXISTS = -2,
        MOSQ_ERR_CONN_PENDING = -1,
        MOSQ_ERR_SUCCESS = 0,
        MOSQ_ERR_NOMEM = 1,
        MOSQ_ERR_PROTOCOL = 2,
        MOSQ_ERR_INVAL = 3,
        MOSQ_ERR_NO_CONN = 4,
        MOSQ_ERR_CONN_REFUSED = 5,
        MOSQ_ERR_NOT_FOUND = 6,
        MOSQ_ERR_CONN_LOST = 7,
        MOSQ_ERR_TLS = 8,
        MOSQ_ERR_PAYLOAD_SIZE = 9,
        MOSQ_ERR_NOT_SUPPORTED = 10,
        MOSQ_ERR_AUTH = 11,
        MOSQ_ERR_ACL_DENIED = 12,
        MOSQ_ERR_UNKNOWN = 13,
        MOSQ_ERR_ERRNO = 14,
        MOSQ_ERR_EAI = 15,
        MOSQ_ERR_PROXY = 16,
        MOSQ_ERR_PLUGIN_DEFER = 17,
        MOSQ_ERR_MALFORMED_UTF8 = 18,
        MOSQ_ERR_KEEPALIVE = 19,
        MOSQ_ERR_LOOKUP = 20,
        MOSQ_ERR_MALFORMED_PACKET = 21,
        MOSQ_ERR_DUPLICATE_PROPERTY = 22,
        MOSQ_ERR_TLS_HANDSHAKE = 23,
        MOSQ_ERR_QOS_NOT_SUPPORTED = 24,
        MOSQ_ERR_OVERSIZE_PACKET = 25,
        MOSQ_ERR_OCSP = 26,
        MOSQ_ERR_TIMEOUT = 27,
        MOSQ_ERR_RETAIN_NOT_SUPPORTED = 28,
        MOSQ_ERR_TOPIC_ALIAS_INVALID = 29,
        MOSQ_ERR_ADMINISTRATIVE_ACTION = 30,
        MOSQ_ERR_ALREADY_EXISTS = 31,
};
*/

#endif // lib
