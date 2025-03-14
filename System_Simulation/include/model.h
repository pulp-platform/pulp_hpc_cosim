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

#ifndef INC_MODEL_H_
#define INC_MODEL_H_

#include "cmdconf.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h>

void model_initialization(int Nc, int Nh, int Nv, float Ts, uint32_t *otdim, uint32_t *oudim);
void model_step(float *oTemp, float *oDomainPower, float (*iWlPerc)[WL_STATES], int sensor_read);
void matMul(double *O, const double *iA, double *iB, int r1, int col1, int col2);
void matSum(double *O, double *iA, double *iB, int r1, int col1);

void generateGaussianNoise(float *z0, float *z1, float mu, float sigma);

int create_thermal_model(double **A_th, double **B_th, int Nc, int Nh, int Nv, float Ts);

#endif /* INC_MODEL_H_ */
