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

#include "perf_model.h"

#include "sim_config.h"

#include "cmdconf.h"

//Standard Lib
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h>

//
#include <float.h>
#include <math.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>

//#include "hdf5.h"

//TODO:
uint32_t quantum_dim = 800;
uint32_t quanta_finish_and_restart = 0;
struct quantum_st** idle_quantum = NULL;

uint32_t *request_changing_freq = NULL;
uint32_t *request_changing_pw_budget = NULL;
uint32_t *request_changing_voltage = NULL;
uint32_t *request_changing_bindings = NULL;

//internal functions
static inline int move_current_pointer(int elem_counter);
void create_idle_quantum(void);

#define float_apprx_err 0.01f //0.01%
float float_apprx_coeff = 1.0f-(float_apprx_err / 100.0f);

void init_quanta(void)
{
    head_quantum = (struct quantum_st**)malloc(sizeof(struct quantum_st*)*simulation.nb_elements);
    current_quantum = (struct quantum_st**)malloc(sizeof(struct quantum_st*)*simulation.nb_elements);
    last_quantum = (struct quantum_st**)malloc(sizeof(struct quantum_st*)*simulation.nb_elements);
    if ( (head_quantum == NULL) || (current_quantum == NULL) || (last_quantum == NULL) )
    {
        //TODO
        //perror
        exit(1);
    }

    //init the global pointers
    for (int e=0; e<simulation.nb_elements; e++)
    {
        struct quantum_st *ptr = NULL;
        ptr = allocate_quantum(&simulation.elements[e]);

        //populate pointers
        ptr->prev = NULL; //or? = last_quantum[e];
        ptr->next = NULL;
        ptr->nextit = NULL;

        //update global pointers
        last_quantum[e] = ptr;
        current_quantum[e] = ptr;
        head_quantum[e] = ptr;

        int l_states_num = 0;
        //check which component it is
        l_states_num = get_component_states_number(&simulation.elements[e]);

        if (l_states_num <= 0)
        {
            //TODO
        }

        //populate data
        ptr->us_wait_time        = 0;
        ptr->us_waited_time        = 0;
        ptr->change_freq         = 0;
        ptr->change_pw_budget    = 0;
        ptr->change_voltage      = 0;
        ptr->change_bindings     = 0;
        ptr->barrier_synch       = 0;
        ptr->ifiter              = 0;

        for (int state=0; state<l_states_num; state++)
        {
            ptr->wl_perc[state] = 0;
        }
        ptr->wl_perc[0] = 1;
        ptr->cpi_freq = 1.0;
        ptr->dim = quantum_dim;
        ptr->consumed = 0;
    }

    //create IDLE quantum:
    create_idle_quantum();

    request_changing_freq = calloc(simulation.nb_elements, sizeof(uint32_t));
    request_changing_pw_budget = calloc(simulation.nb_elements, sizeof(uint32_t));
    request_changing_voltage = calloc(simulation.nb_elements, sizeof(uint32_t));
    request_changing_bindings = calloc(simulation.nb_elements, sizeof(uint32_t));

}


struct quantum_st* allocate_quantum(struct element_st *element)
{
    struct quantum_st *ptr = NULL;
    ptr = (struct quantum_st*)malloc(sizeof(struct quantum_st));
    if (ptr == NULL)
    {
        //TODO
        //perror
        exit(1);
    }

    ptr->wl_perc = NULL;
    ptr->wl_perc = (float*)malloc(sizeof(float)*get_component_states_number(element));
    if (ptr->wl_perc == NULL)
    {
        //TODO
        //perror
        exit(1);
    }

    //else if
    //TODO
    //else

    return ptr;
}

void dealloc_quantum(struct quantum_st* ptr)
{
    free(ptr->wl_perc);
    free(ptr);
}

void create_quantum(struct element_st *element, int elem_counter)
{
    struct quantum_st *ptr = NULL;
    ptr = allocate_quantum(element);

    //populate pointers
    ptr->prev = last_quantum[elem_counter];
    last_quantum[elem_counter]->next = ptr;
    ptr->next = NULL;
    ptr->nextit = NULL;

    //update global pointers
    last_quantum[elem_counter] = ptr;

    int l_states_num = 0;
    //check which component it is
    l_states_num = get_component_states_number(element);

    if (l_states_num <= 0)
    {
        //TODO
    }

    //populate data
    ptr->us_wait_time        = 0;
    ptr->us_waited_time      = 0;
    ptr->change_freq         = 0;
    ptr->change_pw_budget    = 0;
    ptr->change_voltage      = 0;
    ptr->change_bindings     = 0;
    ptr->barrier_synch       = 0;
    ptr->ifiter              = 0;

    for (int state=0; state<l_states_num; state++)
    {
        ptr->wl_perc[state] = 1.0f/(float)l_states_num;
    }

    ptr->cpi_freq = 1.0;
    ptr->dim = quantum_dim;
    ptr->consumed = 0;
}

