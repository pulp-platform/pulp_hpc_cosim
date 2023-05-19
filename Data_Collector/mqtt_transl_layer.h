
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




#ifndef _MQTT_TRANSL_LAYER_
#define _MQTT_TRANSL_LAYER_

#include "mySem.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>

#define DATA_PRINTED 12000
#define NUMBER_OF_THREADS 1
#define NUMBER_OF_TOPICS 12

//TODO: cancne
#define WL_STATES 5

char* mqtt_topics[NUMBER_OF_TOPICS];
#ifdef USE_FILE_DB
FILE* fp_t[NUMBER_OF_TOPICS];
#endif

#define MQTT_SIGN_DIM 1
#define MQTT_LENGTH_POS 1

//Synchro Stuff
mySem_t sem_mqtt_message_received;
mySem_t sem_mqtt_publish_done[NUMBER_OF_THREADS];
pthread_mutex_t message_lock[NUMBER_OF_THREADS];


//examon
#ifdef USE_EXAMON
#define EXAMON_MAX_TOPIC_CHAR 256
#define EXAMON_TIME_MUL 200 //this is: average_examon*sim_hw_multiplier*sim_multiplier/5
#include "mqtt_publisher.h"
//#include "cmdconf.h"
#include <sys/time.h>
#include <time.h>
struct mqtt_instance mqtt_examon_pub;
struct mqtt_publisher_message mqtt_examon_message;
time_t epoch_time;
#endif



//#define MQTT_BROKER_IP "137.204.213.167" //"137.204.213.228" //"127.0.0.1" //"137.204.213.192" /228
#define MQTT_PORT 1883 // Default MQTT port

#define MQTT_MESSAGE_SIGNAL_INIT(MESS_SIGNAL) {}//{mySem_t* val_sem_ = (mySem_t*)MESS_SIGNAL; mySem_init(val_sem_, 0, 0, mySem_Binary);}
#define MQTT_MESSAGE_SIGNAL_DEINIT(MESS_SIGNAL) {}//{mySem_t* val_sem_ = (mySem_t*)MESS_SIGNAL; mySem_destroy(val_sem_);}
#define MQTT_PUBLISH_SIGNAL_INIT() //mySem_init(&sem_mqtt_publish_done, 0, 0, mySem_Binary)
#define MQTT_PUBLISH_SIGNAL_DEINIT() //mySem_destroy(&sem_mqtt_publish_done)

#define MQTT_SIGNAL_MESSAGE(MESS_SIGNAL) {}//{mySem_t* val_sem_ = (mySem_t*)MESS_SIGNAL; mySem_post(val_sem_);}
#define MQTT_SIGNAL_PUBLISH() //mySem_post(&sem_mqtt_publish_done)

#define MQTT_AWAKE_ON_MESSAGE(MESS_SIGNAL) {}//{mySem_t* val_sem_ = (mySem_t*)MESS_SIGNAL; mySem_wait(val_sem_);}
#define MQTT_AWAKE_ON_PUBLISH() //mySem_wait(&sem_mqtt_publish_done)

#define MQTT_LOCK_MESSAGE_INIT(LOCK) {int rc;pthread_mutexattr_t lock_attr;\
                rc = pthread_mutexattr_init(&lock_attr);\
                if (rc) {printf("Error attr for Mutex Init: %d\n\r", rc);}\
                rc = pthread_mutexattr_settype(&lock_attr, PTHREAD_MUTEX_ERRORCHECK);\
                if (rc) {printf("Error attr for Mutex Init: %d\n\r", rc);}\
                rc = pthread_mutexattr_setprotocol(&lock_attr, PTHREAD_PRIO_INHERIT);\
                if (rc) {printf("Error attr for Mutex Init: %d\n\r", rc);}\
                pthread_mutex_t* val_lock_ = (pthread_mutex_t*)LOCK;\
                rc = pthread_mutex_init( val_lock_, &lock_attr );\
            	if( rc ) {printf("pthread_mutex_init failed - %d\n\r", rc); exit(1);} }
#define MQTT_LOCK_MESSAGE_DEINIT(LOCK) { int rc;\
                pthread_mutex_t* val_lock_ = (pthread_mutex_t*)LOCK;\
                rc = pthread_mutex_destroy( val_lock_ );\
                if(rc) {printf("pthread_mutex_destroy failed - %d\n\r", rc); exit(1);} }
#define MQTT_LOCK_MESSAGE_TAKE(LOCK) {int rc;\
                pthread_mutex_t* val_lock_ = (pthread_mutex_t*)LOCK;\
                rc = pthread_mutex_lock( val_lock_ );\
                if(rc) {printf("pthread_mutex_lock failed (message lock) - %d\n\r", rc); exit(1);} }
