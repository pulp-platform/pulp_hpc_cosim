
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
#include "wl_config.h"
#include "main.h"
#include "sim_config.h"
//OS

//Govern





wl_block_t init_wl_block[4] = { {1,100 },{0,650 },{1,500}, {0,0} }; //1250
//
wl_block_t memory_wl_block[4] = { { 1,100 },{ 0,1050 },{ 1,100 }, { 0,0 }}; //1250
//
wl_block_t high_compute_wl_block[5] = { { 1,100 }, {3, 11000}, {2, 1000}, {3, 2900}, { 0,0 } }; //15000
//
wl_block_t max_compute_wl_block[5] = { { 1,100 }, {4, 10400}, {3, 3100}, {2, 1400}, { 0,0 } }; //15000
//
wl_block_t open_mpi_wl_block1[5] = { { 1,2000 }, {3, 7400}, {2, 4600}, {0, 1000}, { 0,0 } }; //15000
//
wl_block_t open_mpi_wl_block2[5] = { { 1,3000 }, {4, 4400}, {3, 6600}, {1, 1000}, { 0,0 } }; //15000
//
wl_block_t mix1_wl_block[6] = { { 1,1000 }, {2, 250}, {0, 4750}, {1, 4000}, {3, 5000}, { 0,0 } }; //15000
//
wl_block_t mix2_wl_block[6] = { { 3,1000 }, {1, 4250}, {3, 4500}, {2, 250}, {1, 5000}, { 0,0 } }; //15000
//
wl_block_t mix3_wl_block[6] = { { 1,4000 }, {0, 1750}, {4, 250}, {1, 6000}, {0, 3000}, { 0,0 } }; //15000
//

//
vMacroblock_t maxx_wl[5] = { { init_wl_block, 2 }, { memory_wl_block, 22}, { max_compute_wl_block, 22}, { memory_wl_block, 32}, {NULL, 0} }; //400.000
//
vMacroblock_t high_perf_wl[5] = { { init_wl_block, 2 }, { memory_wl_block, 22}, { high_compute_wl_block, 23}, { memory_wl_block, 20}, {NULL, 0} }; //400.000
//
vMacroblock_t memory_bound_wl[4] = { { init_wl_block, 2 }, { memory_wl_block, 76}, { init_wl_block, 2 }, {NULL, 0} }; //100.000
//
vMacroblock_t init_wl[5] = { { init_wl_block, 10}, { memory_wl_block, 7}, { init_wl_block, 10}, { memory_wl_block, 5}, { NULL, 0 } }; //40.000

//
vMacroblock_t mix1_phase[9] = { { init_wl_block, 4}, { memory_wl_block, 4}, { mix1_wl_block, 10}, { memory_wl_block, 4}, { mix2_wl_block, 10}, { memory_wl_block, 4}, { mix3_wl_block, 10}, { memory_wl_block, 4}, { NULL, 0 } }; //475.000
//
vMacroblock_t mix2_phase[9] = { { init_wl_block, 4}, { memory_wl_block, 4}, { mix2_wl_block, 10}, { memory_wl_block, 2}, { mix3_wl_block, 8}, { memory_wl_block, 2}, { mix2_wl_block, 12}, { memory_wl_block, 8}, { NULL, 0 } }; //475.000
//
vMacroblock_t mix3_phase[9] = { { init_wl_block, 4}, { memory_wl_block, 4}, { mix3_wl_block, 8}, { memory_wl_block, 2}, { mix1_wl_block, 8}, { memory_wl_block, 8}, { mix1_wl_block, 14}, { memory_wl_block, 2}, { NULL, 0 } }; //475.000
//

//
vMacroblock_t openmpi1[18] = { { init_wl_block, 40}, { memory_wl_block, 40}, { high_compute_wl_block, 10}, 
	/*{ memory_bound_wl, 1},*/  {init_wl_block, 20}, { memory_wl_block, 196000}, { init_wl_block, 20 },
	/*{ high_perf_wl, 1},*/  { init_wl_block, 20 }, { memory_wl_block, 20}, { open_mpi_wl_block1, 45000}, { memory_wl_block, 40},
	/* { memory_bound_wl, 1},*/ {init_wl_block, 20 }, { memory_wl_block, 196000}, { init_wl_block, 20 },
	/*{ high_perf_wl, 1}, */ { init_wl_block, 20 }, { memory_wl_block, 20}, { open_mpi_wl_block1, 45000}, { memory_wl_block, 40},
	 { NULL, 0 } }; //1.005.000

