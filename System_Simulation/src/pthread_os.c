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
#include "sim_config.h"

//Model
#include "cmdconf.h"

//OS
#include "os_data.h"

//Govern

//CSV
#include "csv.h"



#ifndef USE_MYSEM
int sem_value = 0;
#endif



void* pthread_os_scmi_sim(void *ptr)
{

    /***** Var *****/
    int* target_frequency_counter = (int*)calloc(simulation.nb_elements, sizeof(int));
    int* target_frequency_time = (int*)calloc(simulation.nb_elements, sizeof(int));
    int pwr_budget_board_counter;
    int pwr_budget_board_time;
    int* pwr_budget_quad_counter = (int*)calloc((simulation.nb_power_domains), sizeof(int));
    int* pwr_budget_quad_time = (int*)calloc((simulation.nb_power_domains), sizeof(int));
    int bind_mat_counter;
    int bind_mat_time;

    //TODO:
    int core_time_divider = 1;
    int freq_time_divider = core_time_divider;
    int quad_time_divider = core_time_divider;
    int board_time_divider = core_time_divider;
    int bind_time_divider = core_time_divider;
    int freq_time_multiplier = 1;
    int quad_time_multiplier = 10;
    int board_time_multiplier = 10;
    int bind_time_multiplier = 1;

    //others
    float* mem_address = NULL;


    /***** Init *****/
    #ifdef CPU_AFFINITY
    //we can set one or more bits here, each one representing a single CPU
    cpu_set_t cpuset;
    //the CPU we whant to use
    int cpu = 3;

    CPU_ZERO(&cpuset);       //clears the cpuset
    CPU_SET( cpu , &cpuset); //set CPU 2 on cpuset

    /*
    * cpu affinity for the calling thread
    * first parameter is the pid, 0 = calling thread
    * second parameter is the size of your cpuset
    * third param is the cpuset in which your thread will be
    * placed. Each bit represents a CPU
    */
    sched_setaffinity(0, sizeof(cpuset), &cpuset);
    //pthread_setaffinity_np
    #endif

    bind_mat_counter                    = 0;
    bind_mat_time                       = 0;
    pwr_budget_board_counter            = 0;
    pwr_budget_board_time               = 0;

    //Bis
    for (int core = 0; core < simulation.nb_elements; core++)
    {
        otpc_core_target_freq[core] = TargetFrequency[core*N_FREQ_SEQ + 0].value;
    }
    for (int core = 0; core < simulation.nb_cores; core++)
    {
        otpc_core_bindings[core] = BindMatrix[core*N_BINDING_MAT_SEQ + 0].value;
    }
    for (int quad = 0; quad < simulation.nb_power_domains; quad++)
    {
        otpc_domain_pw_budget[quad+1] = QuadPwrBudget[quad*N_PW_BUDGET_SEQ + 0].value;
    }
    otpc_domain_pw_budget[0] = BoardPwrBudget[0].value;

    //int threadtimer_counter = 0;
    //Infinite Cycle:
    while(*gn_run_simulation)
    {
    	//sem here, activated through timer
	    #ifdef USE_MYSEM
        mySem_wait(&sem_os_timer_tick);
        #else
        sem_wait(&sem_os_timer_tick);
        #endif


        if (!(*gn_pause_simulation))
        {
            //clock_gettime(CLOCK_REALTIME, &threadtime[3][threadtimer_counter++]);
            //if (threadtimer_counter >= 40)
            //    threadtimer_counter = 0;

            #ifdef PRINTF_ACTIVE
            printf("OS runs in CPU %d\n",sched_getcpu());
            #endif

            /*** TARGET FREQUENCY ***/
            for (int core = 0; core < simulation.nb_elements; core++)
            {
                target_frequency_time[core]+=freq_time_divider;

                if (target_frequency_time[core] >= TargetFrequency[core*N_FREQ_SEQ + target_frequency_counter[core]].time * freq_time_multiplier)
                {
                    target_frequency_time[core] = 0;
                    target_frequency_counter[core]++;
                    if ( (TargetFrequency[core*N_FREQ_SEQ + target_frequency_counter[core]].time <= 0) || (target_frequency_counter[core] >= N_FREQ_SEQ) )
                    {
                        target_frequency_counter[core] = 0;
                    }

                    /* SCMI*/
                    #ifndef USE_SCMI
                    otpc_core_target_freq[core] = TargetFrequency[core*N_FREQ_SEQ + target_frequency_counter[core]].value;
                    #endif
                }

                if (global_finished[core] == 1)
                {
                    /* SCMI*/
                    #ifndef USE_SCMI
                    otpc_core_target_freq[core] = 1.0;
                    #endif
                }

            } //for (int core = 0; core < simulation.nb_elements; core++)

            /*** PWR BUDGET QUADS ***/
            for (int quad = 0; quad < simulation.nb_power_domains; quad++)
            {
                pwr_budget_quad_time[quad]+=quad_time_divider;

                if (pwr_budget_quad_time[quad] >= QuadPwrBudget[quad*N_PW_BUDGET_SEQ + pwr_budget_quad_counter[quad]].time * quad_time_multiplier)
                {
                    pwr_budget_quad_time[quad] = 0;
                    pwr_budget_quad_counter[quad]++;
                    if ( (QuadPwrBudget[quad*N_PW_BUDGET_SEQ + pwr_budget_quad_counter[quad]].time <= 0) || (pwr_budget_quad_counter[quad] >= N_PW_BUDGET_SEQ) )
                    {
                        pwr_budget_quad_counter[quad] = 0;
                    }

                    otpc_domain_pw_budget[quad+1] = QuadPwrBudget[quad*N_PW_BUDGET_SEQ + pwr_budget_quad_counter[quad]].value;
                }

            } //for (int quad = 0; quad < simulation.nb_power_domains; quad++)

            /*** PWR BUDGET BOARD ***/
            pwr_budget_board_time+=board_time_divider;

            if (pwr_budget_board_time >= BoardPwrBudget[pwr_budget_board_counter].time * board_time_multiplier)
            {
                pwr_budget_board_time = 0;
                pwr_budget_board_counter++;
                if ( (BoardPwrBudget[pwr_budget_board_counter].time <= 0) || (pwr_budget_board_counter >= N_PW_BUDGET_SEQ) )
                {
                    pwr_budget_board_counter = 0;
                }
                otpc_domain_pw_budget[0] = BoardPwrBudget[pwr_budget_board_counter].value;
            }

            /*** BINDING MATRIX ***/
            bind_mat_time+=bind_time_divider;

            if (bind_mat_time >= BindMatrix[0*N_BINDING_MAT_SEQ + bind_mat_counter].time * bind_time_multiplier)
            {
                bind_mat_time = 0;
                bind_mat_counter++;
                if ( (BindMatrix[0*N_BINDING_MAT_SEQ + bind_mat_counter].time <= 0) || (bind_mat_counter >= N_BINDING_MAT_SEQ) )
                {
                    bind_mat_counter = 0;
                }

                for (int core = 0; core < simulation.nb_cores; core++)
	            {
	                otpc_core_bindings[core] = BindMatrix[core*N_BINDING_MAT_SEQ + bind_mat_counter].value;
	            }
            }

            /**** END ****/

            /*** READ THE DATA ***/
            //TODO
            // FreqRedMap
            // ErrorMap
            // Telemetry + cycleID

            /*** WRITE THESE DATA TO RAM ***/
            //TODO

        } //if (!*gn_pause_simulation)

    } //while(*gn_run_simulation)

    free(pwr_budget_quad_counter);
    free(pwr_budget_quad_time);
}
