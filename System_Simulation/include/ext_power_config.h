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

#ifndef INC_EXT_PW_CONFIG_H_
#define INC_EXT_PW_CONFIG_H_

#include "cmdconf.h"
#include "wl_config.h"

#define N_EXT_PW_SEQ 5

/*** Data Struct ***/
typedef struct _extpw_data {
    float power;
    uint32_t time_us;
} extpw_data_t;

// To simplify and better thing should end with 0/0
extpw_data_t ext_power_input[N_EXT_DOMAIN][N_EXT_PW_SEQ];

float total_ext_power(int us_per_step);

#endif // lib INC_EXT_PW_CONFIG_H_
