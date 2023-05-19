
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
 * helloworld.c: simple test application
 *
 * This application configures UART 16550 to baud rate 9600.
 * PS7 UART (Zynq) is not initialized by this application, since
 * bootrom/bsp configures it to baud rate 115200
 *
 * ------------------------------------------------
 * | UART TYPE   BAUD RATE                        |
 * ------------------------------------------------
 *   uartns550   9600
 *   uartlite    Configurable only in HW design
 *   ps7_uart    115200 (configured by bootrom/bsp)
 */

//Standard Lib
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h>

//System & Thread
#include <pthread.h>
#include <sched.h>
#ifdef USE_MYSEM
#include "mySem.h"
#else
#include <semaphore.h>
#endif
#include <errno.h>

//Mem
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

//

//MyLib
#include "main.h"
#include "addresses.h"
//Model
#include "model.h"
#include "cmdconf.h"
#include "wl_config.h"
#include "ext_power_config.h"
#include "sim_config.h"
//OS
#include "os_data.h"

//Govern

//Interrupt Management
#include <signal.h>
#include <time.h>

static void model_handler(int sig, siginfo_t *si, void *uc);
const int os_timer_multiplier = steps_per_sim_time_ms; //todo: since os triggers every ms. If it will changed in the future, change accordingly.
const int govern_timer_multiplier = steps_per_sim_time_ms/4; //steps_per_sim_time_ms /2(500) /2; //todo: since os triggers every ms. If it will changed in the future, change accordingly.

//**** global Model Var: ****//

//struct timespec threadtime[4][40] = {{0}};

// outputs (to) --> Pulp:
float* otp_model_core_temp = NULL;
float* otp_domain_pw = NULL;
uint32_t* otp_instructions_information = NULL;
int* gs_workload_acc_read = NULL;
// cmd outputs
float* otpc_core_target_freq = NULL;
float* otpc_domain_pw_budget = NULL;
uint32_t* otpc_core_bindings = NULL;

// inputs (from) <-- Pulp:
float* ifp_ctrl_quad_freq = NULL;
float* ifp_ctrl_quad_vdd = NULL;
float* ifp_ctrl_core_freq = NULL;
//TODO:CANC
float* ifp_debug_alpha = NULL;
float* ifp_debug_redpw = NULL;
uint32_t* ifp_debug_freqredmap = NULL;
//

// outputs (to) --> Pulp:
int otp_domain_pw_dim = 0;
int otp_model_core_temp_dim = 0;
int otp_instructions_information_dim = 0;
// cmd outputs
int otpc_core_target_freq_dim = 0;
int otpc_domain_pw_budget_dim = 0;
int otpc_core_bindings_dim = 0;

// inputs (from) <-- Pulp:
int ifp_ctrl_quad_freq_dim = 0;
int ifp_ctrl_quad_vdd_dim = 0;
int ifp_ctrl_core_freq_dim = 0;
//TODO:canc
int ifp_debug_alpha_dim = 0;
int ifp_debug_redpw_dim = 0;
int ifp_debug_freqredmap_dim = 0;

//Simulation
volatile int* gn_run_simulation = NULL;
volatile int* gn_pause_simulation = NULL;
//unsigned int model_step_counter = 0;
unsigned int CycleIDnumber = 0;
//unsigned int workload_counter = 0;

/*** pthread ***/
pthread_mutex_t pthread_lock_gp_wl;
//data passing
float* gp_computed_workload;
//Peripherals Exclusion
pthread_mutex_t pthread_lock_printf;

//Synchro Stuff
#ifdef USE_MYSEM
mySem_t sem_to_Model;
mySem_t sem_to_Wl;
mySem_t sem_os_timer_tick;
mySem_t sem_model_timer_tick;
mySem_t sem_govern_timer_tick;
#else
sem_t sem_to_Model;
sem_t sem_to_Wl;
sem_t sem_os_timer_tick;
sem_t sem_model_timer_tick;
sem_t sem_govern_timer_tick;
#endif

//mem stuff

//Linux stuff
int dynamic_flag = 0;


