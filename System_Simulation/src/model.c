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

#include "model.h"
#include "main.h"
#include "data.h"
#include "cmdconf.h"

#include "sim_config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <time.h>

#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif


//#define COE_PI 3.14159265358979323846

//Model Variables:
double* state;
unsigned int NoiseIndex = 0;

double* A = NULL;
double* B = NULL;
//had to put these here even though they are just needed inside model_step, cuz otherwise I needed them to be malloced each iteration
double* inputMat;
double* tempInput;
double* tempState;

uint32_t sdim;
uint32_t udim;

//TODO create a clear model function to free(A), and free(B) and all others

void model_initialization(int Nc, int Nh, int Nv, float Ts, uint32_t* otdim, uint32_t* oudim)
{

	if (create_thermal_model(&A, &B, Nc, Nh, Nv, Ts))
	{
		exit(1);
	}
    /*
    for (int i=0; i<2*Nc*2*Nc; i++)
    {     
        if (i%(2*Nc)==0)
            printf("\n\r");
        printf("%f ", A[i]);
    }
    */
    /*
    for(int t=0; t<N_EPI_CORE*2*N_EPI_CORE*2; t++)
    {
        if (A[t] - A_test[t] > 1e-3)
            printf("A[%d][%d]: %f / %f \n\r", (t/(N_EPI_CORE*2)), (t%(N_EPI_CORE*2)), A[t], A_test[t]);
        if (B[t] - B_test[t] > 1e-3)
            printf("B[%d][%d]: %f / %f \n\r", (t/(N_EPI_CORE*2)), (t%(N_EPI_CORE*2)), B[t], B_test[t]);
    }
    */

	//Initialization
	inputMat = (double*)malloc(sizeof(double)*udim);
	tempInput = (double*)malloc(sizeof(double)*sdim);
	tempState = (double*)malloc(sizeof(double)*sdim);
    state = (double*)malloc(sizeof(double)*sdim);
    //TODO, deinit??

    srand(1546);
	for (int i=0; i< sdim; i++)
	{
        double noise0 = (double)rand()/(double)RAND_MAX;
		state[i] = (double)Tamb + noise0*5.0f;
	}

    *otdim = sdim;
    *oudim = udim;
}


void model_step(float* oTemp, float *oDomainPower, float (*iWlPerc)[WL_STATES], int sensor_read)
{


	/*
	float Pstat = 0.0;
	float Pdin = 0.0;
	float Pthrottl = 0.0;
	float Ptot = 0.0;
	float throttl = 0.0;
	*/

	NoiseIndex++;
	if (NoiseIndex >= (NOISE_MAT_DIM - 3))
		{NoiseIndex = 0;}

    int core_count=0;

	// POWER MODEL
	//float step_total_power = 0;
	// iterate through the elements and compute the power model
    for (int i = 0; i < simulation.nb_elements; i++)
	{
        //TODO: add additional checking for nb_elements, for OoB
        //printf("%d, p1, cid: %d  ", i, simulation.elements[i].core_config.cid);
        //printf(" - pointers %d: %p, %p\n", i, &simulation.elements[i], simulation.elements[i].compute_power);
        float power = simulation.elements[i].compute_power(&simulation.elements[i], ifp_ctrl_core_freq[i], ifp_ctrl_quad_vdd[simulation.elements[i].domain], (float)state[i*2], 1.0, &iWlPerc[i][0]);
        //printf("number: %d, value: %f\n\r", i, power);
		//printf("freq: %f, vdd: %f, temp: %f, instr: %f %f %f %f %f\n\r", ifp_ctrl_core_freq[i], ifp_ctrl_quad_vdd[CoreQuad[i]], state[i*2], iWlPerc[i][0], iWlPerc[i][1], iWlPerc[i][2], iWlPerc[i][3], iWlPerc[i][4]);
        // save results in memory
        //out_domain_power[1 + domain] += p_tot;
        //out_domain_power[0] += p_tot;
	/*
	for (int i = 0; i < Nc; i++)
	{
		int temp_index = ( ((int)state[i*2] - 274) / TEMP_DER_STEP);

		Ptot = 0;

		float equiv_Icc = 0;
		float equiv_Ceff = 0;
		for (int j = 0; j < WL_STATES; j++)
		{
			equiv_Icc += Icc[j] * iWlPerc[i][j];
			equiv_Ceff += Ceff[j] * iWlPerc[i][j];
		}

#ifdef CCE_1
		equiv_Icc *= Icc_Chiplet * Icc_coreUncert[i] * Icc_tempDer[temp_index];
		equiv_Icc += (Icc_noise[NoiseIndex]);

		equiv_Ceff *= Ceff_Chiplet * Ceff_coreUncert[i] * Ceff_tempDer[temp_index];
		//equiv_Ceff += 0;
#endif

		//Equations:
		//throttl = 1 - (ifp_ctrl_core_freq[i] / ifp_ctrl_quad_freq[CoreQuad[i]]);
		throttl = 0;

		Pstat = equiv_Icc * ifp_ctrl_quad_vdd[CoreQuad[i]];
		Pdin = (1 - throttl) * ( equiv_Ceff * ifp_ctrl_core_freq[i] * ifp_ctrl_quad_vdd[CoreQuad[i]] * ifp_ctrl_quad_vdd[CoreQuad[i]] ) ;
		Pthrottl = throttl * (P_thrott__);

		Ptot = ( Pstat + Pdin + Pthrottl);
 	*/
	 	oDomainPower[1+simulation.elements[i].domain] += power;
		oDomainPower[0] += power;
    
        if (simulation.elements[i].type == JT_CORE) //TOO fix thermal model
        {
            //TODO using the id is wrong.
            //Also I'm not properly considering cores positions.
            inputMat[core_count] = (double)(power * ThermalCorrection[core_count]); //after, because we want it to affect just the thermal model
            //printf("core: %d, i: %d, id: %d\n\r", app, i, simulation.elements[i].id);
            core_count++; //here I assumed all cores are in a sequence, no core position
        }
        //inputMat[i] = (double)(power * ThermalCorrection); //after, because we want it to affect just the thermal model
	}

	//Finishing populating input matrix
	inputMat[udim-1] = (double)Tamb*1000;

	//State Generation (A*X(t))
	matMul(tempState, A, state, (sdim), (sdim), 1 );

	//Model Temp Computation (B*u(t))
	matMul(tempInput, B, inputMat, (sdim), (udim), 1);
	matSum(state, tempState, tempInput, sdim, 1);

	//State Update - copying
	//for(int i=0; i< Nc*2; i++)
	//	state[i] = tempState[i];

	//Output generation:
    if (sensor_read)
    {
        for (int i = 0; i < sdim /*simulation.nb_cores*/; i++)
        {
#ifdef CCE_1.0 //Noise
		    //oTemp[i] = (float)state[i*2] + TempSensorNoise[NoiseIndex];
            oTemp[i] = (float)state[i] + TempSensorNoise[NoiseIndex];
#else
		    //oTemp[i] = (float)state[i*2];
            oTemp[i] = (float)state[i];
#endif
            //Index overflow! Do not uncomment!
            //oTemp[simulation.nb_cores+i] = state[i*2];
	    }
    } //sensor_read

	return;
}


