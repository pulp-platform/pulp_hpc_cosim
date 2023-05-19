
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



/*
 * model.c
 *
 *  Created on: 19 feb 2020
 *      Author: giova
 */

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

//TODO create a clear model function to free(A), and free(B) and all others

void model_initialization(int Nc, int Nh, int Nv, uint32_t Ts)
{

	if (create_thermal_model(&A, &B, Nc, Nh, Nv, Ts))
	{
		exit(1);
	}

    /*
    for(int t=0; t<N_HPC_CORE*2*N_HPC_CORE*2; t++)
    {
        if (A[t] - A_test[t] > 1e-3)
            printf("A[%d][%d]: %f / %f \n\r", (t/(N_HPC_CORE*2)), (t%(N_HPC_CORE*2)), A[t], A_test[t]);
        if (B[t] - B_test[t] > 1e-3)
            printf("B[%d][%d]: %f / %f \n\r", (t/(N_HPC_CORE*2)), (t%(N_HPC_CORE*2)), B[t], B_test[t]);
    }
    */

	//Initialization
	inputMat = (double*)malloc(sizeof(double)*Nc+1);
	tempInput = (double*)malloc(sizeof(double)*Nc*2);
	tempState = (double*)malloc(sizeof(double)*Nc*2);
    state = (double*)malloc(sizeof(double)*Nc*2);
    //TODO, deinit??

	for (int i=0; i< Nc*2; i++)
	{
		state[i] = (double)Tamb;
	}
}


void model_step(float* oTemp, float *oDomainPower, float (*iWlPerc)[WL_STATES])
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
            inputMat[core_count] = (double)(power * ThermalCorrection); //after, because we want it to affect just the thermal model
            //printf("core: %d, i: %d, id: %d\n\r", app, i, simulation.elements[i].id);
            core_count++; //here I assumed all cores are in a sequence, no core position
        }
        //inputMat[i] = (double)(power * ThermalCorrection); //after, because we want it to affect just the thermal model
	}

	//Finishing populating input matrix
	inputMat[simulation.nb_cores] = (double)Tamb;

	//State Generation (A*X(t))
	matMul(tempState, A, state, (simulation.nb_cores*2), (simulation.nb_cores*2), 1 );

	//Model Temp Computation (B*u(t))
	matMul(tempInput, B, inputMat, (simulation.nb_cores*2), (simulation.nb_cores+1), 1);
	matSum(state, tempState, tempInput, simulation.nb_cores*2, 1);

	//State Update - copying
	//for(int i=0; i< Nc*2; i++)
	//	state[i] = tempState[i];

	//Output generation:
	for (int i = 0; i < simulation.nb_cores; i++)
	{
#ifdef CCE_1 //Noise
		oTemp[i] = (float)state[i*2] + TempSensorNoise[NoiseIndex];
#else
		oTemp[i] = (float)state[i*2];
#endif
		//Index overflow! Do not uncomment!
		//oTemp[simulation.nb_cores+i] = state[i*2];
	}

	return;
}


