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

#include "cmdconf.h"

#include <stdio.h>


//---
//CONFIGURATION
//---
/*
const int CoreQuad[Nc] = 			{0, 0, 0, 1, 0, 0, 1, 1, 0,
                                    1, 0, 0, 1, 0, 0, 1, 1, 0,
                                    1, 0, 0, 1, 0, 0, 1, 1, 0,
                                    1, 0, 0, 1, 0, 0, 1, 1, 0};
*/
const float Tamb = 					25.0 + 273.15;
//const float ThermalCorrection = 	0.41;//0.75;
const float ThermalCorrection[N_EPI_CORE] = {
    /*
     0.1651,    0.2722,    0.3495,    0.3495,    0.2722,    0.1651,
    0.2722 ,   0.4488,    0.5763,    0.5763,    0.4488,    0.2722,
    0.3495,    0.5763,    0.7400,    0.7400,    0.5763,    0.3495,
    0.3495,    0.5763,    0.7400,    0.7400,    0.5763,    0.3495,
    0.2722,    0.4488,    0.5763,    0.5763,    0.4488,    0.2722,
    0.1651,    0.2722,    0.3495,    0.3495,    0.2722,    0.1651

    0.0046,    0.0342,    0.0929,    0.0929,    0.0342,    0.0046,
    0.0342,    0.2525,    0.6864,    0.6864,    0.2525,    0.0342,
    0.0929,    0.6864,    1.8659,    1.8659,    0.6864,    0.0929,
    0.0929,    0.6864,    1.8659,    1.8659,    0.6864,    0.0929,
    0.0342,    0.2525,    0.6864,    0.6864,    0.2525,    0.0342,
    0.0046,    0.0342,    0.0929,    0.0929,    0.0342,    0.0046
    
    0.4100,    0.4100,    0.4100,    0.4100,    0.4100,    0.4100,
    0.4100,    0.4100,    0.4100,    0.4100,    0.4100,    0.4100,
    0.4100,    0.4100,    0.4100,    0.4100,    0.4100,    0.4100,
    0.4100,    0.4100,    0.4100,    0.4100,    0.4100,    0.4100,
    0.4100,    0.4100,    0.4100,    0.4100,    0.4100,    0.4100,
    0.4100,    0.4100,    0.4100,    0.4100,    0.4100,    0.4100
    */
    0.95f,   0.95f,  0.95f,  0.95f,  0.95f,  0.95f,
    0.95f,   0.95f,  0.95f,  0.95f,  0.95f,  0.95f,
    0.95f,   0.95f,  0.95f,  0.95f,  0.95f,  0.95f,
    0.95f,   0.95f,  0.95f,  0.95f,  0.95f,  0.95f,
    0.95f,   0.95f,  0.95f,  0.95f,  0.95f,  0.95f,
    0.95f,   0.95f,  0.95f,  0.95f,  0.95f,  0.95f
};

//int steps_per_sim_time_ms = 1000;
//int sim_hw_multiplier = 25;
//int sim_multiplier = 4; //2