int generate_wl(float *owl, float ifreq, int elem_counter, struct element_st *element)
{
    int return_value = 0;

	float total_cycles = 0;
	//
	float core_freq_in_KHz = (ifreq * 1000000.0f);
    float cycles_per_step = core_freq_in_KHz / (float)steps_per_sim_time_ms;

    int l_states_num = 0;
    //check which component it is
    l_states_num = get_component_states_number(element);

    if (l_states_num <= 0)
    {
        //TODO
        return_value = 1;
    }

    float *accum_instr = calloc(l_states_num, sizeof(float));
    if (accum_instr == NULL)
    {
        //TODO
        return_value = 1;
    }

    //main
    while (total_cycles < float_apprx_coeff*cycles_per_step)
    {
        /* First manage the waiting time */
        if ( (current_quantum[elem_counter]->us_wait_time > 0) && (current_quantum[elem_counter]->us_wait_time - current_quantum[elem_counter]->us_waited_time >= 0) )
        {
            // Convert cycles in time
            float time_us = (cycles_per_step-total_cycles) / (ifreq * 1000.0f);
            time_us = fminf(time_us, current_quantum[elem_counter]->us_wait_time - current_quantum[elem_counter]->us_waited_time);
            /*
            if (elem_counter == 0){
                printf("** Time: %f\n\r", time_us);
            }
            */

            // Check for numbers less than 1 (//TBC) - Saturate to 1 to avoid being stuck in the while loop
            //      Otherwise saturate to 0 to avoid nagatives
            time_us = (time_us > 0) ? fmaxf(time_us, 1.0f) : 0;
            current_quantum[elem_counter]->us_waited_time += time_us;

            //Convert back to cycles
            float cycles = time_us*(ifreq * 1000.0f);
            accum_instr[WAIT_STATE] += cycles;
            total_cycles += cycles;
        }

        float cycles = fminf(((float)(current_quantum[elem_counter]->dim - current_quantum[elem_counter]->consumed)*current_quantum[elem_counter]->cpi_freq), 
                        (cycles_per_step-total_cycles));
        // Check for numbers less than 1 (//TBC) - Saturate to 1 to avoid being stuck in the while loop
        //      Otherwise saturate to 0 to avoid nagatives
        cycles = (cycles > 0) ? fmaxf(cycles, 1.0f) : 0;
        for (int state=0; state<l_states_num; state++)
        {
            accum_instr[state] += cycles * current_quantum[elem_counter]->wl_perc[state];
        }
        total_cycles += cycles;
        current_quantum[elem_counter]->consumed += (int64_t)ceil(cycles/current_quantum[elem_counter]->cpi_freq); //TBC: ceil

        /*
        if (elem_counter == 0){
        printf("Tot Cycl in while: %f, consumed: %ld, added2cons: %d\n\r", total_cycles, 
                current_quantum[elem_counter]->consumed, (int)(ceil(cycles/current_quantum[elem_counter]->cpi_freq)));
        for(int s=0; s<get_component_states_number(&simulation.elements[elem_counter]); s++)
            printf("[%d]=%f , ", s, accum_instr[s]);
        printf("\n\r");
        }
        */

        //Move current
        move_current_pointer(elem_counter);
    }

    //generate owl
    for (int state=0; state<l_states_num; state++)
    {
        owl[state] = accum_instr[state] / cycles_per_step;
    }

    //Already Managed inside the move_current_pointer() function
    // if (surplus > 0)
    // {
    //     //move current back
    //     current_quantum[elem_counter] = current_quantum[elem_counter]->prev;
    // }

    //clean
    free(accum_instr);

    return return_value;
}

