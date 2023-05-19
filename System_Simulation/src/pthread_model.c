
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
#include <math.h>

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
#include "model.h"
#include "cmdconf.h"
#include "wl_config.h"
#include "ext_power_config.h"
#include "sim_config.h"
//OS

//Govern

#define CYCLYES_DIVIDER 1
#define MAX_MEAN_WINDOW 50 * (steps_per_sim_time_ms / 2) //TODO here should be steps_per_controller_iteration 
//TODO make these avobe variables?

float* power_meas_precision_us = NULL;
int* power_meas_steps_offset = NULL;


void *pthread_workload_computation(void *ptr)
{
    //Translation var
    //uint32_t instruct_count[N_HPC_CORE][WL_STATES];
    uint32_t wl_accum_cycles[N_HPC_CORE];
    uint32_t cycles_per_step[N_HPC_CORE];
    float l_workload_mean[N_HPC_CORE][WL_STATES];
    #ifndef USE_MYSEM
    int sem_value = 0;
    #endif
    int ret = 0;

    int mean_accum_counter = 0;

    /*** Init ***/
    #ifdef CPU_AFFINITY
    //we can set one or more bits here, each one representing a single CPU
    cpu_set_t cpuset;
    //the CPU we whant to use
    int cpu = 1;

    CPU_ZERO(&cpuset);       //clears the cpuset
    CPU_SET( cpu , &cpuset); //set CPU 2 on cpuset

    /*
    * cpu affinity for the calling thread
    * first parameter is the pid, 0 = calling thread
    * second parameter is the size of your cpuset
    * third param is the cpuset in which your thread will be
    * placed. Each bit represents a CPU
    */
    //pthread_setaffinity_np

    sched_setaffinity(0, sizeof(cpuset), &cpuset);
    #endif

    for (int i = 0; i < N_HPC_CORE; i++)
    {
        for (int state = 0; state < WL_STATES; state++)
        	l_workload_mean[i][state] = 0;
    }

    //printf("Init WL Pthread p1 done\n");

    //printf("Init WL Pthread p2 done\n");
    //int threadtimer_counter = 0;
    while(*gn_run_simulation)
    {
        if (!(*gn_pause_simulation))
        {
            //clock_gettime(CLOCK_REALTIME, &threadtime[1][threadtimer_counter++]);
            //if (threadtimer_counter >= 40)
            //    threadtimer_counter = 0;

            #ifdef PRINTF_ACTIVE
            printf("WL Thread runs in CPU %d\n",sched_getcpu());
            #endif

            /*** Workload Function ***/
        	execWlTransl(cycles_per_step);

            //Debug printf
            #ifdef DEBUG_ACTIVE
            ret = pthread_mutex_lock(&pthread_lock_printf);
            if (ret) printf("Pf Mutex Error: %s\n", strerror(ret));
            printf("COMPWL:  WL: %f  %f  %f  %f\n\r", workload_perc[1][0], workload_perc[1][1],
                workload_perc[1][2], workload_perc[1][3]);
            ret = pthread_mutex_unlock(&pthread_lock_printf);
            if (ret) printf("Pf Mutex Error: %s\n", strerror(ret));
            #endif

            /*** Take Lock ***/
            //ret = pthread_mutex_lock(&pthread_lock_gp_wl);
            //if (ret) printf("Mem Mutex Error: %s\n", strerror(ret));
            //memcpy(gp_computed_workload, workload_perc, sizeof(float)*simulation.nb_cores*WL_STATES);
            /*** Release The Lock ***/
            //ret = pthread_mutex_unlock(&pthread_lock_gp_wl);
            //if (ret) printf("Mem Mutex Error: %s\n", strerror(ret));

            /*** Signal the Model Thread the data are Ready ***/
            #ifdef USE_MYSEM
            mySem_post(&sem_to_Model);
            #else
            //Busy Waiting to simulate a Binary Semaphore
            sem_getvalue(&sem_to_Model, &sem_value);
            while(sem_value > 0)
            {sem_getvalue(&sem_to_Model, &sem_value);}
            sem_post(&sem_to_Model);
            #endif

            /*** Manage Pulp Data ***/
            if (*gs_workload_acc_read)
            {
                *gs_workload_acc_read = 0;
                for (int core = 0; core < N_HPC_CORE; core++)
                {
                    wl_accum_cycles[core] = cycles_per_step[core] / CYCLYES_DIVIDER; // /10 we lose precision but we increase time before overflow

                    for (int state = 0; state < WL_STATES; state++)
                    {
                    	//float app = (l_workload_mean[core][state] / (float)wl_accum_cycles[core]) * 100.0f;
                        //otp_instructions_information[core*WL_STATES + state] = (unsigned int)app;
                        //l_workload_mean[core][state] = 0.0f;
                        //wl_accum_cycles[core] = 0;

                        //l_workload_mean[core][state] = ((uint32_t)round(workload_perc[core][state]*100.0f)); // / cycles_per_step[core];
                        l_workload_mean[core][state] = simulation.elements[core].core_config.workload[state]*100.0f; // / cycles_per_step[core];
                    }
                }
            }
            else
            {
                mean_accum_counter++; //TBD: I can improve this and make it per core with a different window / a different offset start?
                //e.g. an array that is initialized at random while the window is constant
                for (int core = 0; core < N_HPC_CORE; core++)
                {
                    if (wl_accum_cycles[core] >= 4000000000)
                    {
                        wl_accum_cycles[core] = 0;
                        for (int state = 0; state < WL_STATES; state++)
                            l_workload_mean[core][state] = 0; //hard reset but no other way                        
                        //printf("overflow\n\r");
                    }
                    if ( (mean_accum_counter > MAX_MEAN_WINDOW + 2) || (mean_accum_counter == 0) ) // + 2 to offset the window otherwise first value was no mean
                    {
                        wl_accum_cycles[core] = 0;
                        for (int state = 0; state < WL_STATES; state++)
                            l_workload_mean[core][state] = 0; //hard reset but no other way                        
                        //printf("reset: %d\n\r", mean_accum_counter);
                        mean_accum_counter = 0;
                    }
                    wl_accum_cycles[core] += cycles_per_step[core]/CYCLYES_DIVIDER;

                    #ifndef USE_INSTRUCTIONS_COMPOSITION
                    unsigned int single_wl_accum = 0;
                    #endif

                    for (int state = 0; state < WL_STATES; state++)
                    {
                        //Provisional means algorithm
                        //( ((uint32_t)round(workload_perc[core][state]*100.0f) * cycles_per_step[core]) - (l_workload_mean[core][state] * cycles_per_step[core]) )
                        float value = ( ((simulation.elements[core].core_config.workload[state]*100.0f) - l_workload_mean[core][state]) * (float)cycles_per_step[core] / (float)CYCLYES_DIVIDER)
                                        / (float)wl_accum_cycles[core];
                        //printf("mean: %d, Value: %f\n\r", l_workload_mean[core][state], value);
                        l_workload_mean[core][state] += value;
                        #ifdef USE_INSTRUCTIONS_COMPOSITION
                        otp_instructions_information[core*WL_STATES + state] = (uint32_t)round(l_workload_mean[core][state]);
                        #else
                        //TODO: This is a nonsense, I should multiply to the Ceff, not divided for wl_accum_cycles. Fix this!
                        single_wl_accum += (uint32_t)(l_workload_mean[core][state]); // / wl_accum_cycles[core]);
                        #endif
                    }
                    #ifndef USE_INSTRUCTIONS_COMPOSITION
                    otp_instructions_information[core*WL_STATES + 0] = single_wl_accum;
                    #endif
                }
            }

            /*** Wait signal from Model to keep synchronization ***/
            #ifdef USE_MYSEM
            mySem_wait(&sem_to_Wl);
            #else
            sem_wait(&sem_to_Wl);
            #endif

        } //*gn_pause_simulation
    }//*gn_run_simulation

    //end
    //return NULL;
}

