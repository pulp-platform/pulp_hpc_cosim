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

#ifndef _MAIN_H_
#define _MAIN_H_

// Standard Lib
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h>

// System & Thread
#include <pthread.h>
#ifdef USE_MYSEM
#include "mySem.h"
#else
#include <semaphore.h>
#endif
#include <errno.h>

// Mem
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

//

// MyLib
#include "main.h"
#include "addresses.h"
// Model
// #include "model.h"
#include "cmdconf.h"
// #include "wl_config.h"
// #include "ext_power_config.h"
// OS

// Govern

// time
#include <sys/time.h>
#include <time.h>

/****************************************/

/** Global Vars **/

//* global Model Var: *//

/* Names:
 * otp: output to pulp
 * ifp: input from pulp
 * gd: global data
 * gs: global signal
 * gn: global command
 * gp: global data passing (data exchanged btw threads)
 *
 */

// struct timespec threadtime[4][40];

// outputs (to) --> Pulp:
float *otp_domain_pw;
int otp_domain_pw_dim;
float *otp_model_core_temp;
int otp_model_core_temp_dim;
uint32_t *otp_instructions_information;
int otp_instructions_information_dim;
int *gs_workload_acc_read;
// cmd outputs
float *otpc_core_target_freq;
int otpc_core_target_freq_dim;
float *otpc_domain_pw_budget;
int otpc_domain_pw_budget_dim;
uint32_t *otpc_core_bindings;
int otpc_core_bindings_dim;

// inputs (from) <-- Pulp:
float *ifp_ctrl_quad_freq;
int ifp_ctrl_quad_freq_dim;
float *ifp_ctrl_quad_vdd;
int ifp_ctrl_quad_vdd_dim;
float *ifp_ctrl_core_freq;
int ifp_ctrl_core_freq_dim;
// TODO:CANC
float *ifp_debug_alpha;
float *ifp_debug_redpw;
uint32_t *ifp_debug_freqredmap;
int ifp_debug_alpha_dim;
int ifp_debug_redpw_dim;
int ifp_debug_freqredmap_dim;
//
// SCMI
float *hlc_workload_delivery;

// Simulation
volatile int *gn_run_simulation;
volatile int *gn_pause_simulation;
unsigned int CycleIDnumber;
// XTime Timer_measured_us;

/*** pthread ***/
void *pthread_workload_computation(void *ptr);
void *pthread_model_execution(void *ptr);
void *pthread_os_scmi_sim(void *ptr);
void *pthread_govern_interface(void *ptr);
void *pthread_data_scalper(void *ptr);
void *pthread_data_saver(void *ptr);
void *pthread_hlc_execution(void *ptr);

pthread_mutex_t pthread_lock_gp_wl;
// data passing
float *gp_computed_workload;
// Peripherals Exclusion
pthread_mutex_t pthread_lock_printf;

// Synchro Stuff
#ifdef USE_MYSEM
mySem_t sem_to_Model;
mySem_t sem_to_Wl;
mySem_t sem_os_timer_tick;
mySem_t sem_model_timer_tick;
mySem_t sem_scalper_timer_tick;
mySem_t sem_to_HLC;
mySem_t sem_saver_timer_tick;
#else
sem_t sem_to_Model;
sem_t sem_to_Wl;
sem_t sem_os_timer_tick;
sem_t sem_model_timer_tick;
sem_t sem_scalper_timer_tick;
sem_t sem_to_HLC;
sem_t sem_saver_timer_tick;
#endif

// Linux stuff
int dynamic_flag;

struct t_eventData {
    int myData;
};

// mem stuff
/*int mem_fd;
void *mem_pointer;
void *virt_addr;
*/

#endif // lib