static inline int move_current_pointer(int elem_counter)
{
    int return_value = 0;

    if ( (current_quantum[elem_counter]->consumed - current_quantum[elem_counter]->dim >= 0) &&
        (current_quantum[elem_counter]->us_waited_time >= float_apprx_coeff*current_quantum[elem_counter]->us_wait_time) )
    {
        if (current_quantum[elem_counter]->change_freq)
        {
            request_changing_freq[elem_counter] = 1;
        }
        if (current_quantum[elem_counter]->change_pw_budget)
        {
            request_changing_pw_budget[elem_counter] = 1;
        }
        if (current_quantum[elem_counter]->change_voltage)
        {
            request_changing_voltage[elem_counter] = 1;
        }
        if (current_quantum[elem_counter]->change_bindings)
        {
            request_changing_bindings[elem_counter] = 1;
        }

        // Zeroing in case we pass again from this Quantum
        current_quantum[elem_counter]->consumed = 0;
        current_quantum[elem_counter]->us_waited_time = 0;

        //
        current_quantum[elem_counter]->ifiter--;
        if ((current_quantum[elem_counter]->ifiter < 0) //strict because I put -- before
            || (current_quantum[elem_counter]->nextit == NULL))
        {
            if (current_quantum[elem_counter]->next != NULL)
            {
                current_quantum[elem_counter] = current_quantum[elem_counter]->next;

                //TBD: Reset before using the Quantum
                current_quantum[elem_counter]->consumed = 0;
                current_quantum[elem_counter]->us_waited_time = 0;

                /*
                if (elem_counter == 0){
                    printf("... ... Changing quantum ... ...\n\r");   
                }
                */
            }
            else //the end
            {
                return_value = 1;

                printf("Finished application element: %d!\n\r", elem_counter);

                //A: finish populating with idle, and break + keep idling
                //--B: finish populating with idle, and break + restart
                //C: restart.

                if (quanta_finish_and_restart)
                {
                    current_quantum[elem_counter] = head_quantum[elem_counter];
                }
                else
                {
                    current_quantum[elem_counter] = idle_quantum[elem_counter];
                }
            }
        }
        else
        {
            current_quantum[elem_counter] = current_quantum[elem_counter]->nextit;
        }
    }
    // else //if (surminus < 0) --> Do nothing
    // {
    //     current_quantum[elem_counter]->consumed += floor();
    // }

    return return_value;
}

int get_component_states_number(struct element_st *element)
{
    int n_states = 0;

    if (element->type == JT_CORE)
    {
        n_states = element->core_config.wl_states;
    }

    return n_states;
}

void create_idle_quantum(void)
{
    idle_quantum = (struct quantum_st**)malloc(sizeof(struct quantum_st*)*simulation.nb_elements);

    for (int e=0; e<simulation.nb_elements; e++)
    {
        struct quantum_st *ptr = NULL;
        ptr = allocate_quantum(&simulation.elements[e]);

        //populate pointers
        ptr->prev = ptr; //itself
        ptr->next = ptr;
        ptr->nextit = ptr;

        idle_quantum[e] = ptr;

        int l_states_num = 0;
        //check which component it is
        l_states_num = get_component_states_number(&simulation.elements[e]);

        if (l_states_num <= 0)
        {
            //TODO
        }

        //populate data
        ptr->us_wait_time        = 0;
        ptr->us_waited_time        = 0;
        ptr->change_freq         = 0;
        ptr->change_pw_budget    = 0;
        ptr->change_voltage      = 0;
        ptr->change_bindings     = 0;
        ptr->barrier_synch       = 0;
        ptr->ifiter              = 0;

        for (int state=0; state<l_states_num; state++)
        {
            ptr->wl_perc[state] = 0;
        }
        ptr->wl_perc[0] = 1.0f;
        ptr->cpi_freq = 1.0;
        ptr->dim = quantum_dim;
        ptr->consumed = 0;
    }    
}


// interface read_workload(char* h5_path) {
//     // char *datasets[] = {"power-pkg-0",
//     //                     "rank-0-cpu-14-freq",
//     //                     "rank-0-cpu-14-ipc",
//     //                     "rank-0-cpu-14-cycles",
//     //                     "rank-0-cpu-14-inst_ret",
//     //                     "rank-0-cpu-14-128b-packed_double_inst",
//     //                     "rank-0-cpu-14-128b-packed_single_inst",
//     //                     "rank-0-cpu-14-256b-packed_double_inst",
//     //                     "rank-0-cpu-14-256b-packed_single_inst",
//     //                     "rank-0-cpu-14-512b-packed_double_inst",
//     //                     "rank-0-cpu-14-512b-packed_single_inst",
//     //                     "rank-0-cpu-14-scalar_double_inst",
//     //                     "rank-0-cpu-14-scalar_single_inst"};
//     // hid_t dataset_types[] = {
//     //     H5T_NATIVE_FLOAT, H5T_NATIVE_FLOAT, H5T_NATIVE_FLOAT,
//     //     H5T_NATIVE_ULONG, H5T_NATIVE_ULONG, H5T_NATIVE_ULONG,
//     //     H5T_NATIVE_ULONG, H5T_NATIVE_ULONG, H5T_NATIVE_ULONG,
//     //     H5T_NATIVE_ULONG, H5T_NATIVE_ULONG, H5T_NATIVE_ULONG,
//     //     H5T_NATIVE_ULONG};
//     hid_t file_id, dataset_id; /* identifiers */
//     interface res;
//     // int num_samples = 0;
//     // int num_datasets = sizeof(datasets) / sizeof(char *);

