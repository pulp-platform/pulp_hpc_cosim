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

#ifndef INC_SIM_CONFIG_H_
#define INC_SIM_CONFIG_H_

#include "cmdconf.h"
#include <stdint.h>

/*
typedef int bool;
#define false 0
#define true 1
*/

typedef enum {
    JT_CORE = 0,
    JT_HBM = 1,
    JT_COMMON = 99
} ElementType;

/*
typedef enum {
    ZERO = 0,
    IDLE = 1,
    DRYSTONE = 2,
    DGEMM = 3,
    MAXP_CPU = 4,
    MAXP_L2 = 5,
} PatternType;

typedef enum {
    LOW = 0,
    MID = 1,
    HIGH = 2,
} VoltageType;
*/

enum {
    MAX_JSON_DIM = 8192,
    STRING_DIM_NOISE_TYPE = 9
};

struct element_st;

struct gaussian_variation_t {
    float mean;
    float variance;
    float variance_limits_perc[2];
    float three_sigma_prop_perc;
};
struct noise_t {
    char type[STRING_DIM_NOISE_TYPE];
    float snr;
};
struct uncert_t {
    //float inter_wafer_silicon_var_perc;
    struct gaussian_variation_t intra_die_silicon_var;
    struct noise_t temperature_noise;
    struct noise_t workload_noise;
};
struct chiplet_t {
    int cid;
    struct gaussian_variation_t inter_wafer_silicon_var;
    float inter_wafer_silicon_var_perc;
};

struct chiplet_link_t {
    int cid;
    int index;
};

struct core_config_st {
    char* name;
    uint32_t cid;
    float leak_vdd;
    float leak_temp;
    float leak_process;
    uint32_t wl_states;
    float* dyn_pow_cpu_coeff;
    float* workload;
    struct uncert_t uncert;
};

struct common_config_st {
    char* name;
    uint32_t cid;
    float coeff;
    int activity;
};

//struct core_config_st *core_components;
// core power model function
float compute_core_power_cid1(struct element_st *self, float freq, float voltage, float temp,
                                float process,
                                float* workload_perc);
float compute_core_power_cid2(struct element_st *self, float freq, float voltage, float temp,
                                float process,
                                float* workload_perc);
//struct common_config_st *common_components;

struct pos_st {
    int x;
    int y;
};

struct element_st {
    ElementType type;
    //uint32_t cid;
    uint32_t id;
    uint32_t domain;
    struct pos_st position[2]; //TODO
    union {
        struct core_config_st core_config;
        struct common_config_st common_config;
    };
    float (*compute_power)(struct element_st *self, float freq, float voltage, float temp,
                       float process, float* workload_perc); //TBD: here pass the pointer?
};


//struct element_config_st** hw_config;


struct simulation_st {
    uint32_t nb_elements;
    struct element_st* elements;
    //uint32_t nb_components;
    //TODO this below.
    uint32_t nb_cores;
    uint32_t nb_cores_rows;
    uint32_t nb_cores_columns;
    //struct element_st* elements;
    uint32_t nb_power_domains;
};

int initialize_simstruct(char *filepath);

struct simulation_st simulation;

#endif /* INC_SIM_CONFIG_H_*/
