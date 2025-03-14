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

#ifndef PERF_MODEL_H_
#define PERF_MODEL_H_

#include "cmdconf.h"
#include <stdint.h>

#include "sim_config.h"

struct quantum_st {
    float *wl_perc;
    float us_wait_time;
    float us_waited_time;
    uint8_t change_freq;
    uint8_t change_pw_budget;
    uint8_t change_voltage;
    uint8_t change_bindings;
    uint32_t barrier_synch;
    // int wl_perc_dim;
    // float freq;
    // uint32_t instruct_count;
    // ElementType type;
    // uint32_t cid;
    // uint32_t id;
    float cpi_freq;
    float cpi_mem;
    float dist[2];
    int64_t dim;
    int64_t consumed;
    struct quantum_st *next;
    struct quantum_st *prev;

    struct quantum_st *nextit;
    int ifiter;
};

/*
 * always keep:
 *  1) head;
 *  2) current;
 *  (3)) prev for creation;
 */

uint32_t quantum_dim;
uint32_t quanta_finish_and_restart;

uint32_t *request_changing_freq;
uint32_t *request_changing_pw_budget;
uint32_t *request_changing_voltage;
uint32_t *request_changing_bindings;

// TODO: pointers below should be separated between accessible and not accessible
struct quantum_st **head_quantum;    // constant
struct quantum_st **current_quantum; // used by generate
struct quantum_st **last_quantum;    // used by populate

struct quantum_st **idle_quantum;

void init_quanta(void);
struct quantum_st *allocate_quantum(struct element_st *element);
void dealloc_quantum(struct quantum_st *ptr);
void create_quantum(struct element_st *element, int elem_counter);
int generate_wl(float *owl, float ifreq, int elem_counter, struct element_st *element);
int get_component_states_number(struct element_st *element);

#define WAIT_STATE 0 // TODO move to another header

// TODO:
// insert_quantum()
// copy_and_add_quantum()

// TODO: remove, rearrange
//  typedef struct _interface {
//      float power_pkg;
//      uint32_t cpu_freq;
//      float cpu_ipc; //instruction per cycle
//      uint32_t cpu_cycles; // total cycles
//      uint32_t cpu_inst_ret; //
//      uint32_t vect_ops; // numero istruzioni vettoriali
//      uint32_t fp_ops; //number of floating point operations
//      uint32_t mem_ops; // !!
//      float power_l2; // !!
//      float time; //total time
//      uint32_t zero; // !!
//      uint32_t maxpower_cpu;
//      uint32_t maxpower_l2;
//  } interface;

// interface read_workload(char* h5_path /*interface *wl*/);

#endif // lib
