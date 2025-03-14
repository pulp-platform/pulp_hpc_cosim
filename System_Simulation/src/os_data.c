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

//OS
#include "os_data.h"

//Govern

//other
#include "csv.h"

input_data_t* TargetFrequency;
input_data_t* QuadPwrBudget;
input_data_t* BoardPwrBudget;
input_data_bm_t* BindMatrix;

int N_FREQ_SEQ = 0;
int N_PW_BUDGET_SEQ  = 0;
int N_BINDING_MAT_SEQ = 0;

void init_os(void)
{

	char* row;
	/*const*/ char* col;
	int nb_cols, nb_rows, counter_cols, counter_rows;
	int prev_time;
	CsvHandle handle;

	/*** Target Frequency ***/
	 
	handle = CsvOpen("System_Simulation/sym_inputs/target_freq.csv");
	//CsvHandle handle = CsvOpen2("System_Simulation/sym_inputs/target_freq.csv", ';', '"', '\'');
	if (!handle)
	{
		printf("[CSV] Error opening the Target Frequency file\r\n");
 		//TODO, use default values
 		//exit(1);
	}

	/* read first row */
	row = CsvReadNextRow(handle);
	nb_cols = 0;
	while (col = CsvReadNextCol(row, handle))
	    {nb_cols++;}

	nb_cols -= 1;

	if (nb_cols != simulation.nb_elements)
	{
		printf("[CSV] Error: Target Frequency file has a different number of columns (%d) wrt the components in the Model (%d)\r\n", nb_cols, simulation.nb_elements);
 		//TODO, use default values
 		//exit(1);
	}

	/* check number of rows */
	nb_rows = 0;
	while (row = CsvReadNextRow(handle))
		{nb_rows++;}

	TargetFrequency = (input_data_t*)malloc(sizeof(input_data_t)*nb_cols*nb_rows); //TODO Check this! is it nb_* or simulation_elements..?
	N_FREQ_SEQ = nb_rows;
	//printf("freq: %d\n", nb_rows);

	/* Reset file */
	CsvClose(handle);

	/* data reading */
	/* Re-Open file */
	handle = CsvOpen("System_Simulation/sym_inputs/target_freq.csv");
	//CsvHandle handle = CsvOpen2("System_Simulation/sym_inputs/target_freq.csv", ';', '"', '\'');
	if (!handle)
	{
		printf("[CSV] Error opening the Target Frequency file (second time)\r\n");
 		//TODO, use default values
 		//exit(1);
	}

	/* skip first row */
	row = CsvReadNextRow(handle);

	counter_rows = 0;
	prev_time = 0;
	while (row = CsvReadNextRow(handle))
	{
	    /* row = CSV row string */
	    const char* col;
	    counter_cols = 0;

	    /* Read time */
	    col = CsvReadNextCol(row, handle);
	   	int csv_time = atoi(col);
	    int time = csv_time - prev_time;
		prev_time = csv_time;

	    while (col = CsvReadNextCol(row, handle))
	    {
	        /* col = CSV col string */
	        TargetFrequency[counter_cols*nb_rows + counter_rows].time = time; //TODO Check this! is it nb_* or simulation_elements..?
	  		//printf("%d/%d: %d\n", counter_rows, counter_cols, TargetFrequency[counter_cols*nb_rows + counter_rows].time);
	        TargetFrequency[counter_cols*nb_rows + counter_rows].value = atof(col); //TODO Check this! is it nb_* or simulation_elements..?
	        //printf("%d/%d: %f\n", counter_rows, counter_cols, TargetFrequency[counter_cols*nb_rows + counter_rows].value);

	        counter_cols++;
	    }

	    counter_rows++;
	}

	/* Close file */
	CsvClose(handle);


/**********************************************************/


	/*** Power Budget ***/
	 
	//handle = CsvOpen("System_Simulation/sym_inputs/power_budget.csv");
	handle = CsvOpen2("System_Simulation/sym_inputs/power_budget.csv", ';', '"', '\'');
	if (!handle)
	{
		printf("[CSV] Error opening the Power Budget file\r\n");
 		//TODO, use default values
 		//exit(1);
	}

	/* read first row */
	row = CsvReadNextRow(handle);
	nb_cols = 0;
	while (col = CsvReadNextCol(row, handle))
	    {nb_cols++;}

	nb_cols -= 1;
	//subtract also board power
	nb_cols -= 1;

	if (nb_cols != simulation.nb_power_domains)
	{
		printf("[CSV] Error: Power Budget file has a different number of columns (%d) wrt the power domains in the Model (%d)\r\n", nb_cols, simulation.nb_power_domains);
 		//TODO, use default values
 		//exit(1);
	}

	/* check number of rows */
	nb_rows = 0;
	while (row = CsvReadNextRow(handle))
		{nb_rows++;}

	BoardPwrBudget = (input_data_t*)malloc(sizeof(input_data_t)*nb_rows); //TODO Check this! is it nb_* or simulation_elements..?
	QuadPwrBudget = (input_data_t*)malloc(sizeof(input_data_t)*nb_rows*nb_cols); //TODO Check this! is it nb_* or simulation_elements..?
	N_PW_BUDGET_SEQ = nb_rows;
	//printf("pw: %d\n", nb_rows);

	/* Reset file */
	CsvClose(handle);

	/* data reading */
	/* Re-Open file */
	//handle = CsvOpen("System_Simulation/sym_inputs/power_budget.csv");
	handle = CsvOpen2("System_Simulation/sym_inputs/power_budget.csv", ';', '"', '\'');
	if (!handle)
	{
		printf("[CSV] Error opening the Power Budget file (second time)\r\n");
 		//TODO, use default values
 		//exit(1);
	}

	/* skip first row */
	row = CsvReadNextRow(handle);

	counter_rows = 0;
	prev_time = 0;
	while (row = CsvReadNextRow(handle))
	{
	    /* row = CSV row string */
	    const char* col;
	    counter_cols = 0;

	    /* Read time */
	    col = CsvReadNextCol(row, handle);
	   	int csv_time = atoi(col);
	    int time = csv_time - prev_time;
		prev_time = csv_time;

	    BoardPwrBudget[counter_rows].time = time;

	    /* Read Board Power */
	    col = CsvReadNextCol(row, handle);
	    BoardPwrBudget[counter_rows].value = atof(col);
	    //printf("%d/%d: %f\n", counter_rows, counter_cols, BoardPwrBudget[counter_rows].value);

	    while (col = CsvReadNextCol(row, handle))
	    {
	        /* col = CSV col string */
		    QuadPwrBudget[counter_cols*nb_rows + counter_rows].time = time; //TODO Check this! is it nb_* or simulation_elements..?
		    //printf("%d/%d: %d\n", counter_rows, counter_cols, QuadPwrBudget[counter_cols*nb_rows + counter_rows].time);
	        QuadPwrBudget[counter_cols*nb_rows + counter_rows].value = atof(col); //TODO Check this! is it nb_* or simulation_elements..?
	        //printf("%d/%d: %f\n", counter_rows, counter_cols, QuadPwrBudget[counter_cols*nb_rows + counter_rows].value);

	        counter_cols++;
	    }

	    counter_rows++;
	}

	/* Close file */
	CsvClose(handle);


/**********************************************************/

	
	/*** Binding Matrix ***/
	 
	//handle = CsvOpen("System_Simulation/sym_inputs/bind_mat.csv");
	handle = CsvOpen2("System_Simulation/sym_inputs/bind_mat.csv", ';', '"', '\'');
	if (!handle)
	{
		printf("[CSV] Error opening the Binding Matrix file\r\n");
 		//TODO, use default values
 		//exit(1);
	}

	/* read first row */
	row = CsvReadNextRow(handle);
	nb_cols = 0;
	while (col = CsvReadNextCol(row, handle))
	    {nb_cols++;}

	nb_cols -= 1;

	if (nb_cols != simulation.nb_cores)
	{
		printf("[CSV] Error: Binding Matrix file has a different number of columns (%d) wrt the number of cores in the Model (%d)\r\n", nb_cols, simulation.nb_cores);
 		//TODO, use default values
 		//exit(1);
	}

	/* check number of rows */
	nb_rows = 0;
	while (row = CsvReadNextRow(handle))
		{nb_rows++;}

	BindMatrix = (input_data_bm_t*)malloc(sizeof(input_data_bm_t)*nb_cols*nb_rows); //TODO Check this! is it nb_* or simulation_elements..?
	N_BINDING_MAT_SEQ = nb_rows;
	//printf("bind: %d\n", nb_rows);

	/* Reset file */
	CsvClose(handle);

	/* data reading */
	/* Re-Open file */
	//handle = CsvOpen("System_Simulation/sym_inputs/bind_mat.csv");
	handle = CsvOpen2("System_Simulation/sym_inputs/bind_mat.csv", ';', '"', '\'');
	if (!handle)
	{
		printf("[CSV] Error opening the Binding Matrix file (second time)\r\n");
 		//TODO, use default values
 		//exit(1);
	}

	/* skip first row */
	row = CsvReadNextRow(handle);

	counter_rows = 0;
	prev_time = 0;
	while (row = CsvReadNextRow(handle))
	{
	    /* row = CSV row string */
	    const char* col;
	    counter_cols = 0;

	    /* Read time */
	    col = CsvReadNextCol(row, handle);
	    int csv_time = atoi(col);
	    int time = csv_time - prev_time;
		prev_time = csv_time;

	    while (col = CsvReadNextCol(row, handle))
	    {
	        /* col = CSV col string */
    	    BindMatrix[counter_cols*nb_rows + counter_rows].time = time; //TODO Check this! is it nb_* or simulation_elements..?
	    	//printf("%d/%d: %d\n", counter_rows, counter_cols, BindMatrix[counter_cols*nb_rows + counter_rows].time);
	        BindMatrix[counter_cols*nb_rows + counter_rows].value = atoi(col); //TODO Check this! is it nb_* or simulation_elements..?
	        //printf("%d/%d: %d\n", counter_rows, counter_cols, BindMatrix[counter_cols*nb_rows + counter_rows].value);

	        counter_cols++;
	    }

	    counter_rows++;
	}

	/* Close file */
	CsvClose(handle);

}

