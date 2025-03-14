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

#ifndef _MQTT_COLLECTOR_
#define _MQTT_COLLECTOR_

#include <stdio.h>
#include <sys/time.h>
#include <stdlib.h>
#include <string.h>

#include "mosquitto.h"
#include "mqtt_config.h"



/**
 * collector_init() - Initialize collector
 * @val:              Struct containing the state of the metric
 *
 * Returns:           0 on Success, 1 on Failure.
 **/
int collector_init(struct mqtt_instance *val);

/**
 * collector_start() - Start monitoring the metric
 * @val:               Struct containing the state of the metric
 *
 * Returns:            0 on Success, 1 on Failure.
 **/
int collector_start(struct mqtt_instance *val);

/**
 * collector_get() - Get mean value, but continue the monitoring
 * @val:             Struct containing the state of the metric
 *
 * Returns:          0 on Success, 1 on Failure.
 **/
//int collector_get(struct mqtt_instance *val);

/**
 * collector_clean() - Cleanup collector
 * @val:               Struct containing the state of the metric
 
 * Returns:            0 on Success, 1 on Failure.
 **/
int collector_stop(struct mqtt_instance *val);

//int collector_pause(struct mqtt_instance *val);

#endif //lib
