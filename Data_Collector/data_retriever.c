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
//System & Thread
#include <pthread.h>

#include "mqtt_config.h"
#include "mqtt_collector.h"
#include "mqtt_publisher.h"

#include "mqtt_transl_layer.h"

//#include "main.h"

char* mqtt_topics[NUMBER_OF_TOPICS] = {NULL};
FILE* fp_t[NUMBER_OF_TOPICS] = {NULL};

/*
struct saved_data {
	//TODO fix these: *_dim no more a #define
	uint32_t id;
	float temp[otp_model_core_temp_dim];
	float target_freq[otpc_core_target_freq_dim];
	float core_freq[ifp_ctrl_core_freq_dim];
	float quad_freq[ifp_ctrl_quad_freq_dim];
	float quad_vdd[ifp_ctrl_quad_vdd_dim];
	uint32_t instr[otp_instructions_information_dim];
	float pw_budget[otpc_domain_pw_budget_dim];
	float pw_domain[otp_domain_pw_dim];
	uint32_t bindings[otpc_core_bindings_dim];
};
struct saved_data shared_file_input;
*/

int main(int argc, char** argv)
{

	int wait_min = 5;

	if (argc>1)
	{
		wait_min = atoi(argv[1]);
		//printf("%d\n", wait_min);
	}

	/***** Var *****/

	char t0[TOPIC_STRING_DIM] = "/data/otp/domain_pw";
	mqtt_topics[0] = t0;
	char t1[TOPIC_STRING_DIM] = "/data/otp/temp";
	mqtt_topics[1] = t1;
	char t2[TOPIC_STRING_DIM] = "/data/otp/instr";
	mqtt_topics[2] = t2;
	char t3[TOPIC_STRING_DIM] = "/data/ifp/domain_freq";
	mqtt_topics[3] = t3;
	char t4[TOPIC_STRING_DIM] = "/data/ifp/domain_vdd";
	mqtt_topics[4] = t4;
	char t5[TOPIC_STRING_DIM] = "/data/ifp/core_freq";
	mqtt_topics[5] = t5;
	char t6[TOPIC_STRING_DIM] = "/cmd/target_freq";
	mqtt_topics[6] = t6;
	char t7[TOPIC_STRING_DIM] = "/cmd/pw_budget";
	mqtt_topics[7] = t7;
	char t8[TOPIC_STRING_DIM] = "/cmd/bindings";
	mqtt_topics[8] = t8;
	char t9[TOPIC_STRING_DIM] = "/data/ifp/alpha";
	mqtt_topics[9] = t9;
	char t10[TOPIC_STRING_DIM] = "/data/ifp/redpw";
	mqtt_topics[10] = t10;
	char t11[TOPIC_STRING_DIM] = "/data/ifp/freqredmap";
	mqtt_topics[11] = t11;
	

	#ifdef USE_FILE_DB
	fp_t[0] = fopen ("./data/domain_pw.csv","w");
	fp_t[1] = fopen ("./data/temp.csv","w");
	fp_t[2] = fopen ("./data/instr.csv","w");
	fp_t[3] = fopen ("./data/domain_freq.csv","w");
	fp_t[4] = fopen ("./data/domain_vdd.csv","w");
	fp_t[5] = fopen ("./data/core_freq.csv","w");
	fp_t[6] = fopen ("./data/target_freq.csv","w");
	fp_t[7] = fopen ("./data/pw_budget.csv","w");
	fp_t[8] = fopen ("./data/bindings.csv","w");
	fp_t[9] = fopen ("./data/alpha.csv","w");
	fp_t[10] = fopen ("./data/redpw.csv","w");
	fp_t[11] = fopen ("./data/freqredmap.csv","w");
	for (int i=0; i<NUMBER_OF_TOPICS; i++)
	{
		if (fp_t[i] == NULL) {
			printf("[DR] Error opening file %d\n", i);
			perror(" ");			
		}
	}
	#endif

	/* Opening the MQTT retriever */
	struct mqtt_instance mqtt_retriever;
	struct mqtt_collector_message mqtt_message;

	mqtt_retriever.message = (void*)&mqtt_message;

	mqtt_message.lock = &message_lock[0];
	mqtt_message.signal = &sem_mqtt_publish_done[0];

	mqtt_retriever.mqtt_topic = (char *)malloc(TOPIC_STRING_DIM*sizeof(char));
	mqtt_retriever.broker_ip = (char *)malloc(32*sizeof(char));
	mqtt_retriever.username = (char *)malloc(32*sizeof(char));
	mqtt_retriever.passwd = (char *)malloc(32*sizeof(char));
	strcpy(mqtt_retriever.broker_ip, MQTT_BROKER_IP); //MQTT_BROKER_IP
	mqtt_retriever.broker_port = MQTT_BROKER_PORT;
	strcpy(mqtt_retriever.username, MQTT_USERNAME); //MQTT_USERNAME
	strcpy(mqtt_retriever.passwd, MQTT_PASSWD); //MQTT_PASSWD

	//snprintf(mqtt_retriever.mqtt_topic, TOPIC_STRING_DIM, "%s%s", base_topic, "/#");
	sprintf(mqtt_retriever.mqtt_topic, "pms/#");

	//printf("%s\n\r", mqtt_retriever.mqtt_topic);

	#ifdef USE_EXAMON
	/* Opening the MQTT examom Publisher */

	mqtt_examon_pub.message = (void*)&mqtt_examon_message;
	mqtt_examon_message.mid = 0;
	mqtt_examon_message.topic = (char *)malloc(EXAMON_MAX_TOPIC_CHAR*sizeof(char));
	mqtt_examon_pub.broker_ip = (char *)malloc(32*sizeof(char));
	mqtt_examon_pub.username = (char *)malloc(32*sizeof(char));
	mqtt_examon_pub.passwd = (char *)malloc(32*sizeof(char));
	//strcpy(mqtt_examon_pub.broker_ip, "131.175.206.244");
	strcpy(mqtt_examon_pub.broker_ip, "137.204.213.167"); //MQTT_BROKER_IP
	mqtt_examon_pub.broker_port = 1883;
	strcpy(mqtt_examon_pub.username, ""); //MQTT_USERNAME
	strcpy(mqtt_examon_pub.passwd, ""); //MQTT_PASSWD

	printf("[Retriever]: Starting Examon Publisher...\n");

	publisher_init(&mqtt_examon_pub);
	publisher_start(&mqtt_examon_pub);

	epoch_time = time(NULL);
	//printf("time: %d\n", epoch_time);
	
	char base_topic[TOPIC_STRING_DIM] = "pms/board0/chip0";
	#endif

    #ifdef USE_FILE_DB_OLD
	// Create the database file:
	FILE* fp;
	fp = fopen ("output.csv","w");
	fprintf(fp, "MId, ");
	for (int i=0; i<otp_model_core_temp_dim; i++)
	{
		char name[15] = "Temp[";
		char core[3];
		sprintf(core, "%d", i);
		strcat(name, core);
		strcat(name, "], ");
		fprintf(fp, "%s", name);
	}
	for (int i=0; i<otpc_core_target_freq_dim; i++)
	{
		char name[23] = "Target_freq[";
		char core[3];
		sprintf(core, "%d", i);
		strcat(name, core);
		strcat(name, "], ");
		fprintf(fp, "%s", name);
	}
	for (int i=0; i<ifp_ctrl_core_freq_dim; i++)
	{
		char name[23] = "Core_freq[";
		char core[3];
		sprintf(core, "%d", i);
		strcat(name, core);
		strcat(name, "], ");
		fprintf(fp, "%s", name);
	}
	for (int i=0; i<ifp_ctrl_quad_freq_dim; i++)
	{
		char name[23] = "Quad_freq[";
		char core[3];
		sprintf(core, "%d", i);
		strcat(name, core);
		strcat(name, "], ");
		fprintf(fp, "%s", name);
	}
	for (int i=0; i<ifp_ctrl_quad_vdd_dim; i++)
	{
		char name[23] = "Quad_vdd[";
		char core[3];
		sprintf(core, "%d", i);
		strcat(name, core);
		strcat(name, "], ");
		fprintf(fp, "%s", name);
	}
	for (int i=0; i<Nc; i++)
	{
		for (int j=0; j<otp_instructions_information_dim/Nc; j++)
		{
			char name[28] = "Instr[";
			char core[8];
			sprintf(core, "%d-%d", i, j);
			strcat(name, core);
			strcat(name, "], ");
			fprintf(fp, "%s", name);
		}
	}
	for (int i=0; i<otpc_domain_pw_budget_dim; i++)
	{
		char name[23] = "pw_budget[";
		char core[3];
		sprintf(core, "%d", i);
		strcat(name, core);
		strcat(name, "], ");
		fprintf(fp, "%s", name);
	}
	for (int i=0; i<otp_domain_pw_dim; i++)
	{
		char name[23] = "pw_domain[";
		char core[3];
		sprintf(core, "%d", i);
		strcat(name, core);
		strcat(name, "], ");
		fprintf(fp, "%s", name);
	}
	for (int i=0; i<otpc_core_bindings_dim; i++)
	{
		char name[28] = "bindings[";
		char core[3];
		sprintf(core, "%d", i);
		strcat(name, core);
		strcat(name, "], ");
		fprintf(fp, "%s", name);
	}
	fprintf(fp, "\n\r");
	#endif

	/*
	* Init collectors and subscribe to the topic.
	*/
	if(collector_init(&mqtt_retriever)){
		printf("[Retriever]: Init error.\n");
		return 1;
	}

	printf("[Retriever]: Init Done!\n");

	/*
	* Start monitoring the metrics.
	*/
	if(collector_start(&mqtt_retriever)){
		printf("[Retriever]: Start error.\n");
		return 1;
	}

	printf("[Retriever]: Started! \n");

	// Wait until connection:
	while(!mqtt_retriever.flag_en);

	printf("[Retriever]: Ready! \n");

	//while(1) //counter < DATA_PRINTED){}
	sleep(wait_min*60);

	#ifdef USE_FILE_DB_OLD
	fclose(fp); //never forget this
	#endif
	#ifdef USE_FILE_DB
	for (int i=0; i<NUMBER_OF_TOPICS; i++)
	{
		fclose(fp_t[i]);
	}
	#endif

	return 0;

}