vMacroblock_t openmpi2[21] = { { init_wl_block, 40}, { memory_wl_block, 40}, { high_compute_wl_block, 10}, 
	/*{ memory_bound_wl, 1},*/  {init_wl_block, 20 }, { memory_wl_block, 196000}, { init_wl_block, 20 },
	/*{ maxx_wl, 1},*/  { init_wl_block, 20 }, { memory_wl_block, 20}, { open_mpi_wl_block2, 45000}, { memory_wl_block, 40},
	/* { memory_bound_wl, 2},*/ {init_wl_block, 20 }, { memory_wl_block, 196000}, { init_wl_block, 20 }, {init_wl_block, 20 }, { memory_wl_block, 760}, { init_wl_block, 20 },
	/*{ high_perf_wl, 1}, */ { init_wl_block, 20 }, { memory_wl_block, 20}, { open_mpi_wl_block1, 45000}, { memory_wl_block, 40},
	 { NULL, 0 } }; //1.105.000

wl_block_t complete_wl_block[2] = { { 0,4000000 }, { 0,0 } };
vMacroblock_t complete_wl[2] = { {complete_wl_block, 30000}, { NULL, 0 } };



const vSeqblock_t Workload[N_HPC_CORE][N_WL_SEQ] = { { {.block_name=init_wl, .times=1500}, {.block_name=maxx_wl, .times=27000}, {.block_name=openmpi1, .times=4}, {.block_name=openmpi2, .times=4},
												{.block_name=mix1_phase, .times=8000}, {.block_name=mix2_phase, .times=8000}, {.block_name=mix3_phase, .times=8000}, {.block_name=complete_wl, .times=10} },
//1
											    { {.block_name=init_wl, .times=1500}, {.block_name=maxx_wl, .times=27000}, {.block_name=openmpi1, .times=4}, {.block_name=openmpi2, .times=4},
												{.block_name=mix2_phase, .times=6000}, {.block_name=mix1_phase, .times=8000}, {.block_name=mix3_phase, .times=10000}, {.block_name=complete_wl, .times=10} },
//2
												{ {.block_name=init_wl, .times=1500}, {.block_name=maxx_wl, .times=27000}, {.block_name=openmpi1, .times=4}, {.block_name=openmpi2, .times=4},
												{.block_name=mix3_phase, .times=8000}, {.block_name=mix2_phase, .times=8000}, {.block_name=mix1_phase, .times=8000}, {.block_name=complete_wl, .times=10} },
//3
												{ {.block_name=init_wl, .times=1500}, {.block_name=maxx_wl, .times=27000}, {.block_name=openmpi1, .times=4}, {.block_name=openmpi2, .times=4},
												{.block_name=mix2_phase, .times=10000}, {.block_name=mix3_phase, .times=6000}, {.block_name=mix1_phase, .times=8000}, {.block_name=complete_wl, .times=10} },
//4
												{ {.block_name=init_wl, .times=1500}, {.block_name=maxx_wl, .times=27000}, {.block_name=openmpi1, .times=4}, {.block_name=openmpi2, .times=4},
												{.block_name=mix1_phase, .times=8000}, {.block_name=mix2_phase, .times=8000}, {.block_name=mix1_phase, .times=8000}, {.block_name=complete_wl, .times=10} },
//5
												{ {.block_name=init_wl, .times=1500}, {.block_name=maxx_wl, .times=27000}, {.block_name=openmpi1, .times=4}, {.block_name=openmpi2, .times=4},
												{.block_name=mix1_phase, .times=8000}, {.block_name=mix2_phase, .times=8000}, {.block_name=mix3_phase, .times=8000}, {.block_name=complete_wl, .times=10} },
//6
												{ {.block_name=init_wl, .times=1500}, {.block_name=maxx_wl, .times=27000}, {.block_name=openmpi1, .times=4}, {.block_name=openmpi2, .times=4},
												{.block_name=mix3_phase, .times=4000}, {.block_name=mix2_phase, .times=10000}, {.block_name=mix1_phase, .times=10000}, {.block_name=complete_wl, .times=10} },
//7
												{ {.block_name=init_wl, .times=1500}, {.block_name=maxx_wl, .times=27000}, {.block_name=openmpi1, .times=4}, {.block_name=openmpi2, .times=4},
												{.block_name=mix2_phase, .times=8000}, {.block_name=mix1_phase, .times=8000}, {.block_name=mix3_phase, .times=8000}, {.block_name=complete_wl, .times=10} },
//8
												{ {.block_name=init_wl, .times=1500}, {.block_name=maxx_wl, .times=27000}, {.block_name=openmpi1, .times=4}, {.block_name=openmpi2, .times=4},
												{.block_name=mix1_phase, .times=10000}, {.block_name=mix2_phase, .times=8000}, {.block_name=mix3_phase, .times=6000}, {.block_name=complete_wl, .times=10} },
//9
												{ {.block_name=init_wl, .times=1500}, {.block_name=maxx_wl, .times=27000}, {.block_name=openmpi1, .times=4}, {.block_name=openmpi2, .times=4},
												{.block_name=mix1_phase, .times=8000}, {.block_name=mix3_phase, .times=8000}, {.block_name=mix1_phase, .times=8000}, {.block_name=complete_wl, .times=10} },
//10
												{ {.block_name=init_wl, .times=1500}, {.block_name=maxx_wl, .times=27000}, {.block_name=openmpi1, .times=4}, {.block_name=openmpi2, .times=4},
												{.block_name=mix3_phase, .times=8000}, {.block_name=mix2_phase, .times=8000}, {.block_name=mix1_phase, .times=8000}, {.block_name=complete_wl, .times=10} },
//11
												{ {.block_name=init_wl, .times=1500}, {.block_name=maxx_wl, .times=27000}, {.block_name=openmpi1, .times=4}, {.block_name=openmpi2, .times=4},
												{.block_name=mix2_phase, .times=8000}, {.block_name=mix1_phase, .times=8000}, {.block_name=mix3_phase, .times=8000}, {.block_name=complete_wl, .times=10} },
//12
												{ {.block_name=init_wl, .times=1500}, {.block_name=maxx_wl, .times=27000}, {.block_name=openmpi1, .times=4}, {.block_name=openmpi2, .times=4},
												{.block_name=mix1_phase, .times=8000}, {.block_name=mix2_phase, .times=8000}, {.block_name=mix1_phase, .times=8000}, {.block_name=complete_wl, .times=10} },
//13
												{ {.block_name=init_wl, .times=1500}, {.block_name=maxx_wl, .times=27000}, {.block_name=openmpi1, .times=4}, {.block_name=openmpi2, .times=4},
												{.block_name=mix1_phase, .times=7000}, {.block_name=mix2_phase, .times=5000}, {.block_name=mix3_phase, .times=8000}, {.block_name=complete_wl, .times=10} },
//14
												{ {.block_name=init_wl, .times=1500}, {.block_name=maxx_wl, .times=27000}, {.block_name=openmpi1, .times=4}, {.block_name=openmpi2, .times=4},
												{.block_name=mix2_phase, .times=8000}, {.block_name=mix1_phase, .times=8000}, {.block_name=mix3_phase, .times=8000}, {.block_name=complete_wl, .times=10} },
//15
												{ {.block_name=init_wl, .times=1500}, {.block_name=maxx_wl, .times=27000}, {.block_name=openmpi1, .times=4}, {.block_name=openmpi2, .times=4},
												{.block_name=mix2_phase, .times=4000}, {.block_name=mix3_phase, .times=10000}, {.block_name=mix1_phase, .times=10000}, {.block_name=complete_wl, .times=10} },
//16
												{ {.block_name=init_wl, .times=1500}, {.block_name=maxx_wl, .times=27000}, {.block_name=openmpi1, .times=4}, {.block_name=openmpi2, .times=4},
												{.block_name=mix1_phase, .times=8000}, {.block_name=mix2_phase, .times=8000}, {.block_name=mix3_phase, .times=8000}, {.block_name=complete_wl, .times=10} },
//17
											    { {.block_name=init_wl, .times=1500}, {.block_name=maxx_wl, .times=27000}, {.block_name=openmpi1, .times=4}, {.block_name=openmpi2, .times=4},
												{.block_name=mix2_phase, .times=6000}, {.block_name=mix1_phase, .times=8000}, {.block_name=mix3_phase, .times=10000}, {.block_name=complete_wl, .times=10} },
//18
												{ {.block_name=init_wl, .times=1500}, {.block_name=maxx_wl, .times=27000}, {.block_name=openmpi1, .times=4}, {.block_name=openmpi2, .times=4},
												{.block_name=mix3_phase, .times=8000}, {.block_name=mix2_phase, .times=8000}, {.block_name=mix1_phase, .times=8000}, {.block_name=complete_wl, .times=10} },
//19
												{ {.block_name=init_wl, .times=1500}, {.block_name=maxx_wl, .times=27000}, {.block_name=openmpi1, .times=4}, {.block_name=openmpi2, .times=4},
												{.block_name=mix2_phase, .times=10000}, {.block_name=mix3_phase, .times=6000}, {.block_name=mix1_phase, .times=8000}, {.block_name=complete_wl, .times=10} },
//20
												{ {.block_name=init_wl, .times=1500}, {.block_name=maxx_wl, .times=27000}, {.block_name=openmpi1, .times=4}, {.block_name=openmpi2, .times=4},
												{.block_name=mix1_phase, .times=8000}, {.block_name=mix2_phase, .times=8000}, {.block_name=mix1_phase, .times=8000}, {.block_name=complete_wl, .times=10} },
//21
												{ {.block_name=init_wl, .times=1500}, {.block_name=maxx_wl, .times=27000}, {.block_name=openmpi1, .times=4}, {.block_name=openmpi2, .times=4},
												{.block_name=mix1_phase, .times=8000}, {.block_name=mix2_phase, .times=8000}, {.block_name=mix3_phase, .times=8000}, {.block_name=complete_wl, .times=10} },
//22
												{ {.block_name=init_wl, .times=1500}, {.block_name=maxx_wl, .times=27000}, {.block_name=openmpi1, .times=4}, {.block_name=openmpi2, .times=4},
												{.block_name=mix3_phase, .times=4000}, {.block_name=mix2_phase, .times=10000}, {.block_name=mix1_phase, .times=10000}, {.block_name=complete_wl, .times=10} },
//23
												{ {.block_name=init_wl, .times=1500}, {.block_name=maxx_wl, .times=27000}, {.block_name=openmpi1, .times=4}, {.block_name=openmpi2, .times=4},
												{.block_name=mix2_phase, .times=8000}, {.block_name=mix1_phase, .times=8000}, {.block_name=mix3_phase, .times=8000}, {.block_name=complete_wl, .times=10} },
//24
												{ {.block_name=init_wl, .times=1500}, {.block_name=maxx_wl, .times=27000}, {.block_name=openmpi1, .times=4}, {.block_name=openmpi2, .times=4},
												{.block_name=mix1_phase, .times=10000}, {.block_name=mix2_phase, .times=8000}, {.block_name=mix3_phase, .times=6000}, {.block_name=complete_wl, .times=10} },
//25
												{ {.block_name=init_wl, .times=1500}, {.block_name=maxx_wl, .times=27000}, {.block_name=openmpi1, .times=4}, {.block_name=openmpi2, .times=4},
												{.block_name=mix1_phase, .times=8000}, {.block_name=mix3_phase, .times=8000}, {.block_name=mix1_phase, .times=8000}, {.block_name=complete_wl, .times=10} },
//26
												{ {.block_name=init_wl, .times=1500}, {.block_name=maxx_wl, .times=27000}, {.block_name=openmpi1, .times=4}, {.block_name=openmpi2, .times=4},
												{.block_name=mix3_phase, .times=8000}, {.block_name=mix2_phase, .times=8000}, {.block_name=mix1_phase, .times=8000}, {.block_name=complete_wl, .times=10} },
//27
												{ {.block_name=init_wl, .times=1500}, {.block_name=maxx_wl, .times=27000}, {.block_name=openmpi1, .times=4}, {.block_name=openmpi2, .times=4},
												{.block_name=mix2_phase, .times=8000}, {.block_name=mix1_phase, .times=8000}, {.block_name=mix3_phase, .times=8000}, {.block_name=complete_wl, .times=10} },
//28
												{ {.block_name=init_wl, .times=1500}, {.block_name=maxx_wl, .times=27000}, {.block_name=openmpi1, .times=4}, {.block_name=openmpi2, .times=4},
												{.block_name=mix1_phase, .times=8000}, {.block_name=mix2_phase, .times=8000}, {.block_name=mix1_phase, .times=8000}, {.block_name=complete_wl, .times=10} },
//29
												{ {.block_name=init_wl, .times=1500}, {.block_name=maxx_wl, .times=27000}, {.block_name=openmpi1, .times=4}, {.block_name=openmpi2, .times=4},
												{.block_name=mix1_phase, .times=7000}, {.block_name=mix2_phase, .times=5000}, {.block_name=mix3_phase, .times=8000}, {.block_name=complete_wl, .times=10} },
//30
												{ {.block_name=init_wl, .times=1500}, {.block_name=maxx_wl, .times=27000}, {.block_name=openmpi1, .times=4}, {.block_name=openmpi2, .times=4},
												{.block_name=mix2_phase, .times=8000}, {.block_name=mix1_phase, .times=8000}, {.block_name=mix3_phase, .times=8000}, {.block_name=complete_wl, .times=10} },
//31
												{ {.block_name=init_wl, .times=1500}, {.block_name=maxx_wl, .times=27000}, {.block_name=openmpi1, .times=4}, {.block_name=openmpi2, .times=4},
												{.block_name=mix2_phase, .times=4000}, {.block_name=mix3_phase, .times=10000}, {.block_name=mix1_phase, .times=10000}, {.block_name=complete_wl, .times=10} },
//32
												{ {.block_name=init_wl, .times=1500}, {.block_name=maxx_wl, .times=27000}, {.block_name=openmpi1, .times=4}, {.block_name=openmpi2, .times=4},
												{.block_name=mix2_phase, .times=4000}, {.block_name=mix3_phase, .times=10000}, {.block_name=mix1_phase, .times=10000}, {.block_name=complete_wl, .times=10} },
//33
												{ {.block_name=init_wl, .times=1500}, {.block_name=maxx_wl, .times=27000}, {.block_name=openmpi1, .times=4}, {.block_name=openmpi2, .times=4},
												{.block_name=mix2_phase, .times=4000}, {.block_name=mix3_phase, .times=10000}, {.block_name=mix1_phase, .times=10000}, {.block_name=complete_wl, .times=10} },
//34
												{ {.block_name=init_wl, .times=1500}, {.block_name=maxx_wl, .times=27000}, {.block_name=openmpi1, .times=4}, {.block_name=openmpi2, .times=4},
												{.block_name=mix2_phase, .times=4000}, {.block_name=mix1_phase, .times=10000}, {.block_name=mix3_phase, .times=10000}, {.block_name=complete_wl, .times=10} },
//35
												{ {.block_name=init_wl, .times=1500}, {.block_name=maxx_wl, .times=27000}, {.block_name=openmpi1, .times=4}, {.block_name=openmpi2, .times=4},
												{.block_name=mix1_phase, .times=6000}, {.block_name=mix3_phase, .times=8000}, {.block_name=mix1_phase, .times=10000}, {.block_name=complete_wl, .times=10} }
//36
											 };

