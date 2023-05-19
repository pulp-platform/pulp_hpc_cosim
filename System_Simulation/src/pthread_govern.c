
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




//Standard Lib
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h>

//System & Thread
#include <pthread.h>
#include <sched.h>
#ifdef USE_MYSEM
#include "mySem.h"
#else
#include <semaphore.h>
#endif
#include <errno.h>

//Mem

//

//MyLib
#include "main.h"

//Model
#include "cmdconf.h"

//OS

//Govern
#include "mqtt_config.h"
#include "mqtt_collector.h"
#include "mqtt_transl_layer.h"
#include "mqtt_publisher.h"


#ifndef USE_MYSEM
int sem_value = 0;
#endif


typedef struct _send_float {
    float* data;
    uint32_t dim;
    uint32_t timeID;
} send_float_t;

typedef struct _send_uint32 {
    uint32_t* data;
    uint32_t dim;
    uint32_t timeID;
} send_uint32_t;

#ifdef USE_EXAMON
#define EXAMON_MAX_TOPIC_CHAR 128
#define EXAMON_TIME_MUL 200 //this is: average_examon*sim_hw_multiplier*sim_multiplier/5
#include <sys/time.h>
#include <time.h>
time_t epoch_time;
#endif



void* pthread_govern_interface(void *ptr)
{

    while(*gn_run_simulation)
    {
        sleep(2);
    }

    /* Pass the new commands */

    // CMD, ID (core, domain, etc), Value, Duration

}