//     file_id = H5Fopen(h5_path /*"../fpinstructions.h5"*/, H5F_ACC_RDONLY, H5P_DEFAULT);

//     dataset_id = H5Dopen(file_id, "time-sample", H5P_DEFAULT);
//     hid_t filespace = H5Dget_space(dataset_id);
//     hsize_t dims[1];
//     H5Sget_simple_extent_dims(filespace, dims, NULL);

//     float float_data[dims[0]];
//     unsigned long ulong_data[dims[0]];

//     H5Dread(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT,
//             float_data);
//     H5Dclose(dataset_id);

//     // Consider the sampling period (~ 1 second) as total time
//     res.time = float_data[0];

//     dataset_id = H5Dopen(file_id, "power-pkg-0", H5P_DEFAULT);
//     H5Dread(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT,
//             float_data);
//     H5Dclose(dataset_id);

//     res.power_pkg = float_data[0];

//     dataset_id = H5Dopen(file_id, "rank-0-cpu-14-freq", H5P_DEFAULT);
//     H5Dread(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT,
//             float_data);
//     H5Dclose(dataset_id);

//     res.cpu_freq = float_data[0];

//     dataset_id = H5Dopen(file_id, "rank-0-cpu-14-ipc", H5P_DEFAULT);
//     H5Dread(dataset_id, H5T_NATIVE_FLOAT, H5S_ALL, H5S_ALL, H5P_DEFAULT,
//             float_data);
//     H5Dclose(dataset_id);

//     res.cpu_ipc = float_data[0];

//     dataset_id = H5Dopen(file_id, "rank-0-cpu-14-cycles", H5P_DEFAULT);
//     H5Dread(dataset_id, H5T_NATIVE_ULONG, H5S_ALL, H5S_ALL, H5P_DEFAULT,
//             ulong_data);
//     H5Dclose(dataset_id);

//     res.cpu_cycles = ulong_data[0];

//     dataset_id = H5Dopen(file_id, "rank-0-cpu-14-inst_ret", H5P_DEFAULT);
//     H5Dread(dataset_id, H5T_NATIVE_ULONG, H5S_ALL, H5S_ALL, H5P_DEFAULT,
//             ulong_data);
//     H5Dclose(dataset_id);

//     res.cpu_inst_ret = ulong_data[0];

//     res.fp_ops = 0;
//     res.vect_ops = 0;

//     dataset_id =
//         H5Dopen(file_id, "rank-0-cpu-14-128b-packed_double_inst", H5P_DEFAULT);
//     H5Dread(dataset_id, H5T_NATIVE_ULONG, H5S_ALL, H5S_ALL, H5P_DEFAULT,
//             ulong_data);
//     H5Dclose(dataset_id);

//     res.fp_ops += ulong_data[0] * 4;
//     res.vect_ops += ulong_data[0];

//     dataset_id =
//         H5Dopen(file_id, "rank-0-cpu-14-128b-packed_single_inst", H5P_DEFAULT);
//     H5Dread(dataset_id, H5T_NATIVE_ULONG, H5S_ALL, H5S_ALL, H5P_DEFAULT,
//             ulong_data);
//     H5Dclose(dataset_id);

//     res.fp_ops += ulong_data[0] * 2;
//     res.vect_ops += ulong_data[0];

//     dataset_id =
//         H5Dopen(file_id, "rank-0-cpu-14-256b-packed_double_inst", H5P_DEFAULT);
//     H5Dread(dataset_id, H5T_NATIVE_ULONG, H5S_ALL, H5S_ALL, H5P_DEFAULT,
//             ulong_data);
//     H5Dclose(dataset_id);