//////////////////////////////////////////////
/********************************************/
//////////////////////////////////////////////

//////////////////////////////////////////////
/********************************************/
//////////////////////////////////////////////

//////////////////////////////////////////////
/********************************************/
//////////////////////////////////////////////

void *pthread_model_execution(void *ptr)
{
    /* Var */
    float workload_perc[N_HPC_CORE][WL_STATES];
    int *power_meas_precision_steps = (int*)malloc(sizeof(int)*(simulation.nb_power_domains+1)); // = precision_ms * steps_per_sim_time_ms
    unsigned int *power_domain_step_counter = (unsigned int*)malloc(sizeof(unsigned int)*(simulation.nb_power_domains+1));
    float* model_step_domain_pw = (float*)malloc(sizeof(float)*(simulation.nb_power_domains+1));
    #ifndef USE_MYSEM
    int sem_value = 0;
    #endif
    int ret = 0;

    int gtl_nb_power_domains = simulation.nb_power_domains;

    power_meas_precision_us = (float*)malloc(sizeof(float)*(simulation.nb_power_domains+1));
    power_meas_steps_offset = (int*)malloc(sizeof(int)*(simulation.nb_power_domains+1));
    
    //power_meas_precision_us[N_QUADS + 1] = {100, 100, 100};
    //power_meas_steps_offset[N_QUADS + 1] = {0, 0, 0}; //To be fixed, cuz if value !=0, the first power value will be wrong.
    for (int i=0; i<(gtl_nb_power_domains+1); i++)
    {
        power_meas_precision_us[i] = 100;
        power_meas_steps_offset[i] = 0;
    }


    /* Initialization */
    #ifdef CPU_AFFINITY
    //we can set one or more bits here, each one representing a single CPU
    cpu_set_t cpuset;
    //the CPU we whant to use
    int cpu = 2;

    CPU_ZERO(&cpuset);       //clears the cpuset
    CPU_SET( cpu , &cpuset); //set CPU 2 on cpuset

    /*
    * cpu affinity for the calling thread
    * first parameter is the pid, 0 = calling thread
    * second parameter is the size of your cpuset
    * third param is the cpuset in which your thread will be
    * placed. Each bit represents a CPU
    */
    //pthread_setaffinity_np
    sched_setaffinity(0, sizeof(cpuset), &cpuset);
    #endif

    for (int core = 0; core < N_HPC_CORE; core++)
    {
        for (int state = 0; state < WL_STATES; state++)
            workload_perc[core][state] = 0;

        workload_perc[core][0] = 1.0f;
    }
    for (int i = 0; i < (gtl_nb_power_domains +1); i++)
    {
        model_step_domain_pw[i] = 0;
        power_domain_step_counter[i] = power_meas_steps_offset[i];
        power_meas_precision_steps[i] = (int)round((float)power_meas_precision_us[i] * ((float)steps_per_sim_time_ms / 1000.0f));
    }

    //int threadtimer_counter = 0;

    /**** Infinite Cycle: ****/
    while(*gn_run_simulation)
    {

        //sem here, activated through timer
        #ifdef USE_MYSEM
        mySem_wait(&sem_model_timer_tick);
        #else
        sem_wait(&sem_model_timer_tick);
        #endif

        if (!(*gn_pause_simulation))
        {
            //clock_gettime(CLOCK_REALTIME, &threadtime[2][threadtimer_counter++]);
            //if (threadtimer_counter >= 40)
            //    threadtimer_counter = 0;

            #ifdef PRINTF_ACTIVE
            printf("Model Thread runs in CPU %d\n",sched_getcpu());
            #endif

        	model_step(otp_model_core_temp, model_step_domain_pw, workload_perc);

            //printf("model step done\n");

            //#ifdef EXT_PWR_ACTIVE
            model_step_domain_pw[0] += total_ext_power(1000 / steps_per_sim_time_ms);
            //#endif

            CycleIDnumber++;
            if (CycleIDnumber >= 1000000000)
                CycleIDnumber = 0;

            //FAKE!
            /*
            if (CycleIDnumber > 100000)
                *gn_run_simulation = 0;
            */

            for (int i = 0; i < (gtl_nb_power_domains +1); i++)
            {
                power_domain_step_counter[i]++;
                if (power_domain_step_counter[i] > power_meas_precision_steps[i])
                {
                    otp_domain_pw[i] = model_step_domain_pw[i] / power_meas_precision_steps[i];
                    power_domain_step_counter[i] = 0;
                    model_step_domain_pw[i] = 0;
                }
            }

            /*** Wait signal from WL meaning data are Ready ***/
            #ifdef USE_MYSEM
            mySem_wait(&sem_to_Model);
            #else
            sem_wait(&sem_to_Model);
            #endif

            /*** Take Lock ***/
            //ret = pthread_mutex_lock(&pthread_lock_gp_wl);
            //if (ret) printf("Mem Mutex Error: %s\n", strerror(ret));
            //memcpy(workload_perc, gp_computed_workload, sizeof(float)*simulation.nb_cores*WL_STATES);
            /*** Release The Lock ***/
            //ret = pthread_mutex_unlock(&pthread_lock_gp_wl);
            //if (ret) printf("Mem Mutex Error: %s\n", strerror(ret));

            //Debug printf
            #ifdef DEBUG_ACTIVE
            ret = pthread_mutex_lock(&pthread_lock_printf);
            if (ret) printf("Pf Mutex Error: %s\n", strerror(ret));
            printf("MODEL:  Temp: %f,   WL: %f  %f  %f  %f,   PwB: %f\n\r", otp_model_core_temp[1], workload_perc[1][0],
                workload_perc[1][1], workload_perc[1][2], workload_perc[1][3], model_step_domain_pw[1]);
            ret = pthread_mutex_unlock(&pthread_lock_printf);
            if (ret) printf("Pf Mutex Error: %s\n", strerror(ret));
            #endif

            /*** Signal the Wl Thread we fetched the data ***/
            #ifdef USE_MYSEM
            mySem_post(&sem_to_Wl);
            #else
            //Busy waiting to simulate Binary Semaphore
            sem_getvalue(&sem_to_Wl, &sem_value);
            while(sem_value > 0)
            {sem_getvalue(&sem_to_Wl, &sem_value);}
            sem_post(&sem_to_Wl);
            #endif


        } //if (!*gn_pause_simulation)

    } //while(*gn_run_simulation)

    free(power_meas_precision_steps);
    free(power_domain_step_counter);
    free(model_step_domain_pw);
    free(power_meas_precision_us);
    free(power_meas_steps_offset);

    //return NULL;
}