int main(int argc, char **argv)
{
    if (argc > 3) {
        printf("Too many arguments\n\rUsage: %s -d -p [/path/to/config.json]", argv[0]);
        exit(1);
    }
    char c;
    char *json_path = NULL;
/*
    while ((c = getopt (argc, argv, "dhp:")) != -1)
        switch (c)
        {
            case 'd':
                dynamic_flag = 1;
                break;
            case 'h':
                printf("Usage: %s \n\r\t -d --> for dynamic memory usage \n\r\t -p [/path/to/config.json]", argv[0]);
                exit(0);
            case 'p':
                json_path = optarg;
                break;
            case '?':
                if (optopt == 'p')
                    fprintf (stderr, "Option -%c requires an argument.\n\r", optopt);
                else if (isprint (optopt))
                    fprintf (stderr, "Unknown option `-%c'.\n\r", optopt);
                else
                    fprintf (stderr,
                        "Unknown option character `\\x%x'.\n\r", optopt);
                exit(1);
            //default:
            //    exit(1);
         }
*/
    /***** Var *****/
    pthread_t th_model, th_workload, th_OS, th_govern, th_scalper;
    int iret_model, iret_wl, iret_OS, iret_govern, iret_scalper;

    /* Timer */
    timer_t timerID_model = 0;
    struct sigevent sev_model;
    struct t_eventData event_data_model = { .myData = 0 };

    /* specifies the action when receiving a signal */
    struct sigaction sa_model = {0};

    int freq_nanosecs_model = sim_multiplier * sim_hw_multiplier * 1000000 / steps_per_sim_time_ms;
    //1000000 / steps_per_sim_time_ms = Real Time
    // Real Time * multiplier = 5

    /* specify start delay and interval */
    //First twos define delay, in sec and nanosec. The second twos define the interval in the same way
    //it_value has to be non-zero: zero will disarm the timer
    struct itimerspec its_model = {
        .it_value.tv_sec  = freq_nanosecs_model / 1000000000,
        .it_value.tv_nsec = freq_nanosecs_model % 1000000000,
        .it_interval.tv_sec  = freq_nanosecs_model / 1000000000,
        .it_interval.tv_nsec = freq_nanosecs_model % 1000000000
    };


    printf("[HiL Sim] Starting Application......\n\r");

    printf("[HiL Sim] Parsing Configuration......\n\r");

    if (json_path != NULL) {
        initialize_simstruct(argv[1]);
    } else {
        printf("[HiL Sim] \t Using Default Configuration for the simulation\n\r");
        initialize_simstruct("System_Simulation/config.json");
    }


    otp_domain_pw_dim = (simulation.nb_power_domains+1);
    otp_model_core_temp_dim = simulation.nb_cores;
    otp_instructions_information_dim = (simulation.nb_cores*WL_STATES);
    // cmd outputs
    otpc_core_target_freq_dim = simulation.nb_cores;
    otpc_domain_pw_budget_dim = (simulation.nb_power_domains+1);
    otpc_core_bindings_dim = simulation.nb_cores;

    // inputs (from) <-- Pulp:
    ifp_ctrl_quad_freq_dim = simulation.nb_power_domains;
    ifp_ctrl_quad_vdd_dim = simulation.nb_power_domains;
    ifp_ctrl_core_freq_dim = simulation.nb_cores;
    //TODO:canc / fix
    ifp_debug_alpha_dim = simulation.nb_cores;
    ifp_debug_redpw_dim = simulation.nb_cores;
    ifp_debug_freqredmap_dim = simulation.nb_cores;

    //TODO FIX THIS: separate between cores, elements, components, etc.
    /*
    int hhh=0;
    printf("%d: %d\n", hhh++, otp_domain_pw_dim);
    printf("%d: %d\n", hhh++, otp_model_core_temp_dim);
    printf("%d: %d\n", hhh++, otp_instructions_information_dim);

    printf("%d: %d\n", hhh++, otpc_core_target_freq_dim);
    printf("%d: %d\n", hhh++, otpc_domain_pw_budget_dim);
    printf("%d: %d\n", hhh++, otpc_core_bindings_dim);

    printf("%d: %d\n", hhh++, ifp_ctrl_quad_freq_dim);
    printf("%d: %d\n", hhh++, ifp_ctrl_quad_vdd_dim);
    printf("%d: %d\n", hhh++, ifp_ctrl_core_freq_dim);
    */

   //this goes here because I don't know how the threads will execute, and it could be bad
   gp_computed_workload = (float*)calloc(simulation.nb_cores*WL_STATES,sizeof(float)); //TODO this is wrong and not flexible!


    /*** Mem Init ***/
    printf("[HiL Sim] Initializing Memory......\n\r");
    int mem_fd = 0;
    void *mem_pointer = NULL;
    void *virt_addr = NULL;
    const uint32_t mem_address = IMP_ADR_FIRST_ADDRESS + IMP_ADR_ADDRESS_CONVERTER;
    const uint32_t mem_size = (IMP_ADR_LAST_ADDRESS - IMP_ADR_FIRST_ADDRESS) +1; //0x180C; //6152

    if (!dynamic_flag)
    {
        mem_fd = open("/dev/mem", O_RDWR | O_SYNC);

        if(mem_fd != -1)
        {
            uint32_t alloc_mem_size, page_mask, page_size;

            page_size = sysconf(_SC_PAGESIZE);
            alloc_mem_size = (((mem_size / page_size) + 1) * page_size);
            page_mask = (page_size - 1);

            mem_pointer = mmap(NULL,
                            alloc_mem_size,
                            PROT_READ | PROT_WRITE,
                            MAP_SHARED,
                            mem_fd,
                            (mem_address & ~page_mask)
                            );

            if (mem_pointer != MAP_FAILED)
            {
                virt_addr = (mem_pointer + (mem_address & page_mask));

                unsigned int* mem_data_clean = (unsigned int*)virt_addr;
                for (int i = 0; i < (mem_size)/4; i++)
                {
                    *mem_data_clean = 0x0;
                    mem_data_clean += 1;
                }

                printf("[HiL Sim] \t mmap() done!\n\r");

                otp_model_core_temp = (float*)(virt_addr + (IMP_ADR_IN_FIRST_CORE_TEMP - IMP_ADR_FIRST_ADDRESS) );
                otp_domain_pw = (float*)(virt_addr + (IMP_ADR_IN_POWER_CPU - IMP_ADR_FIRST_ADDRESS) );
                otp_instructions_information = (uint32_t*)(virt_addr + (IMP_ADR_IN_FIRST_CORE_INSTR - IMP_ADR_FIRST_ADDRESS) );

                // inputs (from) <-- Pulp:
                ifp_ctrl_quad_freq = (float*)(virt_addr + (IMP_ADR_OUT_FIRST_QUAD_FREQ - IMP_ADR_FIRST_ADDRESS) );
                ifp_ctrl_quad_vdd = (float*)(virt_addr + (IMP_ADR_OUT_FIRST_QUAD_VDD - IMP_ADR_FIRST_ADDRESS) );
                ifp_ctrl_core_freq = (float*)(virt_addr + (IMP_ADR_OUT_FIRST_CORE_FREQ - IMP_ADR_FIRST_ADDRESS) );
                //TODO: CANC
                ifp_debug_alpha = (float*)(virt_addr + (IMP_ADR_OUT_FIRST_ALPHA - IMP_ADR_FIRST_ADDRESS) );
                ifp_debug_redpw = (float*)(virt_addr + (IMP_ADR_OUT_FIRST_REDPW - IMP_ADR_FIRST_ADDRESS) );
                ifp_debug_freqredmap = (uint32_t*)(virt_addr + (IMP_ADR_OUT_FIRST_FREQREDMAP - IMP_ADR_FIRST_ADDRESS) );

                gn_run_simulation = (int*)(virt_addr + (IMP_ADR_RUN_SIMULATION - IMP_ADR_FIRST_ADDRESS) );
                gn_pause_simulation = (int*)(virt_addr + (IMP_ADR_PAUSE_SIMULATION - IMP_ADR_FIRST_ADDRESS) );
                gs_workload_acc_read = (int*)(virt_addr + (IMP_ADR_WORKLOAD_READ - IMP_ADR_FIRST_ADDRESS) );

                otpc_core_target_freq = (float*)(virt_addr + (IMP_ADR_CMD_FIRST_CORE_FREQ_T - IMP_ADR_FIRST_ADDRESS) );
                otpc_domain_pw_budget = (float*)(virt_addr + (IMP_ADR_CMD_POWER_BUDGET - IMP_ADR_FIRST_ADDRESS) );
                otpc_core_bindings = (uint32_t*)(virt_addr + (IMP_ADR_CMD_FIRST_CORE_BINDINGS - IMP_ADR_FIRST_ADDRESS) );
            }

        }

        //TODO: add the possibility from argv to lauch the simulation with dynamic memory, enabling printf() instead of sending data?
        if ( (mem_fd == -1) || (mem_pointer == MAP_FAILED) )
        {
            perror("[HiL Sim] \t Error mmap() or Error opening /dev/mem\n\r");
            printf("[HiL Sim] \t Press 'y' to continue with dynamic memory addresses\n\r");
            char user_resp = "";
            user_resp = (char)getchar();
            if ( (user_resp != 'y') && (user_resp != 'Y') )
            {
                printf("[HiL Sim] Exiting......\n\r");
                exit(1);
            }
            else //user wants to continue
            {
                //remove the enter:
                char check;
                scanf("%c", &check);
                //TBC:
                dynamic_flag = 1;
            }

            close(mem_fd);
        }
    }

    if (dynamic_flag == 1)
    {
        otp_model_core_temp = (float*)malloc(sizeof(float) * (otp_model_core_temp_dim+1));
        otp_domain_pw = (float*)malloc(sizeof(float) * (otp_domain_pw_dim+1));
        otp_instructions_information = (uint32_t*)malloc(sizeof(uint32_t) * (otp_instructions_information_dim+1));

        // inputs (from) <-- Pulp:
        ifp_ctrl_quad_freq = (float*)malloc(sizeof(float) * (ifp_ctrl_quad_freq_dim+1));
        ifp_ctrl_quad_vdd = (float*)malloc(sizeof(float) * (ifp_ctrl_quad_vdd_dim+1));
        ifp_ctrl_core_freq = (float*)malloc(sizeof(float) * (ifp_ctrl_core_freq_dim+1));
        //TODO:CANC
        ifp_debug_alpha = (float*)malloc(sizeof(float) * (ifp_debug_alpha_dim+1));
        ifp_debug_redpw = (float*)malloc(sizeof(float) * (ifp_debug_redpw_dim+1));
        ifp_debug_freqredmap = (uint32_t*)malloc(sizeof(uint32_t) * (ifp_debug_freqredmap_dim+1));

        gn_run_simulation = (int*)malloc(sizeof(int));
        gn_pause_simulation = (int*)malloc(sizeof(int));
        gs_workload_acc_read = (int*)malloc(sizeof(int));

        otpc_core_target_freq = (float*)malloc(sizeof(float) * (otpc_core_target_freq_dim+1));
        otpc_domain_pw_budget = (float*)malloc(sizeof(float) * (otpc_domain_pw_budget_dim+1));
        otpc_core_bindings = (uint32_t*)malloc(sizeof(uint32_t) * (otpc_core_bindings_dim+1));
    }


    printf("[HiL Sim] \t Addresses given\n\r");

    /***** Model Init *****/
    printf("[HiL Sim] Starting Initialization......\n\r");
    uint32_t* cafe_addr = NULL;
    CycleIDnumber = 0;
    *gn_run_simulation = 0;
    *gn_pause_simulation = 0;
    *gs_workload_acc_read = 0;
    for (int i = 0; i < (otp_domain_pw_dim); i++) {
        otp_domain_pw[i] = 0; }
    for (int i = 0; i < (otpc_domain_pw_budget_dim); i++) {
        otpc_domain_pw_budget[i] = 34.0f; }
    //Be careful with these: max number of accepted cores is 79 atm. Maybe parametrize the addresses
    //  or write a program to generate them.
    cafe_addr = (uint32_t*)&otp_domain_pw[simulation.nb_power_domains+1];
    *cafe_addr = IMP_ADR_OF_CHARACTERS;
    cafe_addr = (uint32_t*)&otpc_domain_pw_budget[simulation.nb_power_domains+1];
    *cafe_addr = IMP_ADR_OF_CHARACTERS;
    for (int core = 0; core < simulation.nb_cores; core++)
    {
        for (int state = 0; state < WL_STATES; state++)
        {
            gp_computed_workload[core*WL_STATES + state] = 0;
        }
    }
    for (int i = 0; i < otp_instructions_information_dim; i++) {
        otp_instructions_information[i] = 25; }
    for (int i = 0; i < otpc_core_target_freq_dim; i++) {
        otpc_core_target_freq[i] = 3.6; }
    for (int i = 0; i < otpc_core_bindings_dim; i++) {
        otpc_core_bindings[i] = 1; }

    //Be careful with these: (read above)....
    otp_instructions_information[otp_instructions_information_dim] = IMP_ADR_OF_CHARACTERS;
    cafe_addr = (uint32_t*)&otpc_core_target_freq[otpc_core_target_freq_dim];
    *cafe_addr = IMP_ADR_OF_CHARACTERS;
    cafe_addr = (uint32_t*)&otpc_core_bindings[otpc_core_bindings_dim];
    *cafe_addr = IMP_ADR_OF_CHARACTERS;

    initWlTransl();
    model_initialization(simulation.nb_cores, 6, 6, 1000*1000/(uint32_t)steps_per_sim_time_ms); //TODO FIX THIS, plus simply steps_per_sim...


    //global Model Var:
    //TODO: proper values.
    for (int i = 0; i < otp_model_core_temp_dim; i++) {
    	otp_model_core_temp[i] = Tamb; }
    cafe_addr = (uint32_t*)&otp_model_core_temp[otp_model_core_temp_dim];
    *cafe_addr = IMP_ADR_OF_CHARACTERS;
    for (int i = 0; i < simulation.nb_cores; i++) {
        gp_computed_workload[i*WL_STATES + 0] = 1.0; }
    for (int i = 0; i < ifp_ctrl_core_freq_dim; i++) {
        ifp_ctrl_core_freq[i] = 3.0; }
    cafe_addr = (uint32_t*)&ifp_ctrl_core_freq[ifp_ctrl_core_freq_dim];
    *cafe_addr = IMP_ADR_OF_CHARACTERS;

    for (int i = 0; i < ifp_ctrl_quad_freq_dim; i++) {
    	ifp_ctrl_quad_freq[i] = 3.0; }
    cafe_addr = (uint32_t*)&ifp_ctrl_quad_freq[ifp_ctrl_quad_freq_dim];
    *cafe_addr = IMP_ADR_OF_CHARACTERS;
    for (int i = 0; i < ifp_ctrl_quad_vdd_dim; i++) {
    	ifp_ctrl_quad_vdd[i] = 0.75; }
    cafe_addr = (uint32_t*)&ifp_ctrl_quad_vdd[ifp_ctrl_quad_vdd_dim];
    *cafe_addr = IMP_ADR_OF_CHARACTERS;
    for (int i = 0; i < otp_domain_pw_dim; i++) {
        otp_domain_pw[i] = 1.0; } //TODO
    cafe_addr = (uint32_t*)&otp_domain_pw[otp_domain_pw_dim];
    *cafe_addr = IMP_ADR_OF_CHARACTERS;
    otp_domain_pw[0] = 1.0; //TODO.

    //TODO:CANC
    for (int i = 0; i < ifp_debug_alpha_dim; i++) {
        ifp_debug_alpha[i] = 0; }
    cafe_addr = (uint32_t*)&ifp_debug_alpha[ifp_debug_alpha_dim];
    *cafe_addr = IMP_ADR_OF_CHARACTERS;
    for (int i = 0; i < ifp_debug_redpw_dim; i++) {
        ifp_debug_redpw[i] = 0; }
    cafe_addr = (uint32_t*)&ifp_debug_redpw[ifp_debug_redpw_dim];
    *cafe_addr = IMP_ADR_OF_CHARACTERS;
    for (int i = 0; i < ifp_debug_freqredmap_dim; i++) {
        ifp_debug_freqredmap[i] = 0; }
    cafe_addr = (uint32_t*)&ifp_debug_freqredmap[ifp_debug_freqredmap_dim];
    *cafe_addr = IMP_ADR_OF_CHARACTERS;

    printf("[HiL Sim] \t Var Initialization done\n\r");

    /*** Inputs ***/
    printf("[HiL Sim] Parsing Inputs\n\r");
    init_os();


    /*** PTHREAD INITIALIZATION ***/
    //Define Mutex Attr:
    pthread_mutexattr_t lock_attr; //Not sure if it has to be global var.
    if (pthread_mutexattr_init(&lock_attr))
    {
        printf("Error attr for Mutex Init: %s\n\r", strerror(errno));
    }
    if (pthread_mutexattr_settype(&lock_attr, PTHREAD_MUTEX_ERRORCHECK))
    {
        printf("Error attr for Mutex Init: %s\n\r", strerror(errno));
    }
    if (pthread_mutexattr_setprotocol(&lock_attr, PTHREAD_PRIO_INHERIT))
    {
        printf("Error attr for Mutex Init: %s\n\r", strerror(errno));
    }

    //Define CondVar Attr:
    pthread_condattr_t condvar_attr;
    if (pthread_condattr_init(&condvar_attr))
    {
        printf("Error attr for Cond Var Init - %s\n\r", strerror(errno));
    }

    //Initialize Mutexes:
    if (pthread_mutex_init(&pthread_lock_gp_wl, &lock_attr))
    {
        printf("pthread_mutex_init failed - %s\n\r", strerror(errno));
    }
    if (pthread_mutex_init(&pthread_lock_printf, &lock_attr))
    {
        printf("pthread_mutex_init failed - %s\n\r", strerror(errno));
    }

    //Initialize Sem
#ifdef USE_MYSEM
    if (mySem_init(&sem_to_Model, 0, 0, mySem_Binary))
    {
        printf("Error Sem Init\n\r");
    }
    if (mySem_init(&sem_to_Wl, 0, 0, mySem_Binary))
    {
        printf("Error Sem Init\n\r");
    }
    if (mySem_init(&sem_os_timer_tick, 0, 0, mySem_Binary))
    {
        printf("Error Sem Init\n\r");
    }
    if (mySem_init(&sem_model_timer_tick, 0, 0, mySem_Binary))
    {
        printf("Error Sem Init\n\r");
    }
    if (mySem_init(&sem_govern_timer_tick, 0, 0, mySem_Binary))
    {
        printf("Error Sem Init\n\r");
    }

#else //semaphore.h
    if (sem_init(&sem_to_Model, 0, 0))
    {
        printf("Error Sem Init\n\r");
    }
    if (sem_init(&sem_to_Wl, 0, 0))
    {
        printf("Error Sem Init\n\r");
    }
    if (sem_init(&sem_os_timer_tick, 0, 0))
    {
        printf("Error Sem Init\n\r");
    }
    if (sem_init(&sem_model_timer_tick, 0, 0))
    {
        printf("Error Sem Init\n\r");
    }
    if (sem_init(&sem_govern_timer_tick, 0, 0))
    {
        printf("Error Sem Init\n\r");
    }
#endif


    //Thread attrib:
    pthread_attr_t tattr;
    if (pthread_attr_init(&tattr))
    {
        printf("Error attr for thread Init\n\r");
    }
    //pthread_setconcurrency  //no meaning in Linux.
    //Maybe I should also check if I should create bounded or unbounded Threads:
    if (pthread_attr_setschedpolicy(&tattr, SCHED_RR))
    {
        printf("Error attr for thread Init\n\r");
    }
    /*
    sched_param param;
    param.sched_priority = 30;
    if (!pthread_attr_setschedparam(&tattr, &param))
    {
        printf("Error attr for thread Init\n\r");
    }
    */

    printf("[HiL Sim] \t Pthread initialization done\n\r");

    /*** Timer Setup ***/
    sev_model.sigev_notify = SIGEV_SIGNAL;
    sev_model.sigev_signo = SIGRTMIN; //no docu, but all do this
    sev_model.sigev_value.sival_ptr = &event_data_model; //data passed with notification

    /* specifz signal and handler */
    sa_model.sa_flags = SA_SIGINFO;
    sa_model.sa_sigaction = model_handler;

    /* Initialize signal */
    sigemptyset(&sa_model.sa_mask);

    /* Register signal handler */
    if (sigaction(SIGRTMIN, &sa_model, NULL) == -1) {
        printf("Error sigaction: %s\n", strerror(errno));
        exit(-1);
    }

    /* Create Timer */
    if ( timer_create(CLOCK_REALTIME, &sev_model, &timerID_model) != 0 ) {
        printf("Error timer_create: %s\n", strerror(errno));
        exit(-1);
    }

    printf("[HiL Sim] \t Timer initialization done\n\r");

    //FAKE START
    //*gn_run_simulation = 1;
    if (!dynamic_flag)
    {
        while (!(*gn_run_simulation)){};
    }
    else
    {
        printf("[HiL Sim] Press any key to start the simulation:  ");
        char check;
        scanf("%c", &check);
        *gn_run_simulation = 1;
    }
    printf("\r\n[HiL Sim] Starting pthread......\n\r");

    /*** PTHREAD START ***/
    iret_model = pthread_create( &th_model, &tattr, pthread_model_execution, NULL);
    iret_wl = pthread_create( &th_workload, &tattr, pthread_workload_computation, NULL);
    iret_OS = pthread_create( &th_OS, &tattr, pthread_os_scmi_sim, NULL);
    //iret_govern = pthread_create( &th_govern, &tattr, pthread_govern_interface, NULL)
    iret_scalper = pthread_create( &th_scalper, &tattr, pthread_data_scalper, NULL);

    /* Wait till threads are complete before main continues. Unless we  */
    /* wait we run the risk of executing an exit which will terminate   */
    /* the process and all threads before the threads have completed.   */

    /* start timer */
    if (timer_settime(timerID_model, 0, &its_model, NULL) != 0){
        printf("Error timer_settime: %s\n", strerror(errno));
        exit(-1);
    }

    //Dynamic HMI
    if (dynamic_flag)
    {
        //since sleep is not working (cuz a signal will wake the thread)
        uint32_t app = 0;
        for (uint32_t i=100000000; i>0; i++)
            //for (uint32_t j=1; j>0; j++)
                //for (uint32_t k=2; k>0; k++)
                    //for (uint32_t k1=2; k1>0; k1++)
                        //for (uint32_t k2=2; k2>0; k2++)
            {
                app++;
                app *= app;
            }
    printf("%d\n", app);


        int simulating = 1;
        while(simulating)
        {
            printf("[HiL Sim] Press the 's' key to Stop the simulation, 'p' to pause it:  \r");
            char user_resp = "";
            user_resp = (char)getchar();
            if ( (user_resp == 's') || (user_resp == 'S') )
            {
                *gn_run_simulation = 0;
                printf("[HiL Sim] Stopping Simulation......\n\r");
                simulating = 0;
            }
            else if ( (user_resp == 'p') || (user_resp == 'P') )
            {
                *gn_pause_simulation = 1;
                printf("[HiL Sim] Pausing Simulation......\n\r");
                int pausing = 1;
                while(pausing)
                {
                    printf("[HiL Sim] Press the 'r' key to unpause the simulation:  \r");
                    char user_resp = "";
                    user_resp = (char)getchar();
                    if ( (user_resp == 'r') || (user_resp == 'R') )
                    {
                        *gn_pause_simulation = 0;
                        printf("[HiL Sim] Restarting Simulation......\n\r");
                        pausing = 0;
                    }
                }
            } 

            //since I cannot make it work:
            //simulating--;
        }
    }

    /** Stop **/
    pthread_join( th_model, NULL);
    printf("Thread 1 returns: %d\n",iret_model);
    pthread_join( th_workload, NULL);
    printf("Thread 2 returns: %d\n",iret_wl);
    pthread_join( th_OS, NULL);
    printf("Thread 3 returns: %d\n",iret_OS);
    //pthread_join( th_govern, NULL);
    //printf("Thread 4 returns: %d\n",iret_govern);
    pthread_join( th_scalper, NULL);
    printf("Thread 5 returns: %d\n",iret_scalper);


    //clear timer:
    if ( timer_delete(timerID_model) != 0)
    {
        perror("error timer_delete\n\r");
        exit(-1);
    }


    /*** PThread clean-up ***/
#ifdef USE_MYSEM
    mySem_destroy(&sem_to_Model);
    mySem_destroy(&sem_to_Wl);
    mySem_destroy(&sem_os_timer_tick);
    mySem_destroy(&sem_model_timer_tick);
    mySem_destroy(&sem_govern_timer_tick);

#else
    sem_destroy(&sem_to_Model);
    sem_destroy(&sem_to_Wl);
    sem_destroy(&sem_os_timer_tick);
    sem_destroy(&sem_model_timer_tick);
    sem_destroy(&sem_govern_timer_tick);
#endif

    pthread_mutex_destroy(&pthread_lock_gp_wl);
    pthread_mutex_destroy(&pthread_lock_printf);


    /** Memory Dump **/
    //int* fd_write = open("/dump_mem_simulation.txt", O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);
    /*
    FILE* fd_write = fopen("/home/root/dump_mem_simulation.txt", "w");
    if (fd_write == NULL)
    {
        perror("Error opening file!\n");
    }
    unsigned int* mem_data = (unsigned int*)virt_addr;
    unsigned int* mem_trans = IMP_ADR_FIRST_ADDRESS + IMP_ADR_ADDRESS_CONVERTER;
    for (int i = 0; i < (mem_size)/4; i++)
    {
        float* app = mem_data;
        fprintf(fd_write, "mem: %x, hex: %x, float: %f, int: %d\n", (void*)mem_trans, (*mem_data), *app, (int)(*mem_data));
        mem_data += 1;
        mem_trans += 1;
    }
    fclose(fd_write);
    */
    
    //while(1){printf("a");};

    /*** Linux clean-up ***/
    for (int i=0; i<simulation.nb_elements; i++)
    {
        if (simulation.elements[i].type == JT_CORE)
            free(simulation.elements[i].core_config.dyn_pow_cpu_coeff);
    }
        
    free(simulation.elements);
    
    free(TargetFrequency);
    free(QuadPwrBudget);
    free(BoardPwrBudget);
    free(BindMatrix);

    if (!dynamic_flag)
    {
        close(mem_fd);
    }
    else
    {
        //print stuff:
        printf("code: %d\n", *gn_run_simulation );

        for (int i = 0; i < (otp_domain_pw_dim); i++)
        {
            printf("pw: mem: %x, float: %f\n", (void*)&otp_domain_pw[i], otp_domain_pw[i]);
            
        }
        for (int i = 0; i < (otpc_domain_pw_budget_dim); i++)
        {
            printf("pwb: mem: %x, float: %f\n", (void*)&otpc_domain_pw_budget[i], otpc_domain_pw_budget[i]);
        }
        for (int core = 0; core < simulation.nb_cores; core++)
        {
            for (int state = 0; state < WL_STATES; state++)
            {
                printf("instr_info: mem: %x, int: %d\n", (void*)&otp_instructions_information[core*WL_STATES + state], otp_instructions_information[core*WL_STATES + state]);
            }
        }
        for (int core = 0; core < simulation.nb_cores; core++)
        {
            for (int state = 0; state < WL_STATES; state++)
            {
                printf("cmp_wl: mem: %x, float: %f\n", (void*)&simulation.elements[core].core_config.workload[state], simulation.elements[core].core_config.workload[state]);
            }
        }
        for (int core = 0; core < otpc_core_target_freq_dim; core++)
        {
            printf("tar_freq: mem: %x, float: %f\n", (void*)&otpc_core_target_freq[core], otpc_core_target_freq[core]);
            //otpc_core_bindings[core] = 1;
        }
        for (int core = 0; core < ifp_ctrl_core_freq_dim; core++)
        {
            printf("freq: mem: %x, float: %f\n", (void*)&ifp_ctrl_core_freq[core], ifp_ctrl_core_freq[core]);
            //otpc_core_bindings[core] = 1;
        }
        for (int core = 0; core < otp_model_core_temp_dim; core++)
        {
            printf("temp: mem: %x, float: %f\n", (void*)&otp_model_core_temp[core], otp_model_core_temp[core]-273.15);
            //otpc_core_bindings[core] = 1;
        }

        free(otpc_core_bindings);
        free(otpc_domain_pw_budget);
        free(otpc_core_target_freq);
        free(gs_workload_acc_read);
        free(gn_pause_simulation);
        free(gn_run_simulation);
        free(ifp_ctrl_core_freq);
        free(ifp_ctrl_quad_vdd);
        free(ifp_ctrl_quad_freq);
        //TODO:CANC
        free(ifp_debug_alpha );
        free(ifp_debug_redpw );
        free(ifp_debug_freqredmap );
        //
        free(otp_instructions_information);
        free(otp_domain_pw);
        free(otp_model_core_temp);
    }



    exit(0);
    return 1;
}




