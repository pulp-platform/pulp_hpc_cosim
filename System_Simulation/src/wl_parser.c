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
#include "sim_config.h"
#include "perf_model.h"

//OS
#include "os_data.h"

//Govern

//other
#include "csv.h"

#define __max(a,b) \
    ({ __typeof__ (a) _a = (a); \
        __typeof__ (b) _b = (b); \
        _a > _b ? _a : _b; })

void parse_wl (void ) {

    char* row;
	/*const*/ char* col;
	int nb_cols, nb_rows, counter_cols, counter_rows;
	int prev_time;
	CsvHandle handle;

    int n_cores = 0;
    const int tr_base_cols = 11;
    const int tr_st_cols = 8+1;
    const int tr_nd_cols = 22;
    //const int tr_fp_num_b = 10;
    //const int tr_li_num_b = tr_base_cols-tr_fp_num_b;
    //const int tr_fp_num_p = 3;
    //const int tr_li_num_p = (tr_st_cols+tr_nd_cols) - tr_fp_num_p;
    const int fp_dp_vect_dim = 4;
    const int fp_sp_vect_dim = 4;
    const int fp_dp_skip_dim = fp_dp_vect_dim+1;
    const int fp_sp_skip_dim = fp_sp_vect_dim+1;

    char path[] = "System_Simulation/sym_inputs/wl_trace.csv";

	handle = CsvOpen2(path, ';', '"', '\\');
	//CsvHandle handle = CsvOpen2("System_Simulation/sym_inputs/target_freq.csv", ';', '"', '\'');
	if (!handle)
	{
		printf("[CSV] Error opening the Workload Trace\r\n");
 		//TODO, use default values
 		//exit(1);
	}

	/* read first row */
	row = CsvReadNextRow(handle);
	nb_cols = 0;
	while (col = CsvReadNextCol(row, handle))
	    {nb_cols++;}

    /**
     * 11 (Ts, E0, ED0, E1, ED1, P0, PD0, P1, PD1, #MPI_write, #MPI_read)
     * 1-N (app time %) - f
     * 2-N (mpi time %) - f
     * 3-N (mpi net send #) - i
     * 4-N (mpi net receive #) - i
     * 5-N (Avg Freq MHz) - i
     * 5-bis (Load) - f
     * 6-N (Avg IPC) - f
     * 7-N (Cycles) - li
     * 8-N (Instruction Retired) - li
     * Strided: - li
     *      FP-DP tot
     *      FP-DP 64
     *      FP-DP 128
     *      FP-DP 256
     *      FP-DP 512
     *      FP-DP tot (TE/TR)
     *      FP-DP 64 (TE/TR/TM)
     *      FP-DP 128 (TE/TR/TM)
     *      FP-DP 256 (TE/TR/TM)
     *      FP-DP 512 (TE/TR/TM)
     * (10)
     *      FP-SP tot
     *      FP-SP 32
     *      FP-DP 128
     *      FP-DP 256
     *      FP-DP 512
     *      FP-SP tot (TE/TR)
     *      FP-SP 32 (TE/TR/TM)
     *      FP-DP 128 (TE/TR/TM)
     *      FP-DP 256 (TE/TR/TM)
     *      FP-DP 512 (TE/TR/TM)
     * (10)
     *      MEM (TE/TR)
     *      MEM-DATA (bytes)
     * (2) 
     * 
     * 11 + N*8 + 22*N
    */

    n_cores = (nb_cols - tr_base_cols) / (tr_st_cols + tr_nd_cols);
    printf("nc: %d ncols: %d, res: -%d / %d\n", n_cores, nb_cols, tr_base_cols, (tr_st_cols + tr_nd_cols));

	/* check number of rows */
	nb_rows = 0;
	while (row = CsvReadNextRow(handle))
		{nb_rows++;}

	// int64_t* tr_li_data = (int64_t*)malloc(sizeof(int64_t)*n_cores*nb_rows*WL_STATES); //TODO Check this! is it nb_* or simulation_elements..?

	/* Reset file */
	CsvClose(handle);

    /* data reading */
	/* Re-Open file */
	handle = CsvOpen2(path, ';', '"', '\\');
	//CsvHandle handle = CsvOpen2("System_Simulation/sym_inputs/target_freq.csv", ';', '"', '\'');
	if (!handle)
	{
		printf("[CSV] Error opening the Workload Trace file (second time)\r\n");
 		//TODO, use default values
 		//exit(1);
	}

	/* skip first row */
	row = CsvReadNextRow(handle);

	counter_rows = 0;
	prev_time = 0;
    float* app_time = (float *)malloc(sizeof(float)*n_cores);
    float* mpi_time = (float *)malloc(sizeof(float)*n_cores);
    float* load = (float *)malloc(sizeof(float)*n_cores);
    float* cpi = (float *)malloc(sizeof(float)*n_cores);
    int64_t* tot_cycles = (int64_t*)malloc(sizeof(int64_t)*n_cores);
    int64_t* istr_ret = (int64_t*)malloc(sizeof(int64_t)*n_cores);
    int64_t* fpdp_cycles = (int64_t*)malloc(sizeof(int64_t)*fp_dp_vect_dim*n_cores);
    int64_t* fpsp_cycles = (int64_t*)malloc(sizeof(int64_t)*fp_sp_vect_dim*n_cores);
    int64_t* mem_cycles = (int64_t*)malloc(sizeof(int64_t)*n_cores);

    int64_t* idle_c = (int64_t*)malloc(sizeof(int64_t)*n_cores);
    int64_t* int_c = (int64_t*)malloc(sizeof(int64_t)*n_cores);
    int64_t* float_c = (int64_t*)malloc(sizeof(int64_t)*n_cores);
    int64_t* l2mem_c = (int64_t*)malloc(sizeof(int64_t)*n_cores);
    int64_t* vect_c = (int64_t*)malloc(sizeof(int64_t)*n_cores);   

	while (row = CsvReadNextRow(handle))
	{
	    /* row = CSV row string */
	    const char* col;
	    counter_cols = 0;

	    /* skip first tr_base_cols */
        for (int idx=0;idx<tr_base_cols;idx++){
            col = CsvReadNextCol(row, handle);
        }

        /* Read App and MPI time */
        for (int idx=0;idx<n_cores;idx++){
            col = CsvReadNextCol(row, handle);
            app_time[idx] = atof(col);
        }
        for (int idx=0;idx<n_cores;idx++){
            col = CsvReadNextCol(row, handle);
            mpi_time[idx] = atof(col);
        }

        /* Skip 3*N data*/
        for (int idx=0;idx<3*n_cores;idx++){
            col = CsvReadNextCol(row, handle);
        }

        /* Read Load */
        for (int idx=0;idx<n_cores;idx++){
            col = CsvReadNextCol(row, handle);
            load[idx] = atof(col);
        }

        /* Read IPC/CPI */
        for (int idx=0;idx<n_cores;idx++){
            col = CsvReadNextCol(row, handle);
            cpi[idx] = 1/atof(col);
        }

        /* Read tot cycles */
        for (int idx=0;idx<n_cores;idx++){
            col = CsvReadNextCol(row, handle);
            char* spt;
            tot_cycles[idx] = strtol(col,spt,0);
        }

        /* Read instruction retired */
        for (int idx=0;idx<n_cores;idx++){
            col = CsvReadNextCol(row, handle);
            char* spt;
            istr_ret[idx] = strtol(col,spt,0);
        }

        /* Parallel FP/MEM*/
        for (int idx=0;idx<n_cores;idx++){
            char* spt;
            /* FP DP */
            // Skip Total
            col = CsvReadNextCol(row, handle);
            for (int idx2=0;idx2<fp_dp_vect_dim;idx2++){
                col = CsvReadNextCol(row, handle);
                fpdp_cycles[idx*fp_dp_vect_dim+idx2] = strtol(col,spt,0);
            }
            // Skip Tr/te/tm 
            for (int idx2=0;idx2<fp_dp_skip_dim;idx2++){
                col = CsvReadNextCol(row, handle);
            }
            /* FP SP */
            // Skip Total
            col = CsvReadNextCol(row, handle);
            for (int idx2=0;idx2<fp_sp_vect_dim;idx2++){
                col = CsvReadNextCol(row, handle);
                fpsp_cycles[idx*fp_sp_vect_dim+idx2] = strtol(col,spt,0);
            }
            // Skip Tr/te/tm 
            for (int idx2=0;idx2<fp_sp_skip_dim;idx2++){
                col = CsvReadNextCol(row, handle);
            }
            /* MEM */
            col = CsvReadNextCol(row, handle);
            mem_cycles[idx] = strtol(col,spt,0);
            // Skip Data = MEM*64
            col = CsvReadNextCol(row, handle);
        }

        /* Print values */
        /*
        for (int idx=0;idx<n_cores;idx++){
            printf("Row/Core %d/%d: Load: %f, IPC: %f, Fpdp: %d, fpsp: %d, L2: %d, Vect: %d\n\r", counter_rows+1, idx,
                    load[idx], 1/cpi[idx], fpdp_cycles[idx*fp_dp_vect_dim+0], fpsp_cycles[idx*fp_dp_vect_dim+0], mem_cycles[idx], 0 );
        }
        */

        /** Compute Values **/
        int64_t    aprx    = 1000;
        float       aprxf   = 1000.0f;
        float       fpsp_p  = 0.8f;
        float       mem_p   = 0.65f;
        float       vectsp_p[3] = /*fp_sp_vect_dim-1*/
                        {0.5f, 0.8f, 1.0f};
        float       vectdp_p[3] = /*fp_dp_vect_dim-1*/
                        {0.7f, 0.95f, 1.0f};

        for (int idx=0;idx<n_cores;idx++){
            idle_c[idx] = (tot_cycles[idx]*(1*aprx-(int64_t)(load[idx]*aprxf))) / (int64_t)(load[idx]*aprxf);
            int_c[idx] = 0;
            float_c[idx] = (fpsp_cycles[idx*fp_sp_vect_dim+0]*(int64_t)(cpi[idx]*fpsp_p*aprxf))/aprx + 
                            (fpdp_cycles[idx*fp_sp_vect_dim+0]*(int64_t)(cpi[idx]*aprxf))/aprx;
            int_c[idx] += (fpsp_cycles[idx*fp_sp_vect_dim+0]*(int64_t)(cpi[idx]*(1-fpsp_p)*aprxf))/aprx;
            l2mem_c[idx] = (mem_cycles[idx]*(int64_t)(cpi[idx]*mem_p*aprxf))/aprx;
            int_c[idx] += (mem_cycles[idx]*(int64_t)(cpi[idx]*(1-mem_p)*aprxf))/aprx;

            vect_c[idx] = 0;
            for (int idx2=1;idx2<fp_sp_vect_dim;idx2++){
                vect_c[idx] += (fpsp_cycles[idx*fp_sp_vect_dim+idx2]*(int64_t)(cpi[idx]*vectsp_p[idx2-1]*aprxf))/aprx;
                float_c[idx] += (fpsp_cycles[idx*fp_sp_vect_dim+idx2]*(int64_t)(cpi[idx]*(1-vectsp_p[idx2-1])*aprxf))/aprx;
            }
            for (int idx2=1;idx2<fp_dp_vect_dim;idx2++){
                vect_c[idx] += (fpdp_cycles[idx*fp_dp_vect_dim+idx2]*(int64_t)(cpi[idx]*vectdp_p[idx2-1]*aprxf))/aprx;
                float_c[idx] += (fpdp_cycles[idx*fp_dp_vect_dim+idx2]*(int64_t)(cpi[idx]*(1-vectdp_p[idx2-1])*aprxf))/aprx;
            }

            int64_t sumall = int_c[idx] + float_c[idx] + l2mem_c[idx] + vect_c[idx];

            int_c[idx] += __max(0, tot_cycles[idx] - sumall);

            /* Print values */
            /*
            printf("Row/Core %d/%d: 0: %d, INT: %d, Fp: %d, L2: %d, Vect: %d\n\r", counter_rows+1, idx,
                    idle_c[idx]/aprx, int_c[idx]/aprx, float_c[idx]/aprx, l2mem_c[idx]/aprx, vect_c[idx]/aprx );
            */
        }

        for (int i = 0; i < simulation.nb_elements; i++){
            int idx = i%n_cores;
            int64_t sumall = idle_c[idx] + int_c[idx] + float_c[idx] + l2mem_c[idx] + vect_c[idx];

            create_quantum(&simulation.elements[i], i);
            last_quantum[i]->cpi_freq = 1.0f;
            last_quantum[i]->dim = sumall;
            last_quantum[i]->wl_perc[0] = (float)idle_c[idx]/(float)sumall;
            last_quantum[i]->wl_perc[1] = (float)int_c[idx]/(float)sumall;
            last_quantum[i]->wl_perc[2] = (float)float_c[idx]/(float)sumall;
            last_quantum[i]->wl_perc[3] = (float)l2mem_c[idx]/(float)sumall;
            last_quantum[i]->wl_perc[4] = (float)vect_c[idx]/(float)sumall;

            /*
            printf("\n\r quantum: %d \n\r\n\r", counter_rows);
            //for(int s=0; s<get_component_states_number(&simulation.elements[0]); s++)
            for(int s=0; s<5; s++)
                printf("[%d]=%f , ", s, last_quantum[i]->wl_perc[s]);
            printf(" dim: %ld, CPI: %f\n\r", last_quantum[i]->dim, last_quantum[i]->cpi_freq);
            */
        }
	    
	    counter_rows++;
	}

	/* Close file */
	CsvClose(handle);
}

