
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



/*
 * comm.h
 *
 *  Created on: 19 giu 2020
 *      Author: giova
 */

#ifndef INC_COMM_H_
#define INC_COMM_H_

//#include <stdio.h>

//---
//CONFIGURATION
//---
#define N_HPC_CORE			36
//#define Nc 				    N_HPC_CORE //both, for legacy
#define WL_STATES			5 //6
//#define N_QUADS				2
#define N_EXT_DOMAIN		4

//Definitions to Create Scenarios:
//#define CCE_1                 //Realistic Scenario
#define EXT_PWR_ACTIVE        //External Power
//#define QUADS                 //Throttling and other stuff

//Other Code definitions
//...

// Initial environmental conditions
const float Tamb;
//const int CoreQuad[Nc];

const float ThermalCorrection;

#define steps_per_sim_time_ms 1000
#define sim_hw_multiplier 25
#define sim_multiplier 8 //4


float *power_meas_precision_us;
int *power_meas_steps_offset;

int global_finished[N_HPC_CORE];

/*** Data Struct ***/
/*
typedef struct _input_data {
	uint16_t value;
    uint16_t time;
} input_data_t;

typedef struct _wl_input {


} wl_input_t;
*/

//const input_data_t EXT_Power[N_EXT_PW_SEQ];

//const wl_Macroblock_t* Workload[Nc][N_WL_SEQ];

#endif /* INC_COMM_H_ */