double spreading_r_computation(double source_area, double plate_area, double plate_thickness, double k_source, double R_plate)	
{
    
    double src_r = pow(source_area / M_PI,0.5);
    double plt_r = pow(plate_area / M_PI,0.5);
    double tau = plate_thickness / plt_r;
    
    double epsi = src_r/plt_r;
    double h = 1.0 / (R_plate * plate_area);
    double Biot = h * plt_r / k_source;
    
    double delta_c = M_PI + 1.0/(pow(M_PI,0.5)*epsi);
    double phi_c = ( tanh(delta_c*tau) + delta_c/Biot ) / ( 1.0 + delta_c/Biot*tanh(delta_c*tau) );			
    
    double psi_avg = pow(1.0-epsi,1.5)*phi_c / 2.0;
    double psi_max = (1.0-epsi)*phi_c/(pow(M_PI,0.5));

    //printf("src: %f-%f, plt: %f-%f, tau: %f, epsi: %f, h: %f, Biot: %f, delta: %f, phi: %f, psi: %f\n\r", 
    //src_r, source_area, plt_r, plate_area, tau, epsi, h, Biot, delta_c, phi_c, psi_avg);
    
    return (psi_avg / k_source / (pow(source_area,0.5)));
}

int create_thermal_model(double** A_th, double** B_th, int Nc, int Nh, int Nv, float Ts)
{
    /*** CHECKS ***/
    if ((Nc <= 0) || (Nh <= 0) || (Nv <= 0))
    {
        printf("[Model] Error! Thermal model creation one or more dimensions are <= 0\r\n");
        return 1;
    }
    if (Nc != Nh*Nv)
    {
        //TODO: Make a modification for 'not exactly symmetric' cores configurations: i.e. a 9 core  in 2x5 disposition
        printf("[MODEL] Error! The number of cores is not equal to the product of horizontal and vertical dimensions in cores disposition\r\n");
        return 1;
    }

    /*** CONSTANTS ***/
    int add_states = 4;
    int full_model_layers = 2;
    int add_inputs		= 1;

    int air_pos = 0;
    int mb_pos  = 1;
    int pcb_pos = 2;
    int al_pos  = 3;

	int extt_rows = 1;
	int extb_rows = 1;
	int extl_cols = 1;
	int extr_cols = 1;

    int north_pos	= 1-1;
    int east_pos	= 2-1;
    int south_pos	= 3-1;
    int west_pos	= 4-1;

    int si_pos		= 1-1;
    int cu_pos		= 2-1;

    // PHYSICAL values

    double alpha_k_si    = -4e-3;
    double alpha_k_cu    = -1e-4;
    double k_si          = 127; //148;
    double k_cu          = 398.5; //400;
    double k_air         = 0.025;
    double k_al          = 225.94;
    double k_pcb         = 3.096;
    double k_mb          = 0.167;

    double alpha_c_si    = 1e-3;
    double alpha_c_cu    = 3e-4;
    double c_si          = 1.7243e+06; //1.66e6;
    double c_cu          = 3.4794e+06; //3.44e6;
    //SHC = Specific Heat Capacity
    double c_air         = 1004*1.29*1.5; //SHC*air density*"air compression factor" 
    double c_al          = 921*2698; //SHC*density
    double c_pcb         = 753*2900; //SHC*density
    double c_mb          = 795*1900; //SHC*density

    double si_pcb_fact = 0.1;
	double pcb_mb_fact = 15.0;
	double mb_air_fact = 10.0;

    /*** VARIABLES ***/
    //TODO: Import the values from the .json file!!
    double* wid_nFML = (double *)calloc(add_states, sizeof(double));
    int end = add_states-1;
    wid_nFML[end-al_pos]   = 0.0416;
    wid_nFML[end-pcb_pos]  = 0.0366;
    wid_nFML[end-mb_pos]   = 10e-2;
    wid_nFML[end-air_pos]  = 20e-2;
    double* len_nFML = (double *)calloc(add_states, sizeof(double));
    len_nFML[end-al_pos]   = 0.0416;
    len_nFML[end-pcb_pos]  = 0.0366;
    len_nFML[end-mb_pos]   = 10e-2;
    len_nFML[end-air_pos]  = 20e-2;
    double* t_nFML = (double *)calloc(add_states, sizeof(double));
    t_nFML[end-al_pos]   = 1.5e-2;
    t_nFML[end-pcb_pos]  = 1e-3;
    t_nFML[end-mb_pos]   = 2e-3;
    t_nFML[end-air_pos]  = 10e-2;

    double t_si = 5e-4;
	double t_cu = 1e-3;
    double R_TIM1 = 1.0;
	double R_TIM2 = 2.5;
	double case_fan_dis = 0.25;
	double case_fan_nom_speed = 1000.0;
	double al_fan_dis= 10.0;
	double al_fins_coeff = 3.0;

	double air_factor = 100.0;
    double pw2therm_coeff = 1.05;

    // Global vars init:
    sdim = full_model_layers*Nc+add_states;
    udim = Nc+add_inputs;

    /*** Computation of nFML RC ***/
    //Capacitance:
    double* A_comp = (double *)calloc(add_states, sizeof(double));
	for (int i=0;i<add_states;i++)
		A_comp[i] = wid_nFML[i] * len_nFML[i];
	
    end = add_states-1;
	double C_al = c_al * A_comp[end-al_pos] * t_nFML[end-al_pos];
	double C_pcb = c_pcb * A_comp[end-pcb_pos] * t_nFML[end-pcb_pos];
	double C_mb = c_mb * A_comp[end-mb_pos] * t_nFML[end-mb_pos];
	double C_air = c_air * A_comp[end-air_pos] * t_nFML[end-air_pos];

    //Resistance:
	//partial
    end = add_states-1;
	double Ri_air_tot_v = t_nFML[end-air_pos] / A_comp[end-air_pos] / k_air / air_factor;
	double air_al_t = t_nFML[end-air_pos] - (t_nFML[end-al_pos]+t_cu+t_si+t_nFML[end-pcb_pos]);
	double Ri_air_al_t = air_al_t / A_comp[end-air_pos] / k_air / air_factor;
	double Ri_air_al_len = (len_nFML[end-air_pos]-len_nFML[end-al_pos]) / 2.0
					/ (wid_nFML[end-air_pos] * t_nFML[end-air_pos])
					/ k_air / air_factor;
	double Ri_air_al_wid = (wid_nFML[end-air_pos]-wid_nFML[end-al_pos]) / 2.0
					/ (len_nFML[end-air_pos] * t_nFML[end-air_pos])
					/ k_air / air_factor;
	//
	double Ri_al_v = t_nFML[end-al_pos] / A_comp[end-al_pos] / k_al;
	double Ri_al_len = len_nFML[end-al_pos] / (wid_nFML[end-al_pos] * t_nFML[end-al_pos]) / k_al;
	double Ri_al_wid = wid_nFML[end-al_pos] / (len_nFML[end-al_pos] * t_nFML[end-al_pos]) / k_al;
	//
	double Ri_pcb_v = t_nFML[end-pcb_pos] / A_comp[end-pcb_pos] / k_pcb;
	double Ri_mb_v = t_nFML[end-mb_pos] / A_comp[end-mb_pos] / k_mb;
	//
	double RaL = 1.948e09 * pow(air_al_t,3.0);
	double h_alair_cond = pow(RaL,(1.0/4.0)) * 0.54 * k_air / air_al_t;
	double R_alair_cond = 1.0 / (h_alair_cond * A_comp[end-al_pos] * al_fins_coeff);
	
	RaL = 1.948E+09 * pow((air_al_t + t_nFML[end-al_pos]/2.0),3.0);
	h_alair_cond = k_air / (air_al_t + t_nFML[end-al_pos]/2.0) * 
		pow((0.825 + (0.387 * pow(RaL,(1.0/6.0)))/pow(1.0+pow(0.492/7.039E-01,(9.0/16.0)),8.0/27.0)),2.0);
	double R_alair_len_cond = 1.0 / (h_alair_cond * (wid_nFML[end-al_pos] * t_nFML[end-al_pos]));
	double R_alair_wid_cond = 1.0 / (h_alair_cond * (len_nFML[end-al_pos] * t_nFML[end-al_pos]));
	
	RaL = 1.948E+09 * pow((t_nFML[end-air_pos]),3.0);
	double h_mbair_cond = pow(RaL,(1.0/4.0)) * 0.54 * k_air / (t_nFML[end-air_pos]);
	double R_mbair_cond = 1.0 / (h_mbair_cond * (A_comp[end-mb_pos] - A_comp[end-al_pos]));

    //source_area, plate_area, plate_thickness, k_source, R_plate
	double R_spr = spreading_r_computation(A_comp[end-al_pos], A_comp[end-air_pos],
		t_nFML[end-air_pos], k_al, Ri_air_al_t);
	double R_alair_v = Ri_al_v + R_alair_cond + R_spr;

	R_spr = spreading_r_computation((wid_nFML[end-al_pos] * t_nFML[end-al_pos]),
		(wid_nFML[end-air_pos] * t_nFML[end-air_pos]), len_nFML[end-air_pos], k_al, Ri_air_al_len);
	double R_alair_len = Ri_al_len + R_alair_len_cond + R_spr;
	
	R_spr = spreading_r_computation(len_nFML[end-al_pos] * t_nFML[end-al_pos],
		(len_nFML[end-air_pos] * t_nFML[end-air_pos]), wid_nFML[end-air_pos], k_al, Ri_air_al_wid);
	double R_alair_wid = Ri_al_wid + R_alair_wid_cond + R_spr;
	//
	R_spr = spreading_r_computation(A_comp[end-pcb_pos], A_comp[end-mb_pos],
		t_nFML[end-mb_pos], k_pcb, Ri_mb_v);
	double R_pcbmb_v = Ri_pcb_v + Ri_mb_v + R_spr;
	R_spr = spreading_r_computation(A_comp[end-mb_pos], A_comp[end-air_pos],
		t_nFML[end-air_pos], k_mb, Ri_air_tot_v);
	double R_mbair_v = Ri_mb_v + R_mbair_cond + R_spr;
	
	//TODO
	//R_air_v = R_air_v * ThMoNoise(fix(i/2)+1,7);
	//C_air = Ci_air * ThMoNoise(fix(i/2)+1,7);

    /*** Default Floorplan ***/
    int core_cols = Nv;
	int core_rows = Nh;

	int cols = core_cols + extl_cols + extr_cols;
	int rows = core_rows + extt_rows + extb_rows;

	// create Distance/position matrix
	double* RC_fp_dim = (double *)calloc(rows*cols*4, sizeof(double));
	double* CPw_fp_dim = (double *)calloc(core_rows*core_cols*4, sizeof(double));
	double* R_fp_material = (double *)calloc(rows*cols*full_model_layers, sizeof(double));
	double* C_fp_material = (double *)calloc(rows*cols*full_model_layers, sizeof(double));

	// Populating:
	//([1:extt_rows, end-extb_rows+1:end], ...
	//	[1:extl_cols, end-extr_cols+1:end],:)
    for (int i=0; i<rows*cols*4; i++)
    {
        RC_fp_dim[i]		= 0.05;
    }
    for (int i=0; i<rows*cols*full_model_layers; i++)
    {
        R_fp_material[i] = 1e-16;
        C_fp_material[i] = 0.025;
    }	

    int fpdim = cols*rows;
	for (int r =extt_rows; r<(rows - extb_rows); r++)
    {
		for (int c = extl_cols; c<(cols - extr_cols); c++)
        {
			RC_fp_dim[(r*cols+c) + north_pos*fpdim] = 2.30e-3;
			RC_fp_dim[(r*cols+c) + south_pos*fpdim] = 1e-3;

			RC_fp_dim[(r*cols+c) + west_pos*fpdim] = 0.75e-3;
			RC_fp_dim[(r*cols+c) + east_pos*fpdim] = 0.75e-3;					

			// Internal Mesh connection
			if ((c%2 == 0) && (c!=(core_cols+extl_cols-1)))
				RC_fp_dim[(r*cols+c) + east_pos*fpdim] += 0.25e-3;
			
			// Eternal Mesh connection: row
			if (r == (extt_rows))
				RC_fp_dim[(r*cols+c) + north_pos*fpdim] += 0.3e-3;
			if (r == (rows - extb_rows-1))
				RC_fp_dim[(r*cols+c) + south_pos*fpdim] += 0.3e-3;

			// Eternal Mesh connection: cols
			if (c == (extl_cols))
				RC_fp_dim[(r*cols+c) + west_pos*fpdim] += 0.3e-3;
			if (c == (cols - extr_cols-1))
				RC_fp_dim[(r*cols+c) + east_pos*fpdim] += 0.3e-3;
			
			R_fp_material[(r*cols+c) + si_pos*fpdim] = k_si;
			R_fp_material[(r*cols+c) + cu_pos*fpdim] = k_cu;
			C_fp_material[(r*cols+c) + si_pos*fpdim] = c_si;
			C_fp_material[(r*cols+c) + cu_pos*fpdim] = c_cu;
        }
    }
    int fpldim = core_rows*core_cols;
	for (int r=0;r<core_rows;r++)
		for (int c=0;c<core_cols;c++)
        {
			CPw_fp_dim[(r*core_cols+c) + north_pos*fpldim] = 1e-3;
			CPw_fp_dim[(r*core_cols+c) + south_pos*fpldim] = 1e-3;
			CPw_fp_dim[(r*core_cols+c) + west_pos*fpldim] = 0.75e-3;
			CPw_fp_dim[(r*core_cols+c) + east_pos*fpldim] = 0.75e-3;
        }

    /* Print Matrix floorplans */
    /*
    for (int i=0; i<rows*cols*4; i++)
    {
        if (i%(cols)==0)
            printf("\n\r");
        if (i%(rows*cols)==0)
            printf("\n\r");
        printf("%lf ", RC_fp_dim[i]);
        //printf("%.8e ", RC_fp_dim[i]);
    }
    printf("\n\r\n\r");
    */
    /*
    for (int i=0; i<core_rows*core_cols*4; i++)
    {
        if (i%(core_cols)==0)
            printf("\n\r");
        if (i%(core_rows*core_cols)==0)
            printf("\n\r");
        printf("%lf ", CPw_fp_dim[i]);
        //printf("%.8e ", RC_fp_dim[i]);
    }
    printf("\n\r\n\r");
    */
    /* End default floorplan*/

    //State Matrix assuming each node is a second order sub-system (silicon and heater dynamics) interacting with neighbours and T_amb
    double* A_th_c = (double *)calloc(sdim*sdim, sizeof(double));
    if (A_th_c == NULL)
    {
        printf("[Model] Error creating the A continous array\r\n");
        return 1;
    }
    //Populate
    double R_si_hn;
    double R_si_hs;
    double R_si_he;
    double R_si_hw;

    double R_sicu_v;

    double R_cu_hn;
    double R_cu_hs;
    double R_cu_he;
    double R_cu_hw;					
    
    double R_cual_v;
    double R_sipcb_v;
    
    double C_si;
    double C_cu;

    for (int i=0;i<full_model_layers*Nc;i++)
    {
		if ((i%full_model_layers)==0)   // For each core:
        {
			// Core Position
			int ci = i/2; //((i-1)/2)+1;
			int irow = (ci/Nv) + extt_rows;
			int icol = (ci%Nv) + extl_cols;

			// Computing Core R 
			double li = RC_fp_dim[(irow*cols+icol) + north_pos*fpdim] + RC_fp_dim[(irow*cols+icol) + south_pos*fpdim];
			double wi = RC_fp_dim[(irow*cols+icol) + east_pos*fpdim] + RC_fp_dim[(irow*cols+icol) + west_pos*fpdim];
			//Si:
            double R_Si = R_fp_material[(irow*cols+icol) + si_pos*fpdim];
			double Ri_si_hn = RC_fp_dim[(irow*cols+icol) + north_pos*fpdim]/(wi*t_si)/R_Si;
			double Ri_si_hs = RC_fp_dim[(irow*cols+icol) + south_pos*fpdim]/(wi*t_si)/R_Si;
			double Ri_si_he = RC_fp_dim[(irow*cols+icol) + east_pos*fpdim]/(li*t_si)/R_Si;
			double Ri_si_hw = RC_fp_dim[(irow*cols+icol) + west_pos*fpdim]/(li*t_si)/R_Si;
			//
			double Ri_si_v = (t_si/2)/(wi*li)/R_Si;

			//Cu:
            double R_Cu = R_fp_material[(irow*cols+icol) + cu_pos*fpdim];
			double Ri_cu_hn = RC_fp_dim[(irow*cols+icol) + north_pos*fpdim]/(wi*t_cu)/R_Cu;
			double Ri_cu_hs = RC_fp_dim[(irow*cols+icol) + south_pos*fpdim]/(wi*t_cu)/R_Cu;
			double Ri_cu_he = RC_fp_dim[(irow*cols+icol) + east_pos*fpdim]/(li*t_cu)/R_Cu;
			double Ri_cu_hw = RC_fp_dim[(irow*cols+icol) + west_pos*fpdim]/(li*t_cu)/R_Cu;
			//
			double Ri_cu_v = (t_cu/2)/(wi*li)/R_Cu;

			//Others: Al + PCB
            end = add_states-1;
			double R_spr_cual = spreading_r_computation((wi*li), A_comp[end-al_pos],
				t_nFML[end-al_pos], k_cu, Ri_al_v);
			double R_spr_sipcb = spreading_r_computation((wi*li), A_comp[end-pcb_pos],
				t_nFML[end-pcb_pos], k_si, Ri_pcb_v);

			// Computing Core C
			double Ci_si = C_fp_material[(irow*cols+icol) + si_pos*fpdim] * li*wi*t_si;
			double Ci_cu = C_fp_material[(irow*cols+icol) + cu_pos*fpdim] * li*wi*t_cu;
			
			// Computing Neighbourhood R
			wi = RC_fp_dim[((irow-1)*cols+icol) + east_pos*fpdim] + RC_fp_dim[((irow-1)*cols+icol) + west_pos*fpdim];
			double Rin_si_hn = RC_fp_dim[((irow-1)*cols+icol) + south_pos*fpdim] / (wi*t_si) / R_fp_material[((irow-1)*cols+icol) + si_pos*fpdim];
			double Rin_cu_hn = RC_fp_dim[((irow-1)*cols+icol) + south_pos*fpdim] / (wi*t_cu) / R_fp_material[((irow-1)*cols+icol) + cu_pos*fpdim];
			wi = RC_fp_dim[((irow+1)*cols+icol) + east_pos*fpdim] + RC_fp_dim[((irow+1)*cols+icol) + west_pos*fpdim];
			double Rin_si_hs = RC_fp_dim[((irow+1)*cols+icol) + north_pos*fpdim] / (wi*t_si) / R_fp_material[((irow+1)*cols+icol) + si_pos*fpdim];
			double Rin_cu_hs = RC_fp_dim[((irow+1)*cols+icol) + north_pos*fpdim] / (wi*t_cu) / R_fp_material[((irow+1)*cols+icol) + cu_pos*fpdim];
			li = RC_fp_dim[(irow*cols+(icol+1)) + north_pos*fpdim] + RC_fp_dim[(irow*cols+(icol+1)) + south_pos*fpdim];
			double Rin_si_he = RC_fp_dim[(irow*cols+(icol+1)) + west_pos*fpdim] / (li*t_si) / R_fp_material[(irow*cols+(icol+1)) + si_pos*fpdim];
			double Rin_cu_he = RC_fp_dim[(irow*cols+(icol+1)) + west_pos*fpdim] / (li*t_cu) / R_fp_material[(irow*cols+(icol+1)) + cu_pos*fpdim];
			li = RC_fp_dim[(irow*cols+(icol-1)) + north_pos*fpdim] + RC_fp_dim[(irow*cols+(icol-1)) + south_pos*fpdim];
			double Rin_si_hw = RC_fp_dim[(irow*cols+(icol-1)) + east_pos*fpdim] / (li*t_si) / R_fp_material[(irow*cols+(icol-1)) + si_pos*fpdim];
			double Rin_cu_hw = RC_fp_dim[(irow*cols+(icol-1)) + east_pos*fpdim] / (li*t_cu) / R_fp_material[(irow*cols+(icol-1)) + cu_pos*fpdim];

			// Series
			R_si_hn = Ri_si_hn + Rin_si_hn;
			R_si_hs = Ri_si_hs + Rin_si_hs;
			R_si_he = Ri_si_he + Rin_si_he;
			R_si_hw = Ri_si_hw + Rin_si_hw;

			R_sicu_v = Ri_si_v + Ri_cu_v + R_TIM1;

			R_cu_hn = Ri_cu_hn + Rin_cu_hn;
			R_cu_hs = Ri_cu_hs + Rin_cu_hs;
			R_cu_he = Ri_cu_he + Rin_cu_he;
			R_cu_hw = Ri_cu_hw + Rin_cu_hw;					
			
			R_cual_v = Ri_cu_v + R_TIM2 + Ri_al_v + R_spr_cual;
			R_sipcb_v = Ri_si_v + Ri_pcb_v + R_spr_sipcb;
			
			C_si = Ci_si;
			C_cu = Ci_cu;

			// Noise
            //TODO:
            /*
			if pdev==1
			R_si_hn = R_si_hn * param_dev_per(ci,1);
			R_si_hs = R_si_hs * param_dev_per(ci,1);
			R_si_he = R_si_he * param_dev_per(ci,1);
			R_si_hw = R_si_hw * param_dev_per(ci,1);

			R_cu_hn = R_cu_hn * param_dev_per(ci,2);
			R_cu_hs = R_cu_hs * param_dev_per(ci,2);
			R_cu_he = R_cu_he * param_dev_per(ci,2);
			R_cu_hw = R_cu_hw * param_dev_per(ci,2);

			R_sicu_v = R_sicu_v * param_dev_per(ci,3);
			R_cual_v = R_cual_v * param_dev_per(ci,4);

			C_si = Ci_si * param_dev_per(ci,5);
			C_cu = Ci_cu * param_dev_per(ci,6);
			end
            */

            // Models
            //TODO:
        }

        //for j=1:2*Nc
        end = sdim-1;
        
        // ==============
        // diagonal terms
        // ==============

        //Silicon (die temp dyn) 
        if ((i%full_model_layers)==0)
        {
            A_th_c[i*sdim + i] = -1.0/(R_sicu_v*C_si) -1.0/C_si * (1.0/R_si_he+1.0/R_si_hn+1.0/R_si_hw+1.0/R_si_hs) - 1.0/(C_si*R_sipcb_v) * si_pcb_fact;

            //PCB:
            A_th_c[i*sdim + end-pcb_pos] = 1.0/(C_si*R_sipcb_v) * si_pcb_fact;
            A_th_c[(end-pcb_pos)*sdim + i] = 1.0/(C_pcb*R_sipcb_v) * si_pcb_fact;
        }
        // Heat-Spread (Cooper dyn)
        else //if (mod(i,2)==0)
        {
            A_th_c[i*sdim + i] = -1.0/(R_sicu_v*C_cu) -1.0/(R_cual_v*C_cu) -1.0/C_cu * (1.0/R_cu_he+1.0/R_cu_hn+1.0/R_cu_hw+1.0/R_cu_hs);
            
            //heat sink:
            A_th_c[i*sdim + end-al_pos] = 1.0/(C_cu*R_cual_v);
            A_th_c[(end-al_pos)*sdim + i] = 1.0/(C_al*R_cual_v);
        }

        // ==============
        // Vertical coupling terms btween silicon and copper (flow from die to spreader)
        // ==============
        if ((i%full_model_layers)==0)
            A_th_c[i*sdim + (i+1)]=1.0/(R_sicu_v*C_si);
        else //if (mod(i,2)==0)
            A_th_c[i*sdim + (i-1)]=1.0/(R_sicu_v*C_cu);

        // ==============
        // horizontal coupling of die thermal dyn (silicon)
        // ==============
        //if(Nc~=1) //avoid single core case
        if ((i%full_model_layers)==0) 
        {
            // vertici
            if (i==0) {
                A_th_c[i*sdim + (i+2)]=1.0/(R_si_he*C_si); 
                A_th_c[i*sdim + (i+2*Nv)]=1.0/(R_si_hs*C_si);
            } else if (i==2*Nv-2) {
                A_th_c[i*sdim + (i-2)]=1.0/(R_si_hw*C_si); 
                A_th_c[i*sdim + (i+2*Nv)]=1.0/(R_si_hs*C_si);
            } else if (i==(Nh-1)*2*Nv) {
                A_th_c[i*sdim + (i+2)]=1.0/(R_si_he*C_si); 
                A_th_c[i*sdim + (i-2*Nv)]=1.0/(R_si_hn*C_si);
            } else if (i==2*Nc-2) {
                A_th_c[i*sdim + (i-2)]=1.0/(R_si_hw*C_si); 
                A_th_c[i*sdim + (i-2*Nv)]=1.0/(R_si_hn*C_si);
            }
            // bordi verticali (west and east)
            else if ((i%(2*Nv)==0) && (i!=(Nh-1)*2*Nv)&&(i!=0)) {
                A_th_c[i*sdim + (i+2)]=1.0/(R_si_he*C_si); 
                A_th_c[i*sdim + (i+2*Nv)]=1.0/(R_si_hs*C_si);
                A_th_c[i*sdim + (i-2*Nv)]=1.0/(R_si_hn*C_si);
            } else if ((i%(2*Nv)==0) && (i!=2*Nc-2) && (i!=2*Nv-2)) {
                A_th_c[i*sdim + (i-2)]=1.0/(R_si_hw*C_si); 
                A_th_c[i*sdim + (i+2*Nv)]=1.0/(R_si_hs*C_si);
                A_th_c[i*sdim + (i-2*Nv)]=1.0/(R_si_hn*C_si);
            }
            // horizonatal boundaries (upper and lower)
            else if ((i!=0)&& (i<2*Nv-3)) {
                A_th_c[i*sdim + (i-2)]=1.0/(R_si_hw*C_si); 
                A_th_c[i*sdim + (i+2)]=1.0/(R_si_he*C_si); 
                A_th_c[i*sdim + (i+2*Nv)]=1.0/(R_si_hs*C_si);
            } else if ((i>(Nh-1)*2*Nv) && (i<2*Nc-2)) {
                A_th_c[i*sdim + (i-2)]=1.0/(R_si_hw*C_si); 
                A_th_c[i*sdim + (i+2)]=1.0/(R_si_he*C_si); 
                A_th_c[i*sdim + (i-2*Nv)]=1.0/(R_si_hn*C_si);
            }
            //internal
            else {
                A_th_c[i*sdim + (i-2)]=1.0/(R_si_hw*C_si); 
                A_th_c[i*sdim + (i+2)]=1.0/(R_si_he*C_si); 
                A_th_c[i*sdim + (i-2*Nv)]=1.0/(R_si_hn*C_si);
                A_th_c[i*sdim + (i+2*Nv)]=1.0/(R_si_hs*C_si);           
            }
        }
        // ==============
        // horizontal coupling of sperader thermal dyn (copper)
        // ==============
        if ((i%full_model_layers)==1) 
        {
            // vertexes
            if(i==1) {
                A_th_c[i*sdim + (i+2)]=1.0/(R_cu_he*C_cu); 
                A_th_c[i*sdim + (i+2*Nv)]=1.0/(R_cu_hs*C_cu);
            } else if(i==2*Nv-1) {
                A_th_c[i*sdim + (i-2)]=1.0/(R_cu_hw*C_cu); 
                A_th_c[i*sdim + (i+2*Nv)]=1.0/(R_cu_hs*C_cu);
            } else if (i==(Nh-1)*2*Nv+1) {
                A_th_c[i*sdim + (i+2)]=1.0/(R_cu_he*C_cu); 
                A_th_c[i*sdim + (i-2*Nv)]=1.0/(R_cu_hn*C_cu);
            } else if (i==2*Nc-1) {
                A_th_c[i*sdim + (i-2)]=1.0/(R_cu_hw*C_cu); 
                A_th_c[i*sdim + (i-2*Nv)]=1.0/(R_cu_hn*C_cu);
            }
            // vertical boundaries (west and east)
            else if ((i%(2*Nv)==1)&&(i!=(Nh-1)*2*Nv+1) &&(i!=1)) {
                A_th_c[i*sdim + (i+2)]=1.0/(R_cu_he*C_cu); 
                A_th_c[i*sdim + (i+2*Nv)]=1.0/(R_cu_hs*C_cu);
                A_th_c[i*sdim + (i-2*Nv)]=1.0/(R_cu_hn*C_cu);
            } else if ((((i-1)%(2*Nv)==0) && (i!=2*Nc-1) && (i!=2*Nv-1))) {
                A_th_c[i*sdim + (i-2)]=1.0/(R_cu_hw*C_cu); 
                A_th_c[i*sdim + (i+2*Nv)]=1.0/(R_cu_hs*C_cu);
                A_th_c[i*sdim + (i-2*Nv)]=1.0/(R_cu_hn*C_cu);
            }
            // horizontal boundaries (upper and lower)
            else if((i!=1) && (i<2*Nv-1)) {
                A_th_c[i*sdim + (i-2)]=1.0/(R_cu_hw*C_cu); 
                A_th_c[i*sdim + (i+2)]=1.0/(R_cu_he*C_cu); 
                A_th_c[i*sdim + (i+2*Nv)]=1.0/(R_cu_hs*C_cu);
            } else if ((i>(Nh-1)*2*Nv+1) && (i<2*Nc-1)) {
                A_th_c[i*sdim + (i-2)]=1.0/(R_cu_hw*C_cu); 
                A_th_c[i*sdim + (i+2)]=1.0/(R_cu_he*C_cu); 
                A_th_c[i*sdim + (i-2*Nv)]=1.0/(R_cu_hn*C_cu);
            }
            //internal
            else {
                A_th_c[i*sdim + (i-2)]=1.0/(R_cu_hw*C_cu); 
                A_th_c[i*sdim + (i+2)]=1.0/(R_cu_he*C_cu); 
                A_th_c[i*sdim + (i-2*Nv)]=1.0/(R_cu_hn*C_cu);
                A_th_c[i*sdim + (i+2*Nv)]=1.0/(R_cu_hs*C_cu);           
            }
        }
        //end %single core case

        // Last Columns Poles (airs)
        // HeatSpreader
        if ((i%2)==1) {
            // diagonal terms for vertexes
            if (i==1) {
                A_th_c[i*sdim + end-air_pos]=1.0/C_cu * (1.0/R_cu_hn + 1.0/R_cu_hw);
                A_th_c[(end-air_pos)*sdim + i]=1.0/C_air * (1.0/R_cu_hn + 1.0/R_cu_hw);
            } else if (i==2*Nv-1) {
                A_th_c[i*sdim + end-air_pos]=1.0/C_cu * (1.0/R_cu_hn + 1.0/R_cu_he);
                A_th_c[(end-air_pos)*sdim + i]=1.0/C_air * (1.0/R_cu_hn + 1.0/R_cu_he);
            } else if (i==(Nh-1)*2*Nv+1) {
                A_th_c[i*sdim + end-air_pos]=1.0/C_cu * (1.0/R_cu_hs + 1.0/R_cu_hw);
                A_th_c[(end-air_pos)*sdim + i]=1.0/C_air * (1.0/R_cu_hs + 1.0/R_cu_hw);
            } else if (i==2*Nc-1) {
                A_th_c[i*sdim + end-air_pos]=1.0/C_cu * (1.0/R_cu_hs + 1.0/R_cu_he);
                A_th_c[(end-air_pos)*sdim + i]=1.0/C_air * (1.0/R_cu_hs + 1.0/R_cu_he);
                
            // diagonal terms for vertical (west and east) boundary nodes
            } else if ((i%(2*Nv)==1) && (i!=(Nh-1)*2*Nv+1) && (i!=1)) {
                A_th_c[i*sdim + end-air_pos]=1.0/(C_cu*R_cu_hw);
                A_th_c[(end-air_pos)*sdim + i]=1.0/(C_air*R_cu_hw);
            } else if ((i%(2*Nv)==1) && (i!=2*Nc-1) && (i!=2*Nv-1)) {
                A_th_c[i*sdim + end-air_pos]=1.0/(C_cu*R_cu_he);
                A_th_c[(end-air_pos)*sdim + i]=1.0/(C_air*R_cu_he);
            // diagonal terms for horizontal (upper and lower) boundary nodes
            } else if ((i!=1) && (i<2*Nv-1)) {
                A_th_c[i*sdim + end-air_pos]=1.0/(C_cu*R_cu_hn);
                A_th_c[(end-air_pos)*sdim + i]=1.0/(C_air*R_cu_hn);
            } else if ((i>(Nh-1)*2*Nv+1) && (i<2*Nc-1)) {
                A_th_c[i*sdim + end-air_pos]=1.0/(C_cu*R_cu_hs);
                A_th_c[(end-air_pos)*sdim + i]=1.0/(C_air*R_cu_hs);
            }
            // diagonal terms corresponding to other nodes (internal)
            //else
                //A(i,end)=0;
                //A(end,i)=0;
        }
        // Silicium
        else
        {
            // diagonal terms for vertexes
            if (i==0) {
                A_th_c[i*sdim + end-air_pos]=1.0/C_si * (1.0/R_si_hn + 1.0/R_si_hw);
                A_th_c[(end-air_pos)*sdim + i]=1.0/C_air * (1.0/R_si_hn + 1.0/R_si_hw);					
            } else if (i==2*Nv-2) {
                A_th_c[i*sdim + end-air_pos]=1.0/C_si * (1.0/R_si_hn + 1.0/R_si_he);
                A_th_c[(end-air_pos)*sdim + i]=1.0/C_air * (1.0/R_si_hn + 1.0/R_si_he);	
            } else if (i==(Nh-1)*2*Nv) {
                A_th_c[i*sdim + end-air_pos]=1.0/C_si * (1.0/R_si_hs + 1.0/R_si_hw);
                A_th_c[(end-air_pos)*sdim + i]=1.0/C_air * (1.0/R_si_hs + 1.0/R_si_hw);						
            } else if (i==2*Nc-2) {
                A_th_c[i*sdim + end-air_pos]=1.0/C_si * (1.0/R_si_hs + 1.0/R_si_he);
                A_th_c[(end-air_pos)*sdim + i]=1.0/C_air * (1.0/R_si_hs + 1.0/R_si_he);	
            }
            // diagonal terms for vertical (west and east) boundary nodes
            else if ((i%(2*Nv)==0) && (i!=(Nh-1)*2*Nv) && (i!=0) ) {
                A_th_c[i*sdim + end-air_pos]=1.0/(C_si*R_si_hw);
                A_th_c[(end-air_pos)*sdim + i]=1.0/(C_air*R_si_hw);
            } else if ( ((i+1)%(2*Nv)==1) && (i!=2*Nc-2) && (i!=2*Nv-2)) {
                A_th_c[i*sdim + end-air_pos]=1.0/(C_si*R_si_he);
                A_th_c[(end-air_pos)*sdim + i]=1.0/(C_air*R_si_he);
            // diagonal terms for horizontal (upper and lower) boundary nodes
            } else if ( (i!=0) && (i<2*Nv-3) ) {
                A_th_c[i*sdim + end-air_pos]=1.0/(C_si*R_si_hn);
                A_th_c[(end-air_pos)*sdim + i]=1.0/(C_air*R_si_hn);
            } else if ( (i>(Nh-1)*2*Nv) && (i<2*Nc-2) ) {
                A_th_c[i*sdim + end-air_pos]=1.0/(C_si*R_si_hs);
                A_th_c[(end-air_pos)*sdim + i]=1.0/(C_air*R_si_hs);
            }
            //  diagonal terms corresponding to other nodes (internal)
            //else
                //B(i,end) = 0;
        }

        // THERMAL MODEL 2
        /////////
        // CASE 2
        ////////
        //TODO:
    }

	//HeatSink
	A_th_c[(end-al_pos)*sdim + (end-air_pos)] = 1.0/C_al * (1.0/R_alair_v + 2.0/R_alair_len + 2.0/R_alair_wid) + al_fan_dis/C_al;
	A_th_c[(end-air_pos)*sdim + (end-al_pos)] = 1.0/C_air * (1.0/R_alair_v + 2.0/R_alair_len + 2.0/R_alair_wid) + al_fan_dis/C_al; //here C_al or C_air?
	A_th_c[(end-al_pos)*sdim + (end-al_pos)] = 0;
    double accum = 0;
    for (int i=0;i<sdim;i++)
        accum += A_th_c[(end-al_pos)*sdim + i];
	A_th_c[(end-al_pos)*sdim + (end-al_pos)] = -accum; //-sum(A(end-al_pos,:));
	
	//PCB
	//negligible with air
	A_th_c[(end-pcb_pos)*sdim + (end-mb_pos)] =1.0/(C_pcb*R_pcbmb_v) * pcb_mb_fact;
	A_th_c[(end-mb_pos)*sdim + (end-pcb_pos)] =1.0/(C_mb*R_pcbmb_v) * pcb_mb_fact;
	A_th_c[(end-pcb_pos)*sdim + (end-pcb_pos)] = 0;
    accum = 0;
    for (int i=0;i<sdim;i++)
        accum += A_th_c[(end-pcb_pos)*sdim + i];
	A_th_c[(end-pcb_pos)*sdim + (end-pcb_pos)] = -accum; //-sum(A(end-pcb_pos,:));
	
	//Motherboard
	A_th_c[(end-mb_pos)*sdim + (end-air_pos)] = 1.0/(C_mb * R_mbair_v) * mb_air_fact;
	A_th_c[(end-air_pos)*sdim + (end-mb_pos)] = 1.0/(C_air * R_mbair_v) * mb_air_fact;
	A_th_c[(end-mb_pos)*sdim + (end-mb_pos)] = 0;
    accum = 0;
    for (int i=0;i<sdim;i++)
        accum += A_th_c[(end-mb_pos)*sdim + i];
	A_th_c[(end-mb_pos)*sdim + (end-mb_pos)] = -accum; //-sum(A(end-lMbPos,:));
	
	//Air
	//A(end, end) = -(lNc+(lNh-2)*2+(lNv-2)*2+4*2)/(R_cu_v*C_cu) -100*lNc/4; // -0.149075;
	A_th_c[(end-air_pos)*sdim + (end-air_pos)]=0;
    accum = 0;
    for (int i=0;i<sdim;i++)
        accum += A_th_c[(end-air_pos)*sdim + i];
	A_th_c[(end-air_pos)*sdim + (end-air_pos)] = -accum - case_fan_dis; //-sum(A(end-lAirPos,:)) - case_fan_dis; // -0.149075;

    /* Print Matrix A */
    /*
    for (int i=0; i<sdim*sdim; i++)
    {     
        if (i%(sdim)==0)
            printf("\n\r");
        //printf("%lf ", A_th_c[i]);
        printf("%.8e ", A_th_c[i]);
    }
    printf("\n\r\n\r");
    */

   	// ==============
	// input matrix
	// ==============

    // input matrix
    // inputs: core power (controlled) ambient temperature (disturbance)
    double* B_th_c = (double *)calloc(sdim*udim, sizeof(double));
    if (B_th_c == NULL)
    {
        printf("[Model] Error creating the B continous array\r\n");
        return 1;
    }
    int index=-1;
    for (int i=0; i<2*Nc; i++)
    {
        if ( (i+1)%2 > 0 )
        {
            index++;

            // Core Position
			int ci = i/2;
			int irow = ci/Nv;
			int icol = ci%Nv;
            //printf("ci: %d, irow: %d, icol: %d\n\r", ci , irow, icol);
			
			double li = CPw_fp_dim[(irow*core_cols+icol) + north_pos*fpldim] + CPw_fp_dim[(irow*core_cols+icol) + south_pos*fpldim];
			double wi = CPw_fp_dim[(irow*core_cols+icol) + east_pos*fpldim] + CPw_fp_dim[(irow*core_cols+icol) + west_pos*fpldim];
			double C_core = C_fp_material[((irow+extt_rows)*cols+(icol+extl_cols)) + si_pos*fpdim] * li*wi*t_si;
            //printf("mat: %lf, li: %lf, wi: %lf, t: %lf\n\r", C_fp_material[((irow+extt_rows)*cols+(icol+extl_cols)) + si_pos*fpdim], li, wi, t_si);
			//TODO: Noise
            /*
            if pdev==1
				C_core = C_core * param_dev_per(ci,5);
			end
            */
			
            //TODO: THERMAL MODEL VER
            /*
			if (tm_ver == 1) //|| (tm_ver == 2)
				g2=gaussian_filter(max(Nv, Nh),3)*16;
				g2 = g2 + (1-0.01 - g2(1));
				C_core = C_core / g2(irow-extt_rows,icol-extl_cols);// / 1.5;
			end		
            */					
					
			B_th_c[i*udim + index] = 1.0/C_core * pw2therm_coeff;            
        }
    }

    for (int i=0; i<udim; i++)
        B_th_c[(end-air_pos)*udim + i] = 0;
    B_th_c[(end-air_pos)*udim + udim-1] = case_fan_dis/case_fan_nom_speed;

    /* Print Matrix B */
    /*
    for (int i=0; i<sdim*udim; i++)
    {     
        if (i%udim==0)
            printf("\n\r");
        printf("%lf ", B_th_c[i]);
        //printf("%.8e ", B_th_c[i]);
    }
    */

    // Euler Method:
    // Ad = I + t*A, Bd = T*B
    double* addr = NULL;
    addr = (double *)calloc(sdim*sdim, sizeof(double));
    if (addr == NULL)
    {
        printf("[Model] Error creating the A Discrete array\r\n");
        return 1;
    }
    for (int i=0; i<sdim; i++)
    {
        for (int j=0; j<sdim; j++)
        {
            addr[i*sdim + j] = Ts*A_th_c[i*sdim + j];
            if (i==j)
            {
            	addr[i*sdim + j] += 1.0;
            }
        }
    }
    *A_th = addr;

    addr = (double *)calloc(sdim*udim, sizeof(double));
    if (addr == NULL)
    {
        printf("[Model] Error creating the B Discrete array\r\n");
        return 1;
    }
    for (int i=0; i<sdim; i++)
    {
        for (int j=0; j<udim; j++)
        {
            addr[i*udim + j] = Ts*B_th_c[i*udim + j];
        }
    }
    *B_th = addr;

    free(A_th_c);
    free(B_th_c);

    return 0;
}

