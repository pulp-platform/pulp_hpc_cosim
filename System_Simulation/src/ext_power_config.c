
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
#include "ext_power_config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h>


//To simplify and better thing should end with 0/0
extpw_data_t ext_power_input[N_EXT_DOMAIN][N_EXT_PW_SEQ] = { {{15, 2000}, {11, 2500}, {7, 1000}, {12, 1500}, {13, 1000} },
{{11, 2500}, {0, 2000}, {8, 6000}, {10, 6000}, {0, 0} },
{{7, 1000}, {25, 200}, {24, 100}, {20, 200}, {0, 0} },
{{12, 1500}, {11, 2000}, {10, 200}, {0, 2000}, {0, 0} },
{{13, 1000}, {11, 2000}, {10, 200}, {0, 2000}, {0, 0} }
};


/// la roba che server:
uint32_t seq_counter[N_EXT_DOMAIN] = { 0 };
uint32_t us_counter[N_EXT_DOMAIN] = { 0 };

//TODO:
int freq_time_divider = 2;
int sim_time_divider = steps_per_sim_time_ms;

float total_ext_power(int us_per_step)
{
	if (us_counter[0] >= ext_power_input[0][seq_counter[0]].time_us * sim_time_divider)
	{
		seq_counter[0]++;
		if (seq_counter[0] >= N_EXT_PW_SEQ)
		{
			seq_counter[0] = 0;
		}
		us_counter[0] = 0;
	}
	us_counter[0] += freq_time_divider;

	return ext_power_input[0][seq_counter[0]].power;

	// float total_ext_power = 0.0;
	// //
	// for (int domain = 0; domain<N_EXT_DOMAIN; domain++)
	// {
	// 	int total_us = 0;
	// 	float power = 0.0;

	// 	while (total_us < us_per_step)
	// 	{
	// 		//Take infos
	// 		uint32_t stored_time_us = ext_power_input[domain][seq_counter[domain]].time_us; //
	// 		if (stored_time_us <= 0)
	// 		{
	// 			seq_counter[domain] = 0;
	// 			us_counter[domain] = 0;
	// 			stored_time_us = ext_power_input[domain][seq_counter[domain]].time_us;
	// 		}
	// 		uint32_t time_us = stored_time_us - us_counter[domain];
	// 		/* //useless check
	// 		if(time_us <= 0)
	// 		{
	// 			//signal error
	// 			seq_counter[domain] = 0;
	// 			us_counter[domain] = 0;
	// 			time_us = ext_power_input[domain][seq_counter[domain]].time_us;
	// 		}
	// 		*/
	// 		float ext_power = ext_power_input[domain][seq_counter[domain]].power;

	// 		if ((total_us+time_us) > us_per_step)
	// 		{
	// 			time_us = us_per_step - total_us;
	// 		}
	// 		power += (ext_power * (float)time_us);

	// 		total_us += time_us;
	// 		us_counter[domain] += time_us;
	// 		if(us_counter[domain] >= stored_time_us)
	// 		{
	// 			us_counter[domain] = 0;
	// 			seq_counter[domain]++;
	// 		}

	// 	} // while

	// 	total_ext_power += (power / (float)us_per_step);
	// 	// I cannot do just the sum and divide for us_per_step*N_EXT_DOMAIN outside the for loop because of
	// 	// the case of 0 power for all 100. And because it is mathematically wrong.

	// } //for core


	//return total_ext_power;
} //function