//     res.fp_ops += ulong_data[0] * 8;
//     res.vect_ops += ulong_data[0];

//     dataset_id =
//         H5Dopen(file_id, "rank-0-cpu-14-256b-packed_single_inst", H5P_DEFAULT);
//     H5Dread(dataset_id, H5T_NATIVE_ULONG, H5S_ALL, H5S_ALL, H5P_DEFAULT,
//             ulong_data);
//     H5Dclose(dataset_id);

//     res.fp_ops += ulong_data[0] * 4;
//     res.vect_ops += ulong_data[0];

//     dataset_id =
//         H5Dopen(file_id, "rank-0-cpu-14-512b-packed_double_inst", H5P_DEFAULT);
//     H5Dread(dataset_id, H5T_NATIVE_ULONG, H5S_ALL, H5S_ALL, H5P_DEFAULT,
//             ulong_data);
//     H5Dclose(dataset_id);

//     res.fp_ops += ulong_data[0] * 16;
//     res.vect_ops += ulong_data[0];

//     dataset_id =
//         H5Dopen(file_id, "rank-0-cpu-14-512b-packed_single_inst", H5P_DEFAULT);
//     H5Dread(dataset_id, H5T_NATIVE_ULONG, H5S_ALL, H5S_ALL, H5P_DEFAULT,
//             ulong_data);
//     H5Dclose(dataset_id);

//     res.fp_ops += ulong_data[0] * 8;
//     res.vect_ops += ulong_data[0];

//     dataset_id =
//         H5Dopen(file_id, "rank-0-cpu-14-scalar_double_inst", H5P_DEFAULT);
//     H5Dread(dataset_id, H5T_NATIVE_ULONG, H5S_ALL, H5S_ALL, H5P_DEFAULT,
//             ulong_data);
//     H5Dclose(dataset_id);

//     res.fp_ops += ulong_data[0] * 2;

//     dataset_id =
//         H5Dopen(file_id, "rank-0-cpu-14-scalar_single_inst", H5P_DEFAULT);
//     H5Dread(dataset_id, H5T_NATIVE_ULONG, H5S_ALL, H5S_ALL, H5P_DEFAULT,
//             ulong_data);
//     H5Dclose(dataset_id);

//     res.fp_ops += ulong_data[0];

//     res.mem_ops = 0;
//     res.power_l2 = 0;
//     res.zero = 0;
//     res.maxpower_cpu = 0;
//     res.maxpower_l2 = 0;

//     H5Fclose(file_id);

//     return res;

        //DO NOT USE MEMCPY!! Convert to for
//     //memcpy(wl, &res, sizeof(res));
// }

/*int main(){
   interface wl;
   readWorkload(&wl);
   printf("cicli totali: %llu frequenza: %f inst_ret: %llu ipc: %f fp: %llu
maxPcpu: %d maxPl2: %d mem: %llu Pl2: %f Ppkg: %f time: %f zero: %u\n",
wl.cpu_cycles, wl.cpu_freq, wl.cpu_inst_ret, wl.cpu_ipc, wl.fp_ops,
wl.maxpower_cpu, wl.maxpower_l2, wl.mem_ops, wl.power_l2, wl.power_pkg, wl.time,
wl.zero); return 0;
}*/

/*
void trace_populate_offline(void)
{
    //int element_counter = 0;
    head = (struct quantum_st**)malloc(sizeof(struct quantum_st*)*simulation.nb_elements);
    current = (struct quantum_st**)malloc(sizeof(struct quantum_st*)*simulation.nb_elements);
    prev = (struct quantum_st**)malloc(sizeof(struct quantum_st*)*simulation.nb_elements);
    if ( (head == NULL) || (current == NULL) || (prev == NULL) )
    {
        //TODO
        //perror
        exit(1);
    }
    for (int i=0; i<simulation.nb_elements; i++)
    {
        head[i] = create_quanta(&simulation.elements[i]);
        head[i]->freq = init_freq;
        head[i]->wl_perc[0] = 100.0f;
        current[i] = head[i];
        prev[i] = head[i];
    }

    //populate:
    while (fd != NULL)
    {
        for (int i=0; i<simulation.nb_elements; i++)
        {
            struct quantum *ptr = NULL;
            ptr = create_quanta(&simulation.elements[i]);
            current[i] = ptr;
            prev[i]->next = current[i];
            current[i]->prev = prev[i];
            //current[i]->freq =
            //...
        }

    }
}
*/