int create_thermal_model(double** A_th, double** B_th, int Nc, int Nh, int Nv, uint32_t Ts)
{
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

    // Single tile thermal parameters
    double R_si_v=5; // K/W silicon vertical (towards heat spreader) thermal resistance
    double R_cu_v=290; // K/W copper vertical (towards ambient) thermal resistance
    double R_si_h=42; //22.9; // K/W silicon horizontal (towards neighbours) thermal resistance
    double R_cu_h=2*1.2; // K/W copper horizontal (towards neighbours) thermal resistance

    double C_si=1e-3; // Silicon part thermal capacitance
    double C_cu=1.2e-2; // copper part thermal capacitance

    //State Matrix assiming each node is a second order sub-system (silicon and heater dynamics) interacting with neighbours and T_amb
    double* A_th_c = (double *)calloc(2*Nc*2*Nc, sizeof(double));
    if (A_th_c == NULL)
    {
        printf("[Model] Error creating the A continous array\r\n");
        return 1;
    }
    for (int i=0; i<2*Nc; i++)
    {
        for (int j=0; j<2*Nc; j++)
        {       
            //int ooo = i*2*Nc + j;          
            //if ( (ooo==128) || (ooo == 212) || (ooo == 296) )    
            //    printf("Debug!!\r\n");

            //diagonal terms

            // Silicon (die temp dyn)
            //diagonal terms for "vertex nodes" 
            if ((i==j) && ((i+1)%2>0))
            {
                if ( (i==0) || (i==2*Nc-2) || (i==2*Nv-2) || (i==(Nh-1)*2*Nv) )
                    A_th_c[i*2*Nc + j]=-1/(R_si_v*C_si)-2/(R_si_h*C_si);
                    
                //diagonal terms for vertical (upper and lower) boundary nodes
                else if ( ( ((i+1)%(2*Nv)==1)&&(i!=(Nh-1)*2*Nv)&&(i!=0) ) || ( (((i+2)%(2*Nv))==0)&&(i!=2*Nc-2)&&(i!=2*Nv-2) ) )
                    A_th_c[i*2*Nc + j]=-1/(R_si_v*C_si)-3/(R_si_h*C_si);
            
                //diagonal terms for horizontal (upper and lower) boundary nodes
                else if ( ( (i!=0)&&(i<2*Nv-3) ) || ((i==j)&&((i+1)%2>0)&&(i>(Nh-1)*2*Nv)&&(i<2*Nc-2) ) )
                    A_th_c[i*2*Nc + j]=-1/(R_si_v*C_si)-3/(R_si_h*C_si);
                
                //diagonal terms corresponding to other nodes (internal)
                else
                    A_th_c[i*2*Nc + j]=-1/(R_si_v*C_si)-4/(R_si_h*C_si);
            }              
                
            //Heat-Spread (Cooper dyn)
                
            //diagonal terms "vertex"
            if ( (i==j) && ((i+1)%2==0) )
            {                    
                if ( (i==1) || (i==2*Nc-1) || (i==2*Nv-1) || (i==(Nh-1)*2*Nv+1) )
                    A_th_c[i*2*Nc + j]=(-1/(R_si_v*C_cu)-1/(R_cu_v*C_si)-2/(R_cu_h*C_cu));
                    
                //diagonal terms for vertical (upper and lower) boundary nodes
                else if ( ( ((i+1)%(2*Nv)==2)&&(i!=(Nh-1)*2*Nv+1)&&(i!=1) ) || ( (((i+1)%(2*Nv)==0)&&(i!=2*Nc-1)&&(i!=2*Nv-1)) ) )
                    A_th_c[i*2*Nc + j]=(-1/(R_si_v*C_cu)-1/(R_cu_v*C_si)-3/(R_cu_h*C_cu));
            
                //diagonal terms for horizontal (upper and lower) boundary nodes
                else if ( ( (i!=1)&&(i<2*Nv-1) ) || ( (i>(Nh-1)*2*Nv+1)&&(i<2*Nc-1) ) )
                    A_th_c[i*2*Nc + j]=(-1/(R_si_v*C_cu)-1/(R_cu_v*C_si)-3/(R_cu_h*C_cu));
                
                //diagonal terms corresponding to other nodes (internal)
                else
                    A_th_c[i*2*Nc + j]=(-1/(R_si_v*C_cu)-1/(R_cu_v*C_si)-4/(R_cu_h*C_cu));
            }               
                
            //Vertical coupling terms btween silicon and copper (flow from die to spreader)                
            if ( ((i+1)%2>0) && (j==i+1) )
                A_th_c[i*2*Nc + j]=1/(R_si_v*C_si);
            
            if ( ((i+1)%2==0) && (j==i-1) )
                A_th_c[i*2*Nc + j]=1/(R_si_v*C_cu);
            
            //Horizontal coupling of die thermal dyn (silicon)
            if ((i+1)%2 > 0)
            {
                //vertici
                if (i==0)
                {
                    A_th_c[i*2*Nc + i+2]=1/(R_si_h*C_si); 
                    A_th_c[i*2*Nc + i+2*Nv]=1/(R_si_h*C_si);
                }                        
                else if (i==2*Nv-2)
                {
                    A_th_c[i*2*Nc + i-2]=1/(R_si_h*C_si); 
                    A_th_c[i*2*Nc + i+2*Nv]=1/(R_si_h*C_si);
                }
                else if (i==(Nh-1)*2*Nv)
                {
                    A_th_c[i*2*Nc + i+2]=1/(R_si_h*C_si); 
                    A_th_c[i*2*Nc + i-2*Nv]=1/(R_si_h*C_si);
                }                        
                else if (i==2*Nc-2)
                {
                    A_th_c[i*2*Nc + i-2]=1/(R_si_h*C_si); 
                    A_th_c[i*2*Nc + i-2*Nv]=1/(R_si_h*C_si);
                }
                //bordi verticali
                else if ( ((i+1)%(2*Nv)==1) && (i!=(Nh-1)*2*Nv) && (i!=0) )
                {
                    A_th_c[i*2*Nc + i+2]=1/(R_si_h*C_si); 
                    A_th_c[i*2*Nc + i+2*Nv]=1/(R_si_h*C_si);
                    A_th_c[i*2*Nc + i-2*Nv]=1/(R_si_h*C_si);
                }                        
                else if ( ((i+2)%(2*Nv)==0) && (i!=2*Nc-2) && (i!=2*Nv-2) )
                {
                    A_th_c[i*2*Nc + i-2]=1/(R_si_h*C_si); 
                    A_th_c[i*2*Nc + i+2*Nv]=1/(R_si_h*C_si);
                    A_th_c[i*2*Nc + i-2*Nv]=1/(R_si_h*C_si);
                }
                        
                //horizonatal boundaries                
                else if ( (i!=0) && (i<2*Nv-3) )
                {
                    A_th_c[i*2*Nc + i-2]=1/(R_si_h*C_si); 
                    A_th_c[i*2*Nc + i+2]=1/(R_si_h*C_si); 
                    A_th_c[i*2*Nc + i+2*Nv]=1/(R_si_h*C_si);
                }                
                else if ( (i>(Nh-1)*2*Nv) && (i<2*Nc-2) )
                {
                    A_th_c[i*2*Nc + i-2]=1/(R_si_h*C_si); 
                    A_th_c[i*2*Nc + i+2]=1/(R_si_h*C_si); 
                    A_th_c[i*2*Nc + i-2*Nv]=1/(R_si_h*C_si);
                }
                else
                {
                    A_th_c[i*2*Nc + i-2]=1/(R_si_h*C_si); 
                    A_th_c[i*2*Nc + i+2]=1/(R_si_h*C_si); 
                    A_th_c[i*2*Nc + i-2*Nv]=1/(R_si_h*C_si);
                    A_th_c[i*2*Nc + i+2*Nv]=1/(R_si_h*C_si);           
                }
            }
            
            // horizontal coupling of sperader thermal dyn (copper)
            if ((i+1)%2==0)
            {
                // vertex
                if (i==1)
                {
                    A_th_c[i*2*Nc + i+2]=1/(R_cu_h*C_cu); 
                    A_th_c[i*2*Nc + i+2*Nv]=1/(R_cu_h*C_cu);
                }        
                else if (i==2*Nv-1)
                {
                    A_th_c[i*2*Nc + i-2]=1/(R_cu_h*C_cu); 
                    A_th_c[i*2*Nc + i+2*Nv]=1/(R_cu_h*C_cu);
                }
                else if (i==(Nh-1)*2*Nv+1) 
                {
                    A_th_c[i*2*Nc + i+2]=1/(R_cu_h*C_cu); 
                    A_th_c[i*2*Nc + i-2*Nv]=1/(R_cu_h*C_cu);
                }                        
                else if (i==2*Nc-1) 
                {
                    A_th_c[i*2*Nc + i-2]=1/(R_cu_h*C_cu); 
                    A_th_c[i*2*Nc + i-2*Nv]=1/(R_cu_h*C_cu);
                }        
                // vertical boundaries
                else if ( ((i+1)%(2*Nv)==2) && (i!=(Nh-1)*2*Nv+1) && (i!=1) )
                { 
                    A_th_c[i*2*Nc + i+2]=1/(R_cu_h*C_cu); 
                    A_th_c[i*2*Nc + i+2*Nv]=1/(R_cu_h*C_cu);
                    A_th_c[i*2*Nc + i-2*Nv]=1/(R_cu_h*C_cu);
                }        
                else if ( ((i+1)%(2*Nv)==0) && (i!=2*Nc-1) && (i!=2*Nv-1) )
                {
                    A_th_c[i*2*Nc + i-2]=1/(R_cu_h*C_cu); 
                    A_th_c[i*2*Nc + i+2*Nv]=1/(R_cu_h*C_cu);
                    A_th_c[i*2*Nc + i-2*Nv]=1/(R_cu_h*C_cu);
                }        
                
                //horizontal boundaries                
                else if ( (i!=1) && (i<2*Nv-1) )
                {
                    A_th_c[i*2*Nc + i-2]=1/(R_cu_h*C_cu); 
                    A_th_c[i*2*Nc + i+2]=1/(R_cu_h*C_cu); 
                    A_th_c[i*2*Nc + i+2*Nv]=1/(R_cu_h*C_cu);
                }
                else if ( (i>(Nh-1)*2*Nv+1) && (i<2*Nc-1) )
                {
                    A_th_c[i*2*Nc + i-2]=1/(R_cu_h*C_cu); 
                    A_th_c[i*2*Nc + i+2]=1/(R_cu_h*C_cu); 
                    A_th_c[i*2*Nc + i-2*Nv]=1/(R_cu_h*C_cu);
                }
                else
                {
                    A_th_c[i*2*Nc + i-2]=1/(R_cu_h*C_cu); 
                    A_th_c[i*2*Nc + i+2]=1/(R_cu_h*C_cu); 
                    A_th_c[i*2*Nc + i-2*Nv]=1/(R_cu_h*C_cu);
                    A_th_c[i*2*Nc + i+2*Nv]=1/(R_cu_h*C_cu);           
                }
            }                 
        } //for
    } //for

    // input matrix
    // inputs: core power (controlled) ambient temperature (disturbance)
    double* B_th_c = (double *)calloc(2*Nc*(Nc+1), sizeof(double));
    if (B_th_c == NULL)
    {
        printf("[Model] Error creating the B continous array\r\n");
        return 1;
    }
    int index=0;
    for (int i=0; i<2*Nc; i++)
    {
        if ((i+1)%2==0)
        {
            for (int j=0; j<Nc; j++)
            {
                B_th_c[i*(Nc+1) + j]= 0;
            }
            B_th_c[i*(Nc+1) + Nc] = 1/(R_cu_v*C_si);
        }

        if ( (i+1)%2 > 0 )
        {
            B_th_c[i*(Nc+1) + index] = 1/C_si;
            index++;
        }
    }

    // Euler Method:
    // Ad = I + t*A, Bd = T*B
    double T = ((double)Ts)/1e9;
    double* addr = NULL;
    addr = (double *)calloc(2*Nc*2*Nc, sizeof(double));
    if (addr == NULL)
    {
        printf("[Model] Error creating the A Discrete array\r\n");
        return 1;
    }
    for (int i=0; i<2*Nc; i++)
    {
        for (int j=0; j<2*Nc; j++)
        {
            addr[i*2*Nc + j] = T*A_th_c[i*2*Nc + j];
            if (i==j)
            {
            	addr[i*2*Nc + j] += 1.0;
            }
        }
    }
    *A_th = addr;

    addr = (double *)calloc(2*Nc*(Nc+1), sizeof(double));
    if (addr == NULL)
    {
        printf("[Model] Error creating the B Discrete array\r\n");
        return 1;
    }
    for (int i=0; i<2*Nc; i++)
    {
        for (int j=0; j<Nc+1; j++)
        {
            addr[i*(Nc+1) + j] = T*B_th_c[i*(Nc+1) + j];
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