void scmi_wl_parser(void)
{
    float dt_s = SCMI_WL_TIME_S;
    float tfreq = SCMI_TOP_FREQ;

    uint32_t iterations = (uint32_t)ceil((float)SCMI_SIM_TIME_S / dt_s / 2.0) -1;

    uint64_t dim_cl = (uint64_t)round((tfreq*1.0e9)*dt_s);

    for (int i = 0; i < simulation.nb_elements; i++){
        create_quantum(&simulation.elements[i], i);
        last_quantum[i]->cpi_freq = 1.0f;
        last_quantum[i]->dim = 0;
        last_quantum[i]->us_wait_time = SCMI_WL_TIME_S*1000000.0f;
        last_quantum[i]->wl_perc[0] = (float)1.0f;
        last_quantum[i]->wl_perc[1] = 0;
        last_quantum[i]->wl_perc[2] = 0;
        last_quantum[i]->wl_perc[3] = 0;
        last_quantum[i]->wl_perc[4] = 0;

        head_quantum[i] = last_quantum[i];

        create_quantum(&simulation.elements[i], i);
        last_quantum[i]->cpi_freq = 1.0f;
        last_quantum[i]->dim = dim_cl;
        last_quantum[i]->wl_perc[0] = 0;
        last_quantum[i]->wl_perc[1] = 0;
        last_quantum[i]->wl_perc[2] = 0.5f;
        last_quantum[i]->wl_perc[3] = 0;
        last_quantum[i]->wl_perc[4] = 0.5f;
    }

    for (int i=0;i<iterations;i++)
    {
        for (int i = 0; i < simulation.nb_elements; i++){
            create_quantum(&simulation.elements[i], i);
            last_quantum[i]->cpi_freq = 1.0f;
            last_quantum[i]->dim = 0;
            last_quantum[i]->us_wait_time = SCMI_WL_TIME_S*1000000.0f;
            last_quantum[i]->wl_perc[0] = (float)1.0f;
            last_quantum[i]->wl_perc[1] = 0;
            last_quantum[i]->wl_perc[2] = 0;
            last_quantum[i]->wl_perc[3] = 0;
            last_quantum[i]->wl_perc[4] = 0;

            create_quantum(&simulation.elements[i], i);
            last_quantum[i]->cpi_freq = 1.0f;
            last_quantum[i]->dim = dim_cl;
            last_quantum[i]->wl_perc[0] = 0;
            last_quantum[i]->wl_perc[1] = 0;
            last_quantum[i]->wl_perc[2] = 0.5f;
            last_quantum[i]->wl_perc[3] = 0;
            last_quantum[i]->wl_perc[4] = 0.5f;
        }
    }

    /*
    for (int i = 0; i < simulation.nb_elements; i++){
        create_quantum(&simulation.elements[i], i);
        last_quantum[i]->cpi_freq = 1.0f;
        last_quantum[i]->dim = 0;
        last_quantum[i]->us_wait_time = SCMI_WL_TIME_S*1000000.0f;
        last_quantum[i]->wl_perc[0] = (float)1.0f;
        last_quantum[i]->wl_perc[1] = 0;
        last_quantum[i]->wl_perc[2] = 0;
        last_quantum[i]->wl_perc[3] = 0;
        last_quantum[i]->wl_perc[4] = 0;

        head_quantum[i] = last_quantum[i];

        create_quantum(&simulation.elements[i], i);
        last_quantum[i]->cpi_freq = 1.0f;
        last_quantum[i]->dim = dim_cl;
        last_quantum[i]->wl_perc[0] = 0;
        last_quantum[i]->wl_perc[1] = 0;
        last_quantum[i]->wl_perc[2] = 0.5f;
        last_quantum[i]->wl_perc[3] = 0;
        last_quantum[i]->wl_perc[4] = 0.5f;

        last_quantum[i]->next = last_quantum[i]->prev;
    }
    */
}