/*
const input_data_t TargetFrequency[Nc][N_FREQ_SEQ] = 
														{ { {.value=1.20, .time=50}, {.value=3.40, .time=7000},  {.value=2.70, .time=1000}, {.value=2.10, .time=1000}, {.value=2.70, .time=750}, {.value=2.70, .time=1000}, {.value=2.70, .time=500}, {.value=2.10, .time=1000}, {.value=2.70, .time=750}, {.value=2.70, .time=500},
															{.value=3.00, .time=1500}, {.value=0, .time=0} },
														{ {.value=1.20, .time=50}, {.value=3.40, .time=7000},  {.value=3.40, .time=500}, {.value=2.70, .time=1000}, {.value=2.10, .time=750}, {.value=2.70, .time=1000}, {.value=3.00, .time=1000}, {.value=2.70, .time=1000}, {.value=2.70, .time=1000}, {.value=3.00, .time=1000},
															{.value=2.70, .time=750}, {.value=0, .time=0} },
														{ {.value=1.20, .time=50}, {.value=3.40, .time=7000},  {.value=3.40, .time=500}, {.value=3.40, .time=1000}, {.value=2.10, .time=750}, {.value=3.40, .time=1000}, {.value=2.70, .time=1000}, {.value=3.40, .time=500}, {.value=3.00, .time=500}, {.value=2.70, .time=750},
															{.value=3.00, .time=2000}, {.value=0, .time=0} },
														{ {.value=1.20, .time=50}, {.value=3.40, .time=7000},  {.value=2.70, .time=1000}, {.value=2.70, .time=500}, {.value=2.10, .time=1000}, {.value=2.70, .time=1000}, {.value=2.10, .time=1000}, {.value=2.70, .time=750}, {.value=2.70, .time=750}, {.value=3.00, .time=500},
															{.value=2.70, .time=1500}, {.value=0, .time=0} },
														{ {.value=1.20, .time=50}, {.value=3.40, .time=7000},  {.value=2.10, .time=750}, {.value=3.40, .time=1000}, {.value=3.40, .time=500}, {.value=2.70, .time=750}, {.value=2.70, .time=1000}, {.value=2.70, .time=750}, {.value=3.00, .time=750}, {.value=2.70, .time=1000},
															{.value=2.70, .time=1500}, {.value=0, .time=0} },
														{ {.value=1.20, .time=50}, {.value=3.40, .time=7000},  {.value=2.70, .time=750}, {.value=3.40, .time=1000}, {.value=2.70, .time=1000}, {.value=3.40, .time=1000}, {.value=2.70, .time=500}, {.value=2.70, .time=750}, {.value=2.70, .time=750}, {.value=2.70, .time=1000},
															{.value=3.00, .time=1250}, {.value=0, .time=0} }, //6 
														{ {.value=1.20, .time=50}, {.value=3.40, .time=7000},  {.value=3.40, .time=1000}, {.value=2.70, .time=750}, {.value=2.70, .time=750}, {.value=2.70, .time=500}, {.value=3.00, .time=1000}, {.value=3.00, .time=750}, {.value=2.70, .time=1000}, {.value=2.10, .time=1000},
															{.value=3.00, .time=1250}, {.value=0, .time=0} },
														{ {.value=1.20, .time=50}, {.value=3.40, .time=7000},  {.value=2.70, .time=1000}, {.value=2.70, .time=1000}, {.value=3.40, .time=1000}, {.value=2.70, .time=1000}, {.value=3.40, .time=500}, {.value=3.00, .time=750}, {.value=3.00, .time=750}, {.value=2.70, .time=1000}, 
															{.value=3.00, .time=1000}, {.value=0, .time=0} },
														{ {.value=1.20, .time=50}, {.value=3.40, .time=7000},  {.value=3.00, .time=1000}, {.value=2.70, .time=750}, {.value=2.70, .time=1000}, {.value=2.70, .time=1000}, {.value=2.70, .time=1000}, {.value=2.70, .time=1000}, {.value=3.00, .time=1000}, {.value=3.00, .time=750},
															{.value=2.70, .time=500}, {.value=0, .time=0} }, 
														{ {.value=1.20, .time=50}, {.value=3.40, .time=7000},  {.value=2.70, .time=1000}, {.value=3.00, .time=750}, {.value=2.70, .time=1000}, {.value=2.70, .time=1000}, {.value=3.40, .time=500}, {.value=3.00, .time=750}, {.value=3.00, .time=1000}, {.value=2.70, .time=1000},
															{.value=2.70, .time=1000}, {.value=0, .time=0} }, 
														{ {.value=1.20, .time=50}, {.value=3.40, .time=7000},  {.value=2.70, .time=1000}, {.value=3.00, .time=1000}, {.value=2.70, .time=1000}, {.value=3.40, .time=500}, {.value=2.70, .time=500}, {.value=3.40, .time=1000}, {.value=2.70, .time=750}, {.value=2.70, .time=750},
															{.value=3.00, .time=1500}, {.value=0, .time=0} }, 
														{ {.value=1.20, .time=50}, {.value=3.40, .time=7000},  {.value=3.40, .time=500}, {.value=2.70, .time=500}, {.value=2.70, .time=1000}, {.value=3.40, .time=1000}, {.value=3.00, .time=1000}, {.value=2.10, .time=750}, {.value=2.70, .time=500}, {.value=3.00, .time=1000},
															{.value=3.00, .time=1750}, {.value=0, .time=0} }, //12 
														{ {.value=1.20, .time=50}, {.value=3.40, .time=7000},  {.value=2.70, .time=750},  {.value=2.70, .time=750}, {.value=3.40, .time=1000}, {.value=3.00, .time=750}, {.value=2.70, .time=1000}, {.value=3.40, .time=500}, {.value=2.70, .time=1000}, {.value=2.10, .time=1000},
															{.value=2.70, .time=1250}, {.value=0, .time=0} }, 
														{ {.value=1.20, .time=50}, {.value=3.40, .time=7000},  {.value=3.40, .time=1000}, {.value=2.70, .time=1000}, {.value=3.40, .time=500}, {.value=2.70, .time=500}, {.value=3.00, .time=1000}, {.value=3.00, .time=750}, {.value=3.00, .time=1000}, {.value=3.00, .time=750},
															{.value=3.40, .time=1500}, {.value=0, .time=0} },
														{ {.value=1.20, .time=50}, {.value=3.40, .time=7000},  {.value=2.70, .time=500}, {.value=3.00, .time=1000}, {.value=3.00, .time=750}, {.value=2.70, .time=1000}, {.value=2.10, .time=1000}, {.value=2.70, .time=750}, {.value=3.40, .time=1000}, {.value=3.00, .time=1000},
															{.value=3.40, .time=1000}, {.value=0, .time=0} }, 
														{ {.value=1.20, .time=50}, {.value=3.40, .time=7000},  {.value=3.00, .time=1000}, {.value=3.40, .time=1000}, {.value=2.70, .time=750}, {.value=2.70, .time=1000}, {.value=2.70, .time=1000}, {.value=3.40, .time=1000}, {.value=2.10, .time=500}, {.value=3.40, .time=500},
															{.value=2.70, .time=1250}, {.value=0, .time=0} },
														{ {.value=1.20, .time=50}, {.value=3.40, .time=7000},  {.value=3.00, .time=1000}, {.value=2.10, .time=750}, {.value=2.70, .time=500}, {.value=3.40, .time=500}, {.value=3.40, .time=1000}, {.value=2.10, .time=750}, {.value=2.70, .time=1000}, {.value=3.40, .time=500},
															{.value=2.70, .time=2000}, {.value=0, .time=0} }, 
														{ {.value=1.20, .time=50}, {.value=3.40, .time=7000},  {.value=2.70, .time=500}, {.value=3.40, .time=500}, {.value=2.10, .time=500}, {.value=3.40, .time=500}, {.value=2.70, .time=500}, {.value=2.70, .time=1000}, {.value=3.40, .time=1000}, {.value=2.10, .time=500},
															{.value=2.70, .time=3000}, {.value=0, .time=0} }, //18 
														{ {.value=1.20, .time=50}, {.value=3.40, .time=7000},  {.value=3.00, .time=750}, {.value=2.70, .time=1000}, {.value=3.40, .time=500}, {.value=3.00, .time=1000}, {.value=3.40, .time=1000}, {.value=2.70, .time=750}, {.value=3.40, .time=1000}, {.value=2.10, .time=500},
															{.value=3.00, .time=1500}, {.value=0, .time=0} }, 
														{ {.value=1.20, .time=50}, {.value=3.40, .time=7000},  {.value=2.70, .time=1000}, {.value=3.40, .time=500}, {.value=3.00, .time=750}, {.value=2.10, .time=1000},  {.value=3.00, .time=1000}, {.value=2.70, .time=750}, {.value=2.70, .time=500}, {.value=3.00, .time=1000},
															{.value=3.00, .time=1500}, {.value=0, .time=0} },
														{ {.value=1.20, .time=50}, {.value=3.40, .time=7000},  {.value=2.70, .time=750}, {.value=2.70, .time=1000}, {.value=2.70, .time=750}, {.value=2.70, .time=750},  {.value=3.40, .time=1000}, {.value=3.00, .time=1000}, {.value=3.00, .time=1000}, {.value=3.40, .time=1000},
															{.value=2.70, .time=750}, {.value=0, .time=0} }, 
														{ {.value=1.20, .time=50}, {.value=3.40, .time=7000},  {.value=2.10, .time=1000}, {.value=2.70, .time=750}, {.value=3.00, .time=1000}, {.value=3.00, .time=1000}, {.value=3.40, .time=1000}, {.value=2.70, .time=1000}, {.value=3.40, .time=1000}, {.value=2.70, .time=500},
															{.value=3.00, .time=750}, {.value=0, .time=0} },
														{ {.value=1.20, .time=50}, {.value=3.40, .time=7000},  {.value=2.70, .time=1000}, {.value=2.70, .time=1000}, {.value=2.70, .time=500}, {.value=3.40, .time=1000}, {.value=2.10, .time=500}, {.value=3.00, .time=1000}, {.value=2.70, .time=500}, {.value=3.00, .time=1000},
															{.value=3.00, .time=1500}, {.value=0, .time=0} }, 
														{ {.value=1.20, .time=50}, {.value=3.40, .time=7000},  {.value=2.70, .time=1000}, {.value=2.70, .time=1000}, {.value=2.70, .time=1000}, {.value=3.40, .time=1000}, {.value=3.00, .time=1000}, {.value=2.10, .time=750}, {.value=2.70, .time=1000}, {.value=3.40, .time=1000},
															{.value=3.00, .time=250}, {.value=0, .time=0} }, //24 
														{ {.value=1.20, .time=50}, {.value=3.40, .time=7000},  {.value=2.70, .time=750}, {.value=3.40, .time=500}, {.value=3.40, .time=1000}, {.value=2.70, .time=1000}, {.value=3.00, .time=750}, {.value=2.70, .time=1000}, {.value=2.10, .time=500}, {.value=3.00, .time=1000},
															{.value=2.70, .time=1500}, {.value=0, .time=0} }, 
														{ {.value=1.20, .time=50}, {.value=3.40, .time=7000},  {.value=2.70, .time=1000}, {.value=3.40, .time=1000}, {.value=2.70, .time=750}, {.value=2.10, .time=500},  {.value=2.70, .time=500}, {.value=3.00, .time=750}, {.value=3.00, .time=750}, {.value=2.70, .time=1000},
															{.value=2.70, .time=1750}, {.value=0, .time=0} }, 
														{ {.value=1.20, .time=50}, {.value=3.40, .time=7000},  {.value=3.40, .time=1000}, {.value=3.00, .time=1000}, {.value=2.10, .time=750}, {.value=3.00, .time=750}, {.value=2.70, .time=1000}, {.value=3.40, .time=500}, {.value=2.70, .time=500}, {.value=2.70, .time=1000},
															{.value=2.70, .time=1500}, {.value=0, .time=0} },
														{ {.value=1.20, .time=50}, {.value=3.40, .time=7000},  {.value=3.40, .time=500}, {.value=2.70, .time=500}, {.value=3.40, .time=1000}, {.value=2.70, .time=1000}, {.value=3.40, .time=500}, {.value=3.00, .time=750}, {.value=3.40, .time=1000}, {.value=2.10, .time=750},
															{.value=2.70, .time=2000}, {.value=0, .time=0} }, 
														{ {.value=1.20, .time=50}, {.value=3.40, .time=7000},  {.value=3.40, .time=1000}, {.value=2.10, .time=500}, {.value=3.00, .time=1000}, {.value=3.40, .time=1000}, {.value=2.10, .time=500}, {.value=3.00, .time=1000}, {.value=3.00, .time=1000}, {.value=3.40, .time=1000},
															{.value=3.40, .time=1000}, {.value=0, .time=0} },
														{ {.value=1.20, .time=50}, {.value=3.40, .time=7000},  {.value=2.70, .time=500}, {.value=2.70, .time=750}, {.value=2.70, .time=1000}, {.value=2.70, .time=1000}, {.value=2.70, .time=1000}, {.value=2.70, .time=1000}, {.value=2.10, .time=500}, {.value=3.00, .time=1000},
															{.value=3.40, .time=1250}, {.value=0, .time=0} }, //30 
														{ {.value=1.20, .time=50}, {.value=3.40, .time=7000},  {.value=3.00, .time=1000}, {.value=3.40, .time=1000}, {.value=2.70, .time=1000},{.value=2.70, .time=1000}, {.value=2.70, .time=1000}, {.value=2.70, .time=1000}, {.value=2.70, .time=750}, {.value=2.70, .time=500}, 
															{.value=3.40, .time=750}, {.value=0, .time=0} }, 
														{ {.value=1.20, .time=50}, {.value=3.40, .time=7000},  {.value=3.40, .time=1000}, {.value=2.70, .time=500}, {.value=2.70, .time=750}, {.value=3.40, .time=1000}, {.value=3.00, .time=1000}, {.value=2.10, .time=750}, {.value=2.70, .time=1000}, {.value=3.00, .time=750},
															{.value=3.00, .time=1250}, {.value=0, .time=0} }, 
														{ {.value=1.20, .time=50}, {.value=3.40, .time=7000},  {.value=2.70, .time=750},  {.value=3.40, .time=1000}, {.value=3.00, .time=1000}, {.value=2.70, .time=500}, {.value=3.40, .time=500}, {.value=2.10, .time=500}, {.value=3.40, .time=500}, {.value=3.00, .time=750}, 
															{.value=3.00, .time=2500}, {.value=0, .time=0} }, 
														{ {.value=1.20, .time=50}, {.value=3.40, .time=7000},  {.value=2.70, .time=1000},  {.value=2.10, .time=1000}, {.value=3.00, .time=500}, {.value=2.70, .time=500}, {.value=3.00, .time=1000}, {.value=3.00, .time=750}, {.value=3.00, .time=1000}, {.value=2.10, .time=750},
															{.value=3.00, .time=1500}, {.value=0, .time=0} }, 
														{ {.value=1.20, .time=50}, {.value=3.40, .time=7000},  {.value=2.10, .time=500},  {.value=2.70, .time=500}, {.value=3.00, .time=750}, {.value=3.00, .time=750}, {.value=2.70, .time=1000}, {.value=3.40, .time=500}, {.value=3.40, .time=1000}, {.value=2.70, .time=1000},
															{.value=2.70, .time=2000}, {.value=0, .time=0} },
														{ {.value=1.20, .time=50}, {.value=3.40, .time=7000},  {.value=2.10, .time=1000},  {.value=3.00, .time=1000}, {.value=2.70, .time=750}, {.value=3.40, .time=1000}, {.value=2.70, .time=500}, {.value=2.70, .time=750}, {.value=2.10, .time=500}, {.value=3.00, .time=1000},
															{.value=2.70, .time=1500}, {.value=0, .time=0} } //36
													};
*/