/*** WL var ***/
//int wl_vs_cycles[N_HPC_CORE]; //core_freq_in_MHz[core] / (Timer_measured_us / number_of_steps);
//
uint32_t block_counter[N_HPC_CORE] = { 0 };
uint32_t macroblock_counter[N_HPC_CORE] = { 0 };
uint32_t sequence_counter[N_HPC_CORE] = { 0 };
uint32_t wl_counter[N_HPC_CORE] = { 0 };
int block_exec_times[N_HPC_CORE];
//
int cycles_surplus[N_HPC_CORE] = { 0 };
uint32_t  state_surplus[N_HPC_CORE] = { 0 };

int global_finished[N_HPC_CORE];
//
//
vMacroblock_t* macroblock_address;

//TODO:
int workload_times_divider = 3;


/***** Initialization *****/
void initWlTransl(void)
{
	for (int i = 0; i<N_HPC_CORE; i++)
	{
		block_exec_times[i] = 1;

		sequence_counter[i] = 0;
		macroblock_counter[i] = 0;
		block_counter[i] = 0;
		wl_counter[i] = 0;
		block_exec_times[i] = 0;
		cycles_surplus[i] = 0;

		global_finished[i] = 0;
	}
}

uint32_t mem_cycles = 0.4*1000000.0f/steps_per_sim_time_ms;

