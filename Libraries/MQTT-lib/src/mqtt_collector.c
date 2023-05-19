
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
#include "mqtt_collector.h"

#include "mqtt_transl_layer.h"



static void connect_callback(struct mosquitto*, void*, int);
static void message_callback(struct mosquitto*, void*, const struct mosquitto_message*);
static void publish_callback(struct mosquitto*, void*, int);

int collector_init(struct mqtt_instance *val)
{
	val->mosq = NULL;
	val->flag_en = 0;
	val->start.tv_sec = 0;
	val->start.tv_usec = 0;
	val->end.tv_sec = 0;
	val->end.tv_usec = 0;
	val->message_received = 0;

	if ( (val->broker_ip == NULL) || !(strcmp(val->broker_ip,"")) )
	{
		if ( (MQTT_BROKER_IP != NULL) && !(strcmp(MQTT_BROKER_IP, "")) )
		{
			strcpy(val->broker_ip, MQTT_BROKER_IP);
		}
		else
		{
			strcpy(val->broker_ip, "127.0.0.1");
		}
		printf("[Collector] Broker IP not selected! %s will be used instead\n\r", val->broker_ip);
	}
	if (val->broker_port < 1)
	{
		val->broker_port = 1883;
		printf("[Collector] Broker Port not selected! %d will be used instead\n\r", val->broker_port);
	}

	struct mqtt_collector_message *mqtt_message = val->message;
	MQTT_MESSAGE_SIGNAL_INIT(mqtt_message->signal);
	MQTT_LOCK_MESSAGE_INIT(mqtt_message->lock);
	mosquitto_lib_init();
	val->mosq = mosquitto_new(NULL, true, val);
	if(!val->mosq){
		printf("[Collector]: Init error.\n");
		return 1;
	}

	mosquitto_connect_callback_set(val->mosq, connect_callback);
	mosquitto_message_callback_set(val->mosq, message_callback); //This is called when a message is received from the broker.

	mosquitto_username_pw_set(val->mosq, val->username, val->passwd);

	if(mosquitto_connect(val->mosq, val->broker_ip, val->broker_port, MQTT_KEEPALIVE)){
		printf("[Collector]: Unable to connect to the broker.\n");
		return 1;
	}


	return 0;
}


int collector_start(struct mqtt_instance *val)
{
	if (mosquitto_loop_start(val->mosq)){
		printf("[Collector]: Unable to create Thread.");
		return 1;
	}

	int counter = 0;
	while(!val->flag_en && counter < WAIT_SEC_CALLBACK){
		counter++;
		sleep(1);
	}
	if (counter >= WAIT_SEC_CALLBACK)
	{
		printf("[Collector]: Error! Not connected\n\r");
		return 1;
	}
	else
		printf("[Collector]: Connected to the broker.\n\r");

	gettimeofday(&(val->start), NULL);

	return 0;
}


int collector_stop(struct mqtt_instance *val)
{
   if(!val->mosq){
      return 1;
   }

   if(mosquitto_loop_stop(val->mosq, true)){
      printf("[Collector]: Error closing Thread.");
      return 1;
   }

   val->flag_en = 0;
   gettimeofday(&(val->end), NULL);

   mosquitto_destroy(val->mosq);
   mosquitto_lib_cleanup();
   struct mqtt_collector_message *mqtt_message = val->message;
   MQTT_MESSAGE_SIGNAL_DEINIT(mqtt_message->signal);
   MQTT_LOCK_MESSAGE_DEINIT(mqtt_message->lock);

   return 0;
}


static void connect_callback(struct mosquitto *mosq, void *obj, int rc)
{
	struct mqtt_instance *tmp_val = obj;
	if(!rc){
		/*
		* Subscribe to broker information topics on successful connect.
		*/
		tmp_val->flag_en = 1;
		//printf("[Collector]: Connected to broker\n");
		if(mosquitto_subscribe(mosq, NULL, (void*)(tmp_val->mqtt_topic), MQTT_QOS)){
			printf("[Collector]: Error: unable to subscribe.");
		}
	}else{
		tmp_val->flag_en = 0;
		printf("[Collector]: Connect failed\n");
	}
}

static void message_callback(struct mosquitto *mosq, void *obj,
								const struct mosquitto_message *message)
{
	struct mqtt_instance *tmp_val = obj;
	struct mqtt_collector_message *mqtt_message = tmp_val->message;
	//struct mqtt_collector_message *mqtt_message_of = tmp_val->of_message;
	if(tmp_val->flag_en){
		if(message->payloadlen){

			transl_layer_copy_message(message);

			MQTT_SIGNAL_MESSAGE(mqtt_message->signal);

		} //message->payloadlen
		else
		{
			printf("[Collector]: Error: empty message %d \n", message->mid);
			//tmp_val->message_received++;
		}
	} //tmp_val->flag_en
	else
	{
		printf("[Collector]: Error: Collector not active, message %d \n", message->mid);
		//tmp_val->message_received++;
	}
}
