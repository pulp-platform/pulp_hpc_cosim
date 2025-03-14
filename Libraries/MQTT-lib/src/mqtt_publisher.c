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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <math.h>
#include <inttypes.h>
#include <unistd.h>
#include <sched.h>

#include "mosquitto.h"
#include "mqtt_config.h"
#include "mqtt_publisher.h"

#include "mqtt_transl_layer.h"

static void connect_callback(struct mosquitto *, void *, int);
static void message_callback(struct mosquitto *, void *, const struct mosquitto_message *);
static void publish_callback(struct mosquitto *, void *, int);

int publisher_init(struct mqtt_instance *val) {
    val->mosq = NULL;
    val->flag_en = 0;
    val->start.tv_sec = 0;
    val->start.tv_usec = 0;
    val->end.tv_sec = 0;
    val->end.tv_usec = 0;
    val->message_received = 0;

    if ((val->broker_ip == NULL) || !(strcmp(val->broker_ip, ""))) {
        if ((MQTT_BROKER_IP != NULL) && !(strcmp(MQTT_BROKER_IP, ""))) {
            strcpy(val->broker_ip, MQTT_BROKER_IP);
        } else {
            strcpy(val->broker_ip, "127.0.0.1");
        }
        printf("[Publisher] Broker IP not selected! %s will be used instead\n\r", val->broker_ip);
    }
    if (val->broker_port < 1) {
        val->broker_port = 1883;
        printf("[Publisher] Broker Port not selected! %d will be used instead\n\r",
               val->broker_port);
    }

    mosquitto_lib_init();
    printf("[Publisher]: Mosquitto lib inited.\n\r");
    val->mosq = mosquitto_new(NULL, true, val);
    if (!val->mosq) {
        printf("[Publisher]: Init error.\n\r");
        return 1;
    } else
        printf("[Publisher]: Init done.\n\r");

    mosquitto_connect_callback_set(
        val->mosq, connect_callback); // This is called when the broker sends a CONNACK message in
                                      // response to a connection.
    // mosquitto_message_callback_set(val->mosq, message_callback); //This is called when a message
    // is received from the broker.
    mosquitto_publish_callback_set(
        val->mosq, publish_callback); // This is called when a message initiated with
                                      // mosquitto_publish has been sent to the broker successfully.

    mosquitto_username_pw_set(val->mosq, val->username, val->passwd);

    while (mosquitto_connect(val->mosq, val->broker_ip, val->broker_port, MQTT_KEEPALIVE) !=
           MOSQ_ERR_SUCCESS) {
        printf("[Publisher]: Could not connect to broker\n\r");
        printf("[Publisher]: Retry in 30 seconds...\n\r");
        sleep(10);
        // exit(EXIT_FAILURE);
    }

    return 0;
}

int publisher_start(struct mqtt_instance *val) {
    if (mosquitto_loop_start(val->mosq)) {
        printf("[Publisher]: Unable to create Thread.");
        return 1;
    }

    int counter = 0;
    while (!val->flag_en && counter < WAIT_SEC_CALLBACK) {
        counter++;
        sleep(1);
    }
    if (counter >= WAIT_SEC_CALLBACK) {
        printf("[Publisher]: Error! Not connected\n\r");
        return 1;
    } else
        printf("[Publisher]: Connected to the broker.\n\r");

    gettimeofday(&(val->start), NULL);

    return 0;
}

int publisher_send(struct mqtt_instance *val, struct mqtt_publisher_message *message) {
    int rc = mosquitto_publish(val->mosq, &message->mid, message->topic, message->length,
                               message->address, MQTT_QOS, false);
    if (rc != MOSQ_ERR_SUCCESS) {
        printf("[Publisher]: Warning: cannot send message. Errorcode: %d \n\r", rc);
    }
    // reteined messages: https://www.hivemq.com/blog/mqtt-essentials-part-8-retained-messages/

    return 0;
}

static void connect_callback(struct mosquitto *mosq, void *obj, int rc) {
    struct mqtt_instance *tmp_val = obj;
    if (!rc) {
        /*
         * Subscribe to broker information topics on successful connect.
         */
        // There is no need to configure a topic, publishing on it is enough
        // if(mosquitto_subscribe(mosq, NULL, (void*)(tmp_val->mqtt_topic), MQTT_QOS)){
        //   printf("[Publisher]: Error: unable to subscribe.");
        printf("[Publisher]: Connected\n");
        tmp_val->flag_en = 1;
    } else {
        tmp_val->flag_en = 0;
        printf("[Publisher]: Connect failed, error: %d\n", rc);
    }

    // return;
}

static void message_callback(struct mosquitto *mosq, void *obj,
                             const struct mosquitto_message *message) {
    // nothing.
}

static void publish_callback(struct mosquitto *mosq, void *obj, int mid) {
    // printf("\n\r[Publisher]: Message %d received by the broker.\n\r", mid);

    struct mqtt_instance *tmp_val = obj;
    tmp_val->message_received++;
    MQTT_SIGNAL_PUBLISH();
}