void matMul (double* O, const double* iA, double* iB, int r1, int col1, int col2)
{
	int r2 = col1;
	double sum = 0;

	for (int i = 0; i < r1; i++)
		for (int j = 0; j < col2; j++)
		{
			for (int k = 0; k < r2; k++)
			{
				sum += iA[i*col1 + k] * iB[k*col2 + j];
			}

			O[i*col2 + j] = sum;
			sum = 0.0;
		}

}

void matSum (double* O, double* iA, double* iB, int r1, int col1 )
{
	for (int i = 0; i < r1; i++)
		for (int j = 0; j < col1; j++)
		{
			O[i*col1 + j] = iA[i*col1 + j] + iB[i*col1 + j];
		}

}

void generateGaussianNoise(float *z0, float *z1, float mu, float sigma)
{
	//srand((unsigned) time(NULL));

	float u1, u2;

	//create two random numbers, make sure u1 is greater than epsilon
	//do 
	//{
	u1 = ((float)rand()/(float)(RAND_MAX)) /* *1.0f */;
	//}while(u1 <= 1.0f - nextafter(u1, INFINITY));
	u2 = ((float)rand()/(float)(RAND_MAX)) /* *1.0f */;

	//compute z0 and z1
    float mag = sigma * (float)sqrt(-2.0 * log(u1));
    if ( z0 != NULL)
    	*z0  = mag * (float)cos(2*M_PI * u2) + mu;
    if (z1 != NULL)
    	*z1  = mag * (float)sin(2*M_PI * u2) + mu;

	//srand((unsigned) time(NULL));
	/*
    if (((float)rand()/(float)(RAND_MAX)) >= 0.5f)
    	return z0;
    else
    	return z1;
	*/
}