void execWlTransl(uint32_t* icycles_per_step)
{
	int accum_cycles[WL_STATES];
	int total_cycles = 0;
	//
	//cycles_per_step = how many CORE cycles (Hz) there are in 1 execution of
    //cycles_per_step = F(Hz) / step_per_sim_time;
    //if step_per_sim_time_ms --> F(KHz).
    uint32_t core_freq_in_KHz;
    uint32_t cycles_per_step;

	int element_counter = 0;

	for (int core = 0; core<N_HPC_CORE; core++)
	{
		core_freq_in_KHz = (uint32_t)(ifp_ctrl_core_freq[core] * 1000000.0f);
    	cycles_per_step = core_freq_in_KHz / steps_per_sim_time_ms;
		icycles_per_step[core] = cycles_per_step;

		int l_wl_states = simulation.elements[core].core_config.wl_states;

		total_cycles = 0;
		macroblock_address = Workload[core][wl_counter[core]].block_name;
		//This is an additional check, and should not be needed:
		/*
		if (macroblock_address == NULL)
		{
			wl_counter[core] = 0;
		}
		if (macroblock_address[macroblock_counter[core]].block_name == NULL)
		{
			wl_counter[core]++;
			if (wl_counter[core] >= N_WL_SEQ)
				wl_counter[core] = 0;
			else {
				if (Workload[core][wl_counter[core]].block_name == NULL)
					wl_counter[core] = 0;
			}
			block_counter[core] = 0;
			block_exec_times[core] = 1;
			macroblock_counter[core] = 0;

			macroblock_address = Workload[core][wl_counter[core]].block_name;
		}
		*/

		for (int i = 0; i<l_wl_states; i++)
			accum_cycles[i] = 0;

		//Re-add the surplus
		if (cycles_surplus[core] > 0)
		{
			total_cycles += cycles_surplus[core];
			accum_cycles[state_surplus[core]] += cycles_surplus[core];

			cycles_surplus[core] = total_cycles - cycles_per_step;

			if (cycles_surplus[core] > 0) //aka (total_cycles >= cycles_per_step)
			{
				accum_cycles[state_surplus[core]] -= cycles_surplus[core];
			}
		}

		while (total_cycles < cycles_per_step)
		{
			//Take infos
			int cycles = macroblock_address[macroblock_counter[core]].block_name[block_counter[core]].cycles;
			//Change wlType/Block/Macroblock:
			if (cycles <= 0)
			{
				block_counter[core] = 0;

				if (macroblock_address[macroblock_counter[core]].times > block_exec_times[core])
				{
					block_exec_times[core]++;
				}
				else
				{
					block_exec_times[core] = 1;
					macroblock_counter[core]++;
					if (macroblock_address[macroblock_counter[core]].block_name == NULL)
					{
						macroblock_counter[core] = 0;

						sequence_counter[core]+=workload_times_divider;

						if (sequence_counter[core] >= Workload[core][wl_counter[core]].times)
						{
							sequence_counter[core] = 0;

							wl_counter[core]++;

							//TODO CANC:
							if (wl_counter[core] == N_WL_SEQ-1)
								global_finished[core] = 1;

							if (wl_counter[core] >= N_WL_SEQ)
								wl_counter[core] = 0;
							else {
								if (Workload[core][wl_counter[core]].block_name == NULL)
									wl_counter[core] = 0;
							}
						}

						
						macroblock_address = Workload[core][wl_counter[core]].block_name;
					}
				}

				//take new values
				cycles = macroblock_address[macroblock_counter[core]].block_name[block_counter[core]].cycles;
			}
			uint32_t wl_state = macroblock_address[macroblock_counter[core]].block_name[block_counter[core]].state;

			//TO consider memory
			if ((wl_state == 0))
			{
				cycles = cycles * icycles_per_step[core] / mem_cycles *40/100; 
			}
			else if ((wl_state == 4))
			{
				cycles = cycles*75/100 + icycles_per_step[core]*20/mem_cycles/100; 
			}

			total_cycles += cycles;
			block_counter[core]++;
			accum_cycles[wl_state] += cycles;

			//Check for the cycles Overflow
			if (total_cycles > cycles_per_step)
			{
				cycles_surplus[core] = total_cycles - cycles_per_step;
				state_surplus[core] = wl_state;

				accum_cycles[wl_state] -= cycles_surplus[core];
			}

		}

		for (int i = 0; i < l_wl_states; i++)
		{
			simulation.elements[core].core_config.workload[i] = (float)accum_cycles[i] / (float)cycles_per_step;
		}

		//TODO: remove additional check
		float summa = 0.0f;
		for (int i = 0; i < l_wl_states; i++)
		{
			summa += simulation.elements[core].core_config.workload[i];
		}
		if (summa < 0.98f)
		{
			printf("Error Instructions simulation, sum is %d / 100\n\r", (uint32_t)(summa*100.0f));
			float diff = 1.0f - summa;
			for (int i = 0; i < l_wl_states; i++)
			{
				simulation.elements[core].core_config.workload[i] += diff/(float)l_wl_states;
			}
		}
		/*
		if (i_instr_count != NULL)
		{
			for (int i = 0; i < l_wl_states; i++)
			{
				i_instr_count[core][i] = accum_cycles[i];
			}
		}
		*/

	} //for core

	return;
} //function