void* pthread_data_scalper(void *ptr)
{
    uint32_t* l_domain_pw = (uint32_t*)malloc(sizeof(uint32_t)*(otp_domain_pw_dim+1));
    uint32_t* l_model_core_temp = (uint32_t*)malloc(sizeof(uint32_t)*(otp_model_core_temp_dim+1));
    uint32_t* l_instructions_information = (uint32_t*)malloc(sizeof(uint32_t)*(otp_instructions_information_dim+1));
    uint32_t* l_core_target_freq = (uint32_t*)malloc(sizeof(uint32_t)*(otpc_core_target_freq_dim+1));
    uint32_t* l_domain_pw_budget = (uint32_t*)malloc(sizeof(uint32_t)*(otpc_domain_pw_budget_dim+1));
    uint32_t* l_core_bindings = (uint32_t*)malloc(sizeof(uint32_t)*(otpc_core_bindings_dim+1));
    uint32_t* l_ctrl_quad_freq = (uint32_t*)malloc(sizeof(uint32_t)*(ifp_ctrl_quad_freq_dim+1));
    uint32_t* l_ctrl_quad_vdd = (uint32_t*)malloc(sizeof(uint32_t)*(ifp_ctrl_quad_vdd_dim+1));
    uint32_t* l_ctrl_core_freq = (uint32_t*)malloc(sizeof(uint32_t)*(ifp_ctrl_core_freq_dim+1));
    //TODO:CANC
    uint32_t* l_debug_alpha = (uint32_t*)malloc(sizeof(uint32_t)*(ifp_debug_alpha_dim+1));
    uint32_t* l_debug_redpw = (uint32_t*)malloc(sizeof(uint32_t)*(ifp_debug_redpw_dim+1));
    uint32_t* l_debug_freqredmap = (uint32_t*)malloc(sizeof(uint32_t)*(ifp_debug_freqredmap_dim+1));
    if (l_debug_alpha == NULL)
        printf("a ");
    if (l_debug_redpw == NULL)
        printf("b ");
    if (l_debug_freqredmap == NULL)
        printf("c ");


    struct mqtt_instance mqtt_fpga;
	struct mqtt_publisher_message mqtt_message;
    mqtt_message.mid = 0;
    mqtt_fpga.broker_ip = (char *)malloc(32*sizeof(char));
    mqtt_fpga.username = (char *)malloc(32*sizeof(char));
	mqtt_fpga.passwd = (char *)malloc(32*sizeof(char));
    #ifndef USE_EXAMON
    strcpy(mqtt_fpga.broker_ip, MQTT_BROKER_IP);
	mqtt_fpga.broker_port = MQTT_BROKER_PORT;
    strcpy(mqtt_fpga.username, MQTT_USERNAME);
	strcpy(mqtt_fpga.passwd, MQTT_PASSWD);
    #else
    strcpy(mqtt_examon_pub.broker_ip, MQTT_BROKER_IP); //MQTT_BROKER_IP
	mqtt_examon_pub.broker_port = 1883;
	strcpy(mqtt_examon_pub.username, MQTT_USERNAME); //MQTT_USERNAME
	strcpy(mqtt_examon_pub.passwd, MQTT_PASSWD); //MQTT_PASSWD

    int board = -1;
	int chip = -1;
	char examon_base_topic[EXAMON_MAX_TOPIC_CHAR] = "/node/hostname/plugin/pulpcontroller_pub/chnl";
	char examon_topic[EXAMON_MAX_TOPIC_CHAR] = "";
	char examon_cmd[16] = "";
	char examon_position[16] = "";
	char examon_final_name[32] = "";
    #endif //not USE_EXAMON

	mqtt_fpga.message = (void*)&mqtt_message;

	mqtt_message.topic = (char *)malloc(TOPIC_STRING_DIM*sizeof(char));

	printf("[FPGA]: Starting Publisher...\n");

	publisher_init(&mqtt_fpga);
	publisher_start(&mqtt_fpga);

	printf("[FPGA]: Start Completed\n");

    uint32_t timestamp = 0;

    //Average the plot:
    int average_points = 1;
    int average_counter = average_points;

    float* l_average_domain_pw = (float*)calloc(otp_domain_pw_dim, sizeof(float));
    float* l_average_model_core_temp = (float*)calloc(otp_model_core_temp_dim, sizeof(float));
    uint32_t* l_average_instructions_information = (uint32_t*)calloc(otp_instructions_information_dim, sizeof(uint32_t));
    //float l_average_core_target_freq[otpc_core_target_freq_dim];
    //float l_average_domain_pw_budget[otpc_domain_pw_budget_dim];
    //uint32_t l_average_core_bindings[otpc_core_bindings_dim];
    float* l_average_ctrl_quad_freq = (float*)calloc(ifp_ctrl_quad_freq_dim, sizeof(float));
    float* l_average_ctrl_quad_vdd = (float*)calloc(ifp_ctrl_quad_vdd_dim, sizeof(float));
    float* l_average_ctrl_core_freq = (float*)calloc(ifp_ctrl_core_freq_dim, sizeof(float));

    //
    float* l_average_debug_alpha = (float*)calloc((ifp_debug_alpha_dim), sizeof(float));
    float* l_average_debug_redpw = (float*)calloc((ifp_debug_redpw_dim), sizeof(float));

 	//Infinite Cycle:
    while(*gn_run_simulation)
    {
    	//sem here, activated through timer
	    #ifdef USE_MYSEM
        mySem_wait(&sem_govern_timer_tick);
        #else
        sem_wait(&sem_govern_timer_tick);
        #endif


        if (!(*gn_pause_simulation))
        {
            #ifdef PRINTF_ACTIVE
            //printf("Govern Thread runs in CPU %d\n",sched_getcpu());
            #endif

            //averaging:
            for (int i = 0; i < otp_domain_pw_dim; i++) {
                l_average_domain_pw[i] += otp_domain_pw[i];
            }
            for (int i = 0; i < otp_model_core_temp_dim; i++) {
                l_average_model_core_temp[i] += otp_model_core_temp[i];
            }
            for (int i = 0; i < otp_instructions_information_dim; i++) {
                l_average_instructions_information[i] += otp_instructions_information[i];
            }
            /*
            for (int i = 0; i < otpc_core_target_freq_dim; i++) {
                l_average_core_target_freq[i] += otpc_core_target_freq[i];
            }
            for (int i = 0; i < otpc_domain_pw_budget_dim; i++) {
                uint32_t* app_add = otpc_domain_pw_budget;
                l_domain_pw_budget[i] = app_add[i];
            }
            for (int i = 0; i < otpc_core_bindings_dim; i++) {
                uint32_t* app_add = otpc_core_bindings;
                l_core_bindings[i] = app_add[i];
            }
            */
            for (int i = 0; i < ifp_ctrl_quad_freq_dim; i++) {
                l_average_ctrl_quad_freq[i] += ifp_ctrl_quad_freq[i];
            }
            for (int i = 0; i < ifp_ctrl_quad_vdd_dim; i++) {
                l_average_ctrl_quad_vdd[i] += ifp_ctrl_quad_vdd[i];
            }
            for (int i = 0; i < ifp_ctrl_core_freq_dim; i++) {
                l_average_ctrl_core_freq[i] += ifp_ctrl_core_freq[i];
            }

            //
            for (int i = 0; i < ifp_debug_alpha_dim; i++) {
                l_average_debug_alpha[i] += ifp_debug_alpha[i];
            }
            for (int i = 0; i < ifp_debug_redpw_dim; i++) {
                l_average_debug_redpw[i] += ifp_debug_redpw[i];
            }


            //checking
            average_counter--;
            if (average_counter <= 0)
            {
                average_counter = average_points;


                /* Copy mem */
                timestamp = CycleIDnumber;

                for (int i = 0; i < otp_domain_pw_dim; i++) {
                    l_average_domain_pw[i] /= (float)average_points;
                    uint32_t* app_add = l_average_domain_pw;
                    l_domain_pw[i] = app_add[i];
                }
                for (int i = 0; i < otp_model_core_temp_dim; i++) {
                    l_average_model_core_temp[i] /= (float)average_points;
                    l_average_model_core_temp[i] -= 273.15f;
                    uint32_t* app_add = l_average_model_core_temp;
                    l_model_core_temp[i] = app_add[i];
                }
                for (int i = 0; i < otp_instructions_information_dim; i++) {
                    l_average_instructions_information[i] /= average_points;
                    if (l_average_instructions_information[i] % 4 > 1)
                    {
                        l_average_instructions_information[i]++;
                    }
                    uint32_t* app_add = l_average_instructions_information;
                    l_instructions_information[i] = app_add[i];
                }
                for (int i = 0; i < otpc_core_target_freq_dim; i++) {
                    uint32_t* app_add = otpc_core_target_freq;
                    l_core_target_freq[i] = app_add[i];
                }
                for (int i = 0; i < otpc_domain_pw_budget_dim; i++) {
                    uint32_t* app_add = otpc_domain_pw_budget;
                    l_domain_pw_budget[i] = app_add[i];
                }
                for (int i = 0; i < otpc_core_bindings_dim; i++) {
                    uint32_t* app_add = otpc_core_bindings;
                    l_core_bindings[i] = app_add[i];
                }
                for (int i = 0; i < ifp_ctrl_quad_freq_dim; i++) {
                    l_average_ctrl_quad_freq[i] /= (float)average_points;
                    uint32_t* app_add = l_average_ctrl_quad_freq;
                    l_ctrl_quad_freq[i] = app_add[i];
                }
                for (int i = 0; i < ifp_ctrl_quad_vdd_dim; i++) {
                    l_average_ctrl_quad_vdd[i] /= (float)average_points;
                    uint32_t* app_add = l_average_ctrl_quad_vdd;
                    l_ctrl_quad_vdd[i] = app_add[i];
                }
                for (int i = 0; i < ifp_ctrl_core_freq_dim; i++) {
                    l_average_ctrl_core_freq[i] /= (float)average_points;
                    uint32_t* app_add = l_average_ctrl_core_freq;
                    l_ctrl_core_freq[i] = app_add[i];
                }

                //TODO:CANC
                for (int i = 0; i < ifp_debug_alpha_dim; i++) {
                    uint32_t* app_add = l_average_debug_alpha;
                    l_debug_alpha[i] = app_add[i];
                }
                for (int i = 0; i < ifp_debug_redpw_dim; i++) {
                    uint32_t* app_add = l_average_debug_redpw;
                    l_debug_redpw[i] = app_add[i];
                }
                for (int i = 0; i < ifp_debug_freqredmap_dim; i++) {
                    uint32_t* app_add = ifp_debug_freqredmap;
                    l_debug_freqredmap[i] = app_add[i];
                }


                //l_domain_pw[otp_domain_pw_dim] = timestamp;
                l_domain_pw[1] = timestamp; //TODO: REMOVE FOR ECC23 
                l_model_core_temp[otp_model_core_temp_dim] = timestamp;
                l_instructions_information[otp_instructions_information_dim] = timestamp;
                l_core_target_freq[otpc_core_target_freq_dim] = timestamp;
                //l_domain_pw_budget[otpc_domain_pw_budget_dim] = timestamp;
                l_domain_pw_budget[1] = timestamp; //TODO: REMOVE FOR ECC23
                l_core_bindings[otpc_core_bindings_dim] = timestamp;
                l_ctrl_quad_freq[ifp_ctrl_quad_freq_dim] = timestamp;
                l_ctrl_quad_vdd[ifp_ctrl_quad_vdd_dim] = timestamp;
                l_ctrl_core_freq[ifp_ctrl_core_freq_dim] = timestamp;

                //TODO:CANC
                l_debug_alpha[ifp_debug_alpha_dim] = timestamp;
                l_debug_redpw[ifp_debug_redpw_dim] = timestamp;
                l_debug_freqredmap[ifp_debug_freqredmap_dim] = timestamp;
                
                /*
                sprintf(mqtt_message.topic, "pms/ID");
                mqtt_message.length = sizeof(uint32_t)*1;
                mqtt_message.address = (void*)&timestamp;
                publisher_send(&mqtt_fpga, &mqtt_message);
                */

                #ifndef USE_EXAMON
                sprintf(mqtt_message.topic, "pms/board0/chip0/data/otp/domain_pw");
                //mqtt_message.length = sizeof(uint32_t)*(otp_domain_pw_dim+1);
                mqtt_message.length = sizeof(uint32_t)*(1+1); //TODO: REMOVE FOR ECC23
                mqtt_message.address = (void*)l_domain_pw;
                publisher_send(&mqtt_fpga, &mqtt_message);

                sprintf(mqtt_message.topic, "pms/board0/chip0/cmd/target_freq");
                mqtt_message.length = sizeof(uint32_t)*(otpc_core_target_freq_dim+1);
                mqtt_message.address = (void*)l_core_target_freq;
                publisher_send(&mqtt_fpga, &mqtt_message);

                /*
                sprintf(mqtt_message.topic, "pms/board0/chip0/data/ifp/domain_freq");
                mqtt_message.length = sizeof(uint32_t)*(ifp_ctrl_quad_freq_dim+1);
                mqtt_message.address = (void*)l_ctrl_quad_freq;
                publisher_send(&mqtt_fpga, &mqtt_message);
                */

                sprintf(mqtt_message.topic, "pms/board0/chip0/data/otp/temp");
                mqtt_message.length = sizeof(uint32_t)*(otp_model_core_temp_dim+1);
                mqtt_message.address = (void*)l_model_core_temp;
                publisher_send(&mqtt_fpga, &mqtt_message);

                sprintf(mqtt_message.topic, "pms/board0/chip0/cmd/pw_budget");
                //mqtt_message.length = sizeof(uint32_t)*(otpc_domain_pw_budget_dim+1);
                mqtt_message.length = sizeof(uint32_t)*(1+1); //TODO: REMOVE FOR ECC23 (otpc_domain_pw_budget_dim+1);
                mqtt_message.address = (void*)l_domain_pw_budget;
                publisher_send(&mqtt_fpga, &mqtt_message);

                sprintf(mqtt_message.topic, "pms/board0/chip0/data/ifp/domain_vdd");
                mqtt_message.length = sizeof(uint32_t)*(ifp_ctrl_quad_vdd_dim+1);
                mqtt_message.address = (void*)l_ctrl_quad_vdd;
                publisher_send(&mqtt_fpga, &mqtt_message);


                sprintf(mqtt_message.topic, "pms/board0/chip0/data/otp/instr");
                mqtt_message.length = sizeof(uint32_t)*(otp_instructions_information_dim+1);
                mqtt_message.address = (void*)l_instructions_information;
                publisher_send(&mqtt_fpga, &mqtt_message);  

                sprintf(mqtt_message.topic, "pms/board0/chip0/cmd/bindings");
                mqtt_message.length = sizeof(uint32_t)*(otpc_core_bindings_dim+1);
                mqtt_message.address = (void*)l_core_bindings;
                publisher_send(&mqtt_fpga, &mqtt_message);

                sprintf(mqtt_message.topic, "pms/board0/chip0/data/ifp/core_freq");
                mqtt_message.length = sizeof(uint32_t)*(ifp_ctrl_core_freq_dim+1);
                mqtt_message.address = (void*)l_ctrl_core_freq;
                publisher_send(&mqtt_fpga, &mqtt_message);

                //TODO:Canc
                sprintf(mqtt_message.topic, "pms/board0/chip0/data/ifp/core_alpha");
                mqtt_message.length = sizeof(uint32_t)*(ifp_debug_alpha_dim+1);
                mqtt_message.address = (void*)l_debug_alpha;
                publisher_send(&mqtt_fpga, &mqtt_message);

                sprintf(mqtt_message.topic, "pms/board0/chip0/data/ifp/core_redpw");
                mqtt_message.length = sizeof(uint32_t)*(ifp_debug_redpw_dim+1);
                mqtt_message.address = (void*)l_debug_redpw;
                publisher_send(&mqtt_fpga, &mqtt_message);

                sprintf(mqtt_message.topic, "pms/board0/chip0/data/ifp/core_freqredmap");
                mqtt_message.length = sizeof(uint32_t)*(ifp_debug_freqredmap_dim+1);
                mqtt_message.address = (void*)l_debug_freqredmap;
                publisher_send(&mqtt_fpga, &mqtt_message);

                #else //not USE_EXAMON

                // prepare examon topic:
                snprintf(examon_topic, EXAMON_MAX_TOPIC_CHAR, "%s%s%s%d%s%d%s", examon_base_topic, examon_cmd, "/board/", board, "/chip/", chip, examon_position);
                char* examon_data = (char *)malloc(64*(length-MQTT_SIGN_DIM)*sizeof(char));
                char* examon_time = (char *)malloc(32*sizeof(char));
                //sprintf(examon_time, "%.6f", (float)epoch_time + ((float)timestamp / 1000000.0f));

                #ifdef USE_EXAMON_SIM_TIME
                uint32_t report_add = (timestamp*EXAMON_TIME_MUL / 1000000);

                float test_num = (float)((timestamp*EXAMON_TIME_MUL)-(report_add*1000000)) / 1000000.0f;
                //printf("%f,   ", test_num);
                if (test_num < 0.001f)
                {
                    sprintf(examon_time, "%d.000%d", (epoch_time + report_add),  (int)(timestamp * EXAMON_TIME_MUL - (report_add*1000000) ) );
                }
                else if (test_num < 0.01f)
                {
                    sprintf(examon_time, "%d.00%d", (epoch_time + report_add),  (int)(timestamp * EXAMON_TIME_MUL - (report_add*1000000) ) );
                }
                else if (test_num < 0.1f)
                {
                    sprintf(examon_time, "%d.0%d", (epoch_time + report_add),  (int)(timestamp * EXAMON_TIME_MUL - (report_add*1000000) ) );
                }
                else
                {
                    sprintf(examon_time, "%d.%d", (epoch_time + report_add),  (int)(timestamp * EXAMON_TIME_MUL - (report_add*1000000) ) );
                }
                //printf("%d, %d, %d;   ", report_add, timestamp, (timestamp * EXAMON_TIME_MUL - (report_add*1000000) ) );
                //printf("timestamp: %d, time: %s, epoch: %d, division: %f\n\r", timestamp, examon_time, epoch_time, ((float)timestamp / 1000000.0f));
                #else
                struct timeval tv;
                gettimeofday(&tv, NULL);

                sprintf(examon_time, "%.3f", tv.tv_sec + (tv.tv_usec / 1000000.0));

                #endif //USE_EXAMON_SIM_TIME


                sprintf(mqtt_message.topic, "pms/board0/chip0/data/otp/domain_pw");
                //mqtt_message.length = sizeof(uint32_t)*(otp_domain_pw_dim+1);
                mqtt_message.length = sizeof(uint32_t)*(1+1); //TODO: REMOVE FOR ECC23
                mqtt_message.address = (void*)l_domain_pw;
                publisher_send(&mqtt_fpga, &mqtt_message);

                sprintf(mqtt_message.topic, "pms/board0/chip0/cmd/target_freq");
                mqtt_message.length = sizeof(uint32_t)*(otpc_core_target_freq_dim+1);
                mqtt_message.address = (void*)l_core_target_freq;
                publisher_send(&mqtt_fpga, &mqtt_message);

                /*
                sprintf(mqtt_message.topic, "pms/board0/chip0/data/ifp/domain_freq");
                mqtt_message.length = sizeof(uint32_t)*(ifp_ctrl_quad_freq_dim+1);
                mqtt_message.address = (void*)l_ctrl_quad_freq;
                publisher_send(&mqtt_fpga, &mqtt_message);
                */

                sprintf(mqtt_message.topic, "pms/board0/chip0/data/otp/temp");
                mqtt_message.length = sizeof(uint32_t)*(otp_model_core_temp_dim+1);
                mqtt_message.address = (void*)l_model_core_temp;
                publisher_send(&mqtt_fpga, &mqtt_message);

                sprintf(mqtt_message.topic, "pms/board0/chip0/cmd/pw_budget");
                //mqtt_message.length = sizeof(uint32_t)*(otpc_domain_pw_budget_dim+1);
                mqtt_message.length = sizeof(uint32_t)*(1+1); //TODO: REMOVE FOR ECC23 (otpc_domain_pw_budget_dim+1);
                mqtt_message.address = (void*)l_domain_pw_budget;
                publisher_send(&mqtt_fpga, &mqtt_message);

                sprintf(mqtt_message.topic, "pms/board0/chip0/data/ifp/domain_vdd");
                mqtt_message.length = sizeof(uint32_t)*(ifp_ctrl_quad_vdd_dim+1);
                mqtt_message.address = (void*)l_ctrl_quad_vdd;
                publisher_send(&mqtt_fpga, &mqtt_message);


                sprintf(mqtt_message.topic, "pms/board0/chip0/data/otp/instr");
                mqtt_message.length = sizeof(uint32_t)*(otp_instructions_information_dim+1);
                mqtt_message.address = (void*)l_instructions_information;
                publisher_send(&mqtt_fpga, &mqtt_message);  

                sprintf(mqtt_message.topic, "pms/board0/chip0/cmd/bindings");
                mqtt_message.length = sizeof(uint32_t)*(otpc_core_bindings_dim+1);
                mqtt_message.address = (void*)l_core_bindings;
                publisher_send(&mqtt_fpga, &mqtt_message);

                sprintf(mqtt_message.topic, "pms/board0/chip0/data/ifp/core_freq");
                mqtt_message.length = sizeof(uint32_t)*(ifp_ctrl_core_freq_dim+1);
                mqtt_message.address = (void*)l_ctrl_core_freq;
                publisher_send(&mqtt_fpga, &mqtt_message);

                //TODO:Canc
                sprintf(mqtt_message.topic, "pms/board0/chip0/data/ifp/core_alpha");
                mqtt_message.length = sizeof(uint32_t)*(ifp_debug_alpha_dim+1);
                mqtt_message.address = (void*)l_debug_alpha;
                publisher_send(&mqtt_fpga, &mqtt_message);

                sprintf(mqtt_message.topic, "pms/board0/chip0/data/ifp/core_redpw");
                mqtt_message.length = sizeof(uint32_t)*(ifp_debug_redpw_dim+1);
                mqtt_message.address = (void*)l_debug_redpw;
                publisher_send(&mqtt_fpga, &mqtt_message);

                sprintf(mqtt_message.topic, "pms/board0/chip0/data/ifp/core_freqredmap");
                mqtt_message.length = sizeof(uint32_t)*(ifp_debug_freqredmap_dim+1);
                mqtt_message.address = (void*)l_debug_freqredmap;
                publisher_send(&mqtt_fpga, &mqtt_message);

                #endif //not USE_EXAMON

                //check that everything has been sent correctly

                //??

                //zeroing:
                for (int i = 0; i < otp_domain_pw_dim; i++) {
                    l_average_domain_pw[i] = 0;
                }
                for (int i = 0; i < otp_model_core_temp_dim; i++) {
                    l_average_model_core_temp[i] = 0;
                }
                for (int i = 0; i < otp_instructions_information_dim; i++) {
                    l_average_instructions_information[i] = 0;
                }
                /*
                for (int i = 0; i < otpc_core_target_freq_dim; i++) {
                    l_average_core_target_freq[i] = 0;
                }
                for (int i = 0; i < otpc_domain_pw_budget_dim; i++) {
                    uint32_t* app_add = otpc_domain_pw_budget;
                    l_domain_pw_budget[i] = app_add[i];
                }
                for (int i = 0; i < otpc_core_bindings_dim; i++) {
                    uint32_t* app_add = otpc_core_bindings;
                    l_core_bindings[i] = app_add[i];
                }
                */
                for (int i = 0; i < ifp_ctrl_quad_freq_dim; i++) {
                    l_average_ctrl_quad_freq[i] = 0;
                }
                for (int i = 0; i < ifp_ctrl_quad_vdd_dim; i++) {
                    l_average_ctrl_quad_vdd[i] = 0;
                }
                for (int i = 0; i < ifp_ctrl_core_freq_dim; i++) {
                    l_average_ctrl_core_freq[i] = 0;
                }

                //
                for (int i = 0; i < ifp_debug_alpha_dim; i++) {
                    l_average_debug_alpha[i] = 0;
                }
                for (int i = 0; i < ifp_debug_redpw_dim; i++) {
                    l_average_debug_redpw[i] = 0;
                }

            } //average_counter
        } //pause_sim
    } //run_sim

    free(mqtt_message.topic);
    free(mqtt_fpga.broker_ip);
    free(mqtt_fpga.username);
	free(mqtt_fpga.passwd);

    free(l_domain_pw );
    free(l_model_core_temp);
    free(l_instructions_information);
    free(l_core_target_freq);
    free(l_domain_pw_budget);
    free(l_core_bindings);
    free(l_ctrl_quad_freq );
    free(l_ctrl_quad_vdd );
    free(l_ctrl_core_freq);
    //TODO:CANC
    free(l_debug_alpha);
    free(l_debug_redpw );
    free(l_debug_freqredmap );

    free(l_average_domain_pw);
    free(l_average_model_core_temp );
    free(l_average_instructions_information );
    //
    free(l_average_ctrl_quad_freq );
    free(l_average_ctrl_quad_vdd );
    free(l_average_ctrl_core_freq );

    //
    free(l_average_debug_alpha);
    free(l_average_debug_redpw);
}