#define MQTT_LOCK_MESSAGE_RELEASE(LOCK) {int rc;\
                pthread_mutex_t* val_lock_ = (pthread_mutex_t*)LOCK;\
                rc = pthread_mutex_unlock( val_lock_ );\
                if(rc) {printf("pthread_mutex_unlock failed (message lock) - %d\n\r", rc); exit(1);} }

static inline void transl_layer_copy_message(const struct mosquitto_message *message) {

	/*
	* Parse the payload string to catch the measured value
	*/
	//char *subString = strtok(message->payload,";");
	//tmp_val->sum_val += atoi(subString);
	//tmp_val->count_mean++;
	int length = message->payloadlen/4;
	if (length > MAX_DATA_LENGTH)
	{
		length = MAX_DATA_LENGTH;
		printf("[Collector]: Error: Payloadlen exceeding max dimension %d.\n", MAX_DATA_LENGTH);
	}

	//if (!tmp_val->message_received)
	//{
	//MQTT_LOCK_MESSAGE_TAKE(mqtt_message->lock);
	//ID:
	uint32_t *id_addr = (uint32_t*)message->payload;
	uint32_t id = id_addr[length-MQTT_LENGTH_POS];
	//strcpy(mqtt_message->topic, message->topic);
	FILE *fp = NULL;
	int vartype = -1; //0->uint32_t, 1->float
	#ifdef USE_EXAMON
	int board = -1;
	int chip = -1;
	char examon_base_topic[EXAMON_MAX_TOPIC_CHAR] = "/node/hostname/plugin/pulpcontroller_pub/chnl";
	char examon_topic[EXAMON_MAX_TOPIC_CHAR] = "";
	char examon_cmd[16] = "";
	char examon_position[16] = "";
	char examon_final_name[32] = "";
	#endif

	if (!strcmp(message->topic, "pms/board0/chip0/data/otp/domain_pw")) {
		#ifdef USE_FILE_DB
		fp = fp_t[0];
		#endif
		vartype = 2;
		#ifdef USE_EXAMON
		board = 0;
		chip = 0;
		strcpy(examon_cmd, "/data");
		strcpy(examon_position, "/domain/");
		strcpy(examon_final_name, "/measured_pw");
		#endif
	}else if (!strcmp(message->topic, "pms/board0/chip0/data/otp/temp")) {
		#ifdef USE_FILE_DB
		fp = fp_t[1];
		#endif
		vartype = 2;
		#ifdef USE_EXAMON
		board = 0;
		chip = 0;
		strcpy(examon_cmd, "/data");
		strcpy(examon_position, "/core/");
		strcpy(examon_final_name, "/temperature");
		#endif
	}else if (!strcmp(message->topic, "pms/board0/chip0/data/otp/instr")) {
		#ifdef USE_FILE_DB
		fp = fp_t[2];
		#endif
		vartype = 3;
		#ifdef USE_EXAMON
		board = 0;
		chip = 0;
		strcpy(examon_cmd, "/data");
		strcpy(examon_position, "/core/");
		strcpy(examon_final_name, "/instructions");
		#endif
	}else if (!strcmp(message->topic, "pms/board0/chip0/cmd/target_freq")) {
		#ifdef USE_FILE_DB
		fp = fp_t[6];
		#endif
		vartype = 2;
		#ifdef USE_EXAMON
		board = 0;
		chip = 0;
		strcpy(examon_cmd, "/cmd");
		strcpy(examon_position, "/core/");
		strcpy(examon_final_name, "/target_freq");
		#endif
	} else if (!strcmp(message->topic, "pms/board0/chip0/cmd/pw_budget")) {
		#ifdef USE_FILE_DB
		fp = fp_t[7];
		#endif
		vartype = 2;
		#ifdef USE_EXAMON
		board = 0;
		chip = 0;
		strcpy(examon_cmd, "/cmd");
		strcpy(examon_position, "/domain/");
		strcpy(examon_final_name, "/pw_budget");
		#endif
	} else if (!strcmp(message->topic, "pms/board0/chip0/cmd/bindings")) {
		#ifdef USE_FILE_DB
		fp = fp_t[8];
		#endif
		vartype = 1;
		#ifdef USE_EXAMON
		board = 0;
		chip = 0;
		strcpy(examon_cmd, "/cmd");
		strcpy(examon_position, "/core/");
		strcpy(examon_final_name, "/bindings");
		#endif
	} else if (!strcmp(message->topic, "pms/board0/chip0/data/ifp/domain_freq")) {
		#ifdef USE_FILE_DB
		fp = fp_t[3];
		#endif
		vartype = 2;
		#ifdef USE_EXAMON
		board = 0;
		chip = 0;
		strcpy(examon_cmd, "/data");
		strcpy(examon_position, "/domain/");
		strcpy(examon_final_name, "/freq");
		#endif
	} else if (!strcmp(message->topic, "pms/board0/chip0/data/ifp/domain_vdd")) {
		#ifdef USE_FILE_DB
		fp = fp_t[4];
		#endif
		vartype = 2;
		#ifdef USE_EXAMON
		board = 0;
		chip = 0;
		strcpy(examon_cmd, "/data");
		strcpy(examon_position, "/domain/");
		strcpy(examon_final_name, "/vdd");
		#endif
	} else if (!strcmp(message->topic, "pms/board0/chip0/data/ifp/core_freq")) {
		#ifdef USE_FILE_DB
		fp = fp_t[5];
		#endif
		vartype = 2;
		#ifdef USE_EXAMON
		board = 0;
		chip = 0;
		strcpy(examon_cmd, "/data");
		strcpy(examon_position, "/core/");
		strcpy(examon_final_name, "/freq");
		#endif
	} else if (!strcmp(message->topic, "pms/board0/chip0/data/ifp/core_alpha")) {
		#ifdef USE_FILE_DB
		fp = fp_t[9];
		#endif
		vartype = 2;
		#ifdef USE_EXAMON
		board = 0;
		chip = 0;
		strcpy(examon_cmd, "/data");
		strcpy(examon_position, "/core/");
		strcpy(examon_final_name, "/alpha");
		#endif
	} else if (!strcmp(message->topic, "pms/board0/chip0/data/ifp/core_redpw")) {
		#ifdef USE_FILE_DB
		fp = fp_t[10];
		#endif
		vartype = 2;
		#ifdef USE_EXAMON
		board = 0;
		chip = 0;
		strcpy(examon_cmd, "/data");
		strcpy(examon_position, "/core/");
		strcpy(examon_final_name, "/redpw");
		#endif
	} else if (!strcmp(message->topic, "pms/board0/chip0/data/ifp/core_freqredmap")) {
		#ifdef USE_FILE_DB
		fp = fp_t[11];
		#endif
		vartype = 1;
		#ifdef USE_EXAMON
		board = 0;
		chip = 0;
		strcpy(examon_cmd, "/data");
		strcpy(examon_position, "/core/");
		strcpy(examon_final_name, "/freqredmap");
		#endif
	} else {
		printf("[Collector] Error! Topic not found! %s\n", message->topic);
	}

	#ifdef USE_FILE_DB
	if (fp != NULL)
	{
    #endif
	
		// prepare examon topic:
		#ifdef USE_EXAMON
		snprintf(examon_topic, EXAMON_MAX_TOPIC_CHAR, "%s%s%s%d%s%d%s", examon_base_topic, examon_cmd, "/board/", board, "/chip/", chip, examon_position);
		char* examon_data = (char *)malloc(64*(length-MQTT_SIGN_DIM)*sizeof(char));
		char* examon_time = (char *)malloc(32*sizeof(char));
		//sprintf(examon_time, "%.6f", (float)epoch_time + ((float)id / 1000000.0f));

		#ifdef USE_EXAMON_SIM_TIME
		uint32_t report_add = (id*EXAMON_TIME_MUL / 1000000);

		float test_num = (float)((id*EXAMON_TIME_MUL)-(report_add*1000000)) / 1000000.0f;
		//printf("%f,   ", test_num);
		if (test_num < 0.001f)
		{
			sprintf(examon_time, "%d.000%d", (epoch_time + report_add),  (int)(id * EXAMON_TIME_MUL - (report_add*1000000) ) );
		}
		else if (test_num < 0.01f)
		{
			sprintf(examon_time, "%d.00%d", (epoch_time + report_add),  (int)(id * EXAMON_TIME_MUL - (report_add*1000000) ) );
		}
		else if (test_num < 0.1f)
		{
			sprintf(examon_time, "%d.0%d", (epoch_time + report_add),  (int)(id * EXAMON_TIME_MUL - (report_add*1000000) ) );
		}
		else
		{
			sprintf(examon_time, "%d.%d", (epoch_time + report_add),  (int)(id * EXAMON_TIME_MUL - (report_add*1000000) ) );
		}
		printf("%s\n\r", examon_time);
		//printf("%d, %d, %d;   ", report_add, id, (id * EXAMON_TIME_MUL - (report_add*1000000) ) );
		//printf("id: %d, time: %s, epoch: %d, division: %f\n\r", id, examon_time, epoch_time, ((float)id / 1000000.0f));
		#else
		struct timeval tv;
		gettimeofday(&tv, NULL);

		sprintf(examon_time, "%.3f", tv.tv_sec + (tv.tv_usec / 1000000.0));

		#endif //USE_EXAMON_SIM_TIME
		#endif

		// print id column
		#ifdef USE_FILE_DB
		if (fprintf(fp, "%d, ", id) < 0)
		{
			perror("[MQTT] Error writing to file\n");
		}
		#endif	
		
		if (vartype == 1)
		{
			uint32_t* message_addr = (uint32_t*)message->payload;

			for (int i=0; i<length-MQTT_SIGN_DIM; i++)
			{
				// print the data
                #ifdef USE_FILE_DB
				fprintf(fp, "%d, ", message_addr[i]);
                #endif

				// send through examon
				#ifdef USE_EXAMON
				// finish topic
				snprintf(mqtt_examon_message.topic, EXAMON_MAX_TOPIC_CHAR, "%s%d%s",examon_topic, i, examon_final_name);
                //strcpy(mqtt_examon_message.topic, examon_topic);
                //printf("topic: %s\n\r", mqtt_examon_message.topic);
                char* examon_data_index = examon_data + i*64*sizeof(char);
                sprintf(examon_data_index, "%d;%s", message_addr[i], examon_time);
				//mqtt_examon_message.length = sizeof(uint32_t)*(1);
                mqtt_examon_message.length = strlen(examon_data_index);
                //mqtt_examon_message.address = (void*)&message_addr[i];
                mqtt_examon_message.address = (void*)examon_data_index;
				publisher_send(&mqtt_examon_pub, &mqtt_examon_message);
				#endif
			}

			//next line
            #ifdef USE_FILE_DB
			fprintf(fp, "\n");
            #endif

		} else if (vartype == 2) {
			float* message_addr = (float*)message->payload;

			for (int i=0; i<length-MQTT_SIGN_DIM; i++)
			{
				// print the data
                #ifdef USE_FILE_DB
				fprintf(fp, "%f, ", message_addr[i]);
                #endif

				// send through examon
				#ifdef USE_EXAMON
				// finish topic
				snprintf(mqtt_examon_message.topic, EXAMON_MAX_TOPIC_CHAR, "%s%d%s",examon_topic, i, examon_final_name);
				//strcpy(mqtt_examon_message.topic, examon_topic);
                char* examon_data_index = examon_data + i*64*sizeof(char);
                sprintf(examon_data_index, "%f;%s", message_addr[i], examon_time);
                //mqtt_examon_message.length = sizeof(float)*(1);
                mqtt_examon_message.length = strlen(examon_data_index);
				//mqtt_examon_message.address = (void*)&message_addr[i];
                mqtt_examon_message.address = (void*)examon_data_index;
				publisher_send(&mqtt_examon_pub, &mqtt_examon_message);
				#endif
			}

			//next line
            #ifdef USE_FILE_DB
			fprintf(fp, "\n");
            #endif

        } else if (vartype == 3) //instructions
		{
			uint32_t* message_addr = (uint32_t*)message->payload;

			for (int i=0; i<length-MQTT_SIGN_DIM; i++)
			{
				// print the data
                #ifdef USE_FILE_DB
				fprintf(fp, "%d, ", message_addr[i]);
                #endif

				// send through examon
				#ifdef USE_EXAMON
				// finish topic
				snprintf(mqtt_examon_message.topic, EXAMON_MAX_TOPIC_CHAR, "%s%d%s%d",examon_topic, (i/WL_STATES), examon_final_name, (i%WL_STATES));
                //strcpy(mqtt_examon_message.topic, examon_topic);
                //printf("topic: %s\n\r", mqtt_examon_message.topic);
                char* examon_data_index = examon_data + i*64*sizeof(char);
                sprintf(examon_data_index, "%d;%s", message_addr[i], examon_time);
				//mqtt_examon_message.length = sizeof(uint32_t)*(1);
                mqtt_examon_message.length = strlen(examon_data_index);
                //mqtt_examon_message.address = (void*)&message_addr[i];
                mqtt_examon_message.address = (void*)examon_data_index;
				publisher_send(&mqtt_examon_pub, &mqtt_examon_message);
				#endif
			}

			//next line
            #ifdef USE_FILE_DB
			fprintf(fp, "\n");
            #endif


		} else {
			printf("[Collector] Error in parsing datatype! type: %d\n\r", vartype);
		}
    #ifdef USE_FILE_DB
	} //if (fp != NULL)
	else
	{
		printf("[Collector] Error in parsing the topic! topic: %s\n\r", message->topic);
	}
    #endif

}

#endif //lib