static void model_handler(int sig, siginfo_t *si, void *uc)
{
    static int os_counter = os_timer_multiplier;
    static int govern_counter = govern_timer_multiplier;
    /*** Signal the Model Thread the data are Ready ***/
#ifdef USE_MYSEM
    mySem_post(&sem_model_timer_tick);
#else
    //Busy Waiting to simulate a Binary Semaphore
    sem_getvalue(&sem_model_timer_tick, &sem_value);
    while(sem_value > 0)
    {sem_getvalue(&sem_model_timer_tick, &sem_value);}
    sem_post(&sem_model_timer_tick);
#endif

    os_counter--;
    if (os_counter <= 0)
    {
        os_counter = os_timer_multiplier;
        /*** Signal the Model Thread the data are Ready ***/
#ifdef USE_MYSEM
        mySem_post(&sem_os_timer_tick);
#else
        //Busy Waiting to simulate a Binary Semaphore
        sem_getvalue(&sem_os_timer_tick, &sem_value);
        while(sem_value > 0)
        {sem_getvalue(&sem_os_timer_tick, &sem_value);}
        sem_post(&sem_os_timer_tick);
#endif
    }


    govern_counter--;
    if (govern_counter <= 0)
    {
        govern_counter = govern_timer_multiplier;
        /*** Signal the Model Thread the data are Ready ***/
#ifdef USE_MYSEM
        mySem_post(&sem_govern_timer_tick);
#else
        //Busy Waiting to simulate a Binary Semaphore
        sem_getvalue(&sem_govern_timer_tick, &sem_value);
        while(sem_value > 0)
        {sem_getvalue(&sem_govern_timer_tick, &sem_value);}
        sem_post(&sem_govern_timer_tick);
#endif
    }


}
