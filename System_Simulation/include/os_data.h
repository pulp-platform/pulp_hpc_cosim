
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
 * main.h
 *
 *  Created on: 19 feb 2020
 *      Author: giova
 */

#ifndef _OS_DATA_H_
#define _OS_DATA_H_

//Standard Lib
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h>

//System & Thread


//Mem


//

//MyLib

//Model
#include "cmdconf.h"

//OS

//Govern


//---
//COMMANDS
//---
int N_FREQ_SEQ;
int N_PW_BUDGET_SEQ;
int N_BINDING_MAT_SEQ;


/*** Data Struct ***/
typedef struct _input_data {
	float value;
    int time;
} input_data_t;
/*
typedef struct _input_data2 {
	uint16_t value[Nc];
    int time;
} input_data_bm_t;
*/
typedef struct _input_data2 {
    int value;
    int time;
} input_data_bm_t;

void init_os(void);

//---
//COMMANDS
//---

input_data_t* TargetFrequency;
input_data_t* QuadPwrBudget;
input_data_t* BoardPwrBudget;
input_data_bm_t* BindMatrix;


#endif //lib