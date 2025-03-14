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

// Standard Lib
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h>

#include <time.h>

// System & Thread
#include <pthread.h>
#include <sched.h>
#ifdef USE_MYSEM
#include "mySem.h"
#else
#include <semaphore.h>
#endif
#include <errno.h>

// Mem

//

// MyLib
#include "main.h"
#include "sim_config.h"

// Model
#include "cmdconf.h"

// Govern

// TODO remove
#include "perf_model.h"

#ifndef USE_MYSEM
int sem_value = 0;
#endif

void *pthread_hlc_execution(void *ptr) {

    /***** Var *****/
    float fmin = 0.8;
    float fmax = SCMI_TOP_FREQ;
    int scmi_freq_levels = 3;

    int scmi_states_num = get_component_states_number(&simulation.elements[SCMI_CORE]);
    // TODO these calloc are not done in the proper way
    float *hlc_workload = (float *)calloc(scmi_states_num * simulation.nb_cores, sizeof(float));

    // others
    float *mem_address = NULL;

/***** Init *****/
#ifdef CPU_AFFINITY
    // we can set one or more bits here, each one representing a single CPU
    cpu_set_t cpuset;
    // the CPU we whant to use
    int cpu = 3;

    CPU_ZERO(&cpuset);     // clears the cpuset
    CPU_SET(cpu, &cpuset); // set CPU 2 on cpuset

    /*
     * cpu affinity for the calling thread
     * first parameter is the pid, 0 = calling thread
     * second parameter is the size of your cpuset
     * third param is the cpuset in which your thread will be
     * placed. Each bit represents a CPU
     */
    sched_setaffinity(0, sizeof(cpuset), &cpuset);
// pthread_setaffinity_np
#endif

    // Bis
    for (int core = 0; core < simulation.nb_elements; core++) {
        otpc_core_target_freq[core] = fmin;
    }
    otpc_core_target_freq[SCMI_CORE] = fmin;

    /* SCMI */
    FILE *fp_scmi;
    fp_scmi = fopen("/sys/devices/system/cpu/cpu0/cpufreq/scaling_setspeed", "w");
    if (fp_scmi == NULL) {
        printf("[SCMI] error opening file\n\r");
    } else {
        fprintf(fp_scmi, "%s", "1");
        fclose(fp_scmi);
        printf("[SCMI] Initialized");
    }
    struct timespec ts;
    char scmi_message = '1';

    // int threadtimer_counter = 0;
    // Infinite Cycle:
    while (*gn_run_simulation) {
        // sem here, activated through timer
#ifdef USE_MYSEM
        mySem_wait(&sem_to_HLC);
#else
        sem_wait(&sem_to_HLC);
#endif

        if (!(*gn_pause_simulation)) {
            // clock_gettime(CLOCK_REALTIME, &threadtime[3][threadtimer_counter++]);
            // if (threadtimer_counter >= 40)
            //     threadtimer_counter = 0;

#ifdef PRINTF_ACTIVE
            printf("HLC runs in CPU %d\n", sched_getcpu());
#endif

            // TODO: wrong size
            for (int i = 0; i < scmi_states_num * simulation.nb_cores; i++) {
                hlc_workload[i] = hlc_workload_delivery[i];
            }

            for (int core = 0; core < simulation.nb_elements; core++) {

                int l_states_num = get_component_states_number(&simulation.elements[core]);
                int idx = core * scmi_states_num + WAIT_STATE;

                if (hlc_workload[idx] > 0.84) {
                    if (core != SCMI_CORE) {
                        otpc_core_target_freq[core] = fmin;
                    } else {
                        scmi_message = '1';
                    }
                    /*
                    else
                    {
                        fp_scmi = fopen("/sys/devices/system/cpu/cpu0/cpufreq/scaling_setspeed",
                    "w"); if (fp_scmi!=NULL){ fprintf(fp_scmi, "%s", "1"); fclose(fp_scmi);
                        }
                        else{
                            printf("[SCMI] error opening file");
                        }
                    }
                    */
                } else if (hlc_workload[idx] > 0.36) {
                    if (core != SCMI_CORE) {
                        otpc_core_target_freq[core] = 2.41f;
                    } else {
                        scmi_message = '2';
                    }
                    /*
                    else
                    {
                        fp_scmi = fopen("/sys/devices/system/cpu/cpu0/cpufreq/scaling_setspeed",
                    "w"); if (fp_scmi!=NULL){ fprintf(fp_scmi, "%s", "2"); fclose(fp_scmi);
                        }
                        else{
                            printf("[SCMI] error opening file");
                        }
                    }
                    */
                } else {
                    if (core != SCMI_CORE) {
                        otpc_core_target_freq[core] = fmax;
                    } else {
                        scmi_message = '3';
                    }
                    /*
                    else
                    {
                        fp_scmi = fopen("/sys/devices/system/cpu/cpu0/cpufreq/scaling_setspeed",
                    "w"); if (fp_scmi!=NULL){ fprintf(fp_scmi, "%s", "3"); fclose(fp_scmi);
                        }
                        else{
                            printf("[SCMI] error opening file");
                        }
                    }
                    */
                }
            }

            // Simulated delay of the SCMI
            // TODO: here is considering only 1 core
            long int wait_time_us = 71 * (sim_hw_multiplier * sim_multiplier - 1);
            ts.tv_sec = wait_time_us / 1000000;
            ts.tv_nsec = (wait_time_us % 1000000) * 1000;
            int res;
            do {
                res = nanosleep(&ts, &ts);
            } while (res && errno == EINTR);
            fp_scmi = fopen("/sys/devices/system/cpu/cpu0/cpufreq/scaling_setspeed", "w");
            if (fp_scmi != NULL) {
                fprintf(fp_scmi, "%c", scmi_message);
                fclose(fp_scmi);
            } else {
                printf("[SCMI] error opening file");
            }

            /**** END ****/

        } // if (!*gn_pause_simulation)

    } // while(*gn_run_simulation)

    close(fp_scmi);
}