//const input_data_t QuadPwrBudget[N_QUADS][N_PW_BUDGET_SEQ] = 
/*
													const input_data_t QuadPwrBudget[2][N_PW_BUDGET_SEQ] = 
														{ { {.value=120.0f, .time=10000}, {.value=0, .time=0}, {.value=0, .time=0}, {.value=0, .time=0}, {.value=0, .time=0}, {.value=0, .time=0} },
														  { {.value=120.0f, .time=10000}, {.value=0, .time=0}, {.value=0, .time=0}, {.value=0, .time=0}, {.value=0, .time=0}, {.value=0, .time=0} },
														};
const input_data_t BoardPwrBudget[N_PW_BUDGET_SEQ] = { {.value=120.0f, .time=1000}, {.value=75.0f, .time=1000}, {.value=40.0f, .time=1000}, {.value=90.0f, .time=1000}, {.value=120.0f, .time=12050}, {.value=0, .time=0}
													};
*/
/*
const input_data_bm_t BindMatrix[N_BINDING_MAT_SEQ] = { {.value[0]=0, .value[1]=0, .value[2]=0, .value[3]=0, .value[4]=0, .value[5]=0, .value[6]=0, .value[7]=0, .value[8]=0, 
															.value[9]=0, .value[10]=0, .value[11]=0, .value[12]=0, .value[13]=0, .value[14]=0, .value[15]=0, .value[16]=0, .value[17]=0,
															.value[18]=0, .value[19]=0, .value[20]=0, .value[21]=0, .value[22]=0, .value[23]=0, .value[24]=0, .value[25]=0, .value[26]=0, 
															.value[27]=0, .value[28]=0, .value[29]=0, .value[30]=0, .value[31]=0, .value[32]=0, .value[33]=0, .value[34]=0, .value[35]=0, 
															.time=5500}, //1 

														/*{.value[0]=1, .value[1]=1, .value[2]=1, .value[3]=1, .value[4]=1, .value[5]=1, .value[6]=1, .value[7]=1, .value[8]=1, 
															.value[9]=1, .value[10]=1, .value[11]=1, .value[12]=1, .value[13]=1, .value[14]=1, .value[15]=1, .value[16]=1, .value[17]=1,
															.value[18]=1, .value[19]=1, .value[20]=1, .value[21]=1, .value[22]=1, .value[23]=1, .value[24]=1, .value[25]=1, .value[26]=1, 
															.value[27]=1, .value[28]=1, .value[29]=1, .value[30]=1, .value[31]=1, .value[32]=1, .value[33]=1, .value[34]=1, .value[35]=1, 
															.time=5500}, //1 */
