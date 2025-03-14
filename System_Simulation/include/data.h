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

#ifndef INC_DATA_H_
#define INC_DATA_H_

#include "cmdconf.h"

#define TEMP_DER_STEP \
    10 // Steps in which the Temperature is divided,
       // used by power model to compute the function wrt temp

#define MAX_TEMP 120
#define NOISE_MAT_DIM 1000

/*
const float Icc[WL_STATES];
const float Icc_Chiplet;
const float Icc_coreUncert[Nc];
const float Icc_tempDer[(MAX_TEMP / TEMP_DER_STEP)];
const float Icc_noise[NOISE_MAT_DIM];

const float Ceff[WL_STATES];
const float Ceff_Chiplet;
const float Ceff_coreUncert[Nc] ;
const float Ceff_tempDer[(110 / TEMP_DER_STEP)];
const float Ceff_noise[NOISE_MAT_DIM];
*/

// const float P_thrott__;
// const float P_thrott_coreUncert[Nc];

const double A_test[N_EPI_CORE * 2 * N_EPI_CORE * 2];
const double B_test[N_EPI_CORE * 2 * (N_EPI_CORE + 1)];

const double TempSensorNoise[NOISE_MAT_DIM];

#endif /* INC_DATA_H_ */