/*
														{.value[0]=0, .value[1]=0, .value[2]=0, .value[3]=1, .value[4]=1, .value[5]=1, .value[6]=0, .value[7]=0, .value[8]=0, 
															.value[9]=1, .value[10]=1, .value[11]=1, .value[12]=0, .value[13]=0, .value[14]=0, .value[15]=1, .value[16]=1, .value[17]=1,
															.value[18]=2, .value[19]=2, .value[20]=2, .value[21]=3, .value[22]=3, .value[23]=3, .value[24]=2, .value[25]=2, .value[26]=2, 
															.value[27]=3, .value[28]=3, .value[29]=3, .value[30]=2, .value[31]=2, .value[32]=2, .value[33]=3, .value[34]=3, .value[35]=3, 
															.time=2000}, //2 ("4 quadrants")

														{.value[0]=1, .value[1]=1, .value[2]=1, .value[3]=2, .value[4]=2, .value[5]=2, .value[6]=1, .value[7]=1, .value[8]=1, 
															.value[9]=2, .value[10]=2, .value[11]=2, .value[12]=1, .value[13]=1, .value[14]=1, .value[15]=2, .value[16]=2, .value[17]=2,
															.value[18]=1, .value[19]=1, .value[20]=1, .value[21]=2, .value[22]=2, .value[23]=2, .value[24]=1, .value[25]=1, .value[26]=1, 
															.value[27]=2, .value[28]=2, .value[29]=2, .value[30]=1, .value[31]=1, .value[32]=1, .value[33]=2, .value[34]=2, .value[35]=2, 
															.time=2000}, //3 (Half and Half)

														{.value[0]=1, .value[1]=1, .value[2]=1, .value[3]=2, .value[4]=2, .value[5]=2, .value[6]=1, .value[7]=0, .value[8]=0, 
															.value[9]=2, .value[10]=0, .value[11]=2, .value[12]=1, .value[13]=1, .value[14]=0, .value[15]=2, .value[16]=2, .value[17]=2,
															.value[18]=1, .value[19]=0, .value[20]=0, .value[21]=2, .value[22]=0, .value[23]=0, .value[24]=1, .value[25]=1, .value[26]=1, 
															.value[27]=2, .value[28]=0, .value[29]=0, .value[30]=3, .value[31]=3, .value[32]=3, .value[33]=3, .value[34]=3, .value[35]=3, 
															.time=2000}, //4 ("EPI")

														{.value[0]=0, .value[1]=0, .value[2]=0, .value[3]=0, .value[4]=0, .value[5]=0, .value[6]=0, .value[7]=0, .value[8]=0, 
															.value[9]=0, .value[10]=0, .value[11]=0, .value[12]=0, .value[13]=0, .value[14]=0, .value[15]=0, .value[16]=0, .value[17]=0,
															.value[18]=0, .value[19]=0, .value[20]=0, .value[21]=0, .value[22]=0, .value[23]=0, .value[24]=0, .value[25]=0, .value[26]=0, 
															.value[27]=0, .value[28]=0, .value[29]=0, .value[30]=0, .value[31]=0, .value[32]=0, .value[33]=0, .value[34]=0, .value[35]=0, 
															.time=1550}, //5 (0)

														{.value[0]=0, .value[1]=0, .value[2]=0, .value[3]=0, .value[4]=0, .value[5]=0, .value[6]=0, .value[7]=0, .value[8]=0, 
															.value[9]=0, .value[10]=0, .value[11]=0, .value[12]=0, .value[13]=0, .value[14]=0, .value[15]=0, .value[16]=0, .value[17]=0,
															.value[18]=0, .value[19]=0, .value[20]=0, .value[21]=0, .value[22]=0, .value[23]=0, .value[24]=0, .value[25]=0, .value[26]=0, 
															.value[27]=0, .value[28]=0, .value[29]=0, .value[30]=0, .value[31]=0, .value[32]=0, .value[33]=0, .value[34]=0, .value[35]=0, 
															.time=0} //4
														};
*/
