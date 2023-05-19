
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


#include "sim_config.h"
#include "cJSON.h"
#include "cmdconf.h"
#include "model.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <string.h>
#include <time.h>

#define JSON_DEBUG

#define JSON_CORE_CID   0x3FF
#define JSON_HBM_CID    0x7FF
#define JSON_COMMON_CID 0xFFFF

int USE_ALL_DOMAINS = 0;
int USE_CONSTANT_DEVIATION = 0;

struct simulation_st simulation;

static const char noise_white[] = "white";

// general power model function
/*
static float compute_common_power(struct element_st *self, float activity) {
    // multiplying the coefficient with the percentage of the activity of the
    // components
    if (self->common_config.fixed_activity) {
        return self->common_config.coeff * self->common_config.activity;
    } else {
        return self->common_config.coeff * activity;
    }
}
*/

//TODO integrate this better:
float constant_parameter_deviation[36] = {102.829826,
103.376648,
103.92144,
105.575813,
102.984085,
103.015053,
104.723274,
103.396263,
104.62513,

104.85408,
103.248093,
104.179611,
105.452026,
103.569344,
106.145004,
103.760078,
104.386665,
103.893303,

105.23027,
103.896332,
104.006592,
102.723488,
105.160606,
102.424133,
102.297165,
105.767616,
104.434547,

103.279343,
105.896286,
103.253319,
104.001114,
102.666077,
103.194771,
105.131111,
102.493217,
104.080215
};

int initialize_simstruct(char *filepath) {
    char data[MAX_JSON_DIM];
    int status = 0;
    int fd = open(filepath, O_RDONLY);
    read(fd, data, MAX_JSON_DIM); //TODO check return value of read
    data[MAX_JSON_DIM-1] = '\0';
    int total_component_number = 0;
    int index = 0;

    uint32_t nb_components = 0;
    uint32_t nb_core_components = 0;
    uint32_t nb_chiplets = 0;
    uint32_t nb_chiplet_links = 0;

    simulation.nb_cores = 0;


    srand((unsigned) time(NULL));

    // leggo il json e lo salvo nella struttura cJSON
    cJSON *json = cJSON_Parse(data);
    const cJSON *jsonarray = NULL;
    const cJSON *element = NULL;
    if (json == NULL) {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL) {
            int prev = 20;
            int next = 20;
            //TODO: this check
            //while( (error_ptr-(prev*sizeof(char)) < data) ){prev--;}
            //while( (error_ptr+(next*sizeof(char)) > &data[MAX_JSON_DIM]) ){next--;}
            char error_string[64];
            strncpy(error_string, (error_ptr-(prev*sizeof(char))), (prev+next+1));
            fprintf(stderr, "[JSON] Could not find a well formed JSON, char '%c', @ '%s'\n\r", *error_ptr, error_string);
            exit(-10);
        }
        status = 0;
        cJSON_Delete(json);
        return status;
    }

    jsonarray = cJSON_GetObjectItemCaseSensitive(json, "core");
    //Checking the number of cores component
    total_component_number = 0;
    cJSON_ArrayForEach(element, jsonarray) { total_component_number++; }

    // initialization of the array of structure
    struct core_config_st* core_components =
        (struct core_config_st*)malloc(sizeof(struct core_config_st) * total_component_number);
    nb_components += total_component_number;
    nb_core_components = total_component_number;

    //struct uncert_t *ptr_uncert = malloc(sizeof(struct uncert_t) * total_component_number);

    //Chiplet
    jsonarray = cJSON_GetObjectItemCaseSensitive(json, "chiplet");
    nb_chiplets = 0; //NOT ADD THIS TO COMPONENT NUMBER!
    cJSON_ArrayForEach(element, jsonarray) { nb_chiplets++; }
    struct chiplet_t *chiplet_model_ptr = malloc(sizeof(struct chiplet_t) * nb_chiplets);
    index = 0;
    cJSON_ArrayForEach(element, jsonarray) {
        cJSON *chiplet_element;
        chiplet_element = cJSON_GetObjectItemCaseSensitive(element, "cid");
        if (cJSON_IsNumber(chiplet_element) && (chiplet_element->valueint > 0)) {
            #ifdef JSON_DEBUG
            printf("[JSON] Checking cid \"%d\"\n\r", chiplet_element->valueint);
            #endif
            chiplet_model_ptr[index].cid = (uint32_t)chiplet_element->valueint;
        } else {
            //TODO
            printf("[JSON] cid is not a number\n\r");
            exit(1);
        }

        //INTER-WAFER  
        chiplet_element = cJSON_GetObjectItemCaseSensitive(element, "inter-wafer_silicon_variation");
        //TODO: check?

        //1
        cJSON *uncert_element = cJSON_GetObjectItemCaseSensitive(chiplet_element, "gaussian_mean");
        if (cJSON_IsNumber(uncert_element)) {
            #ifdef JSON_DEBUG
            printf("[JSON] Checking component %d inter-wafer gaussian mean\n\r", chiplet_model_ptr[index].cid);
            #endif
            chiplet_model_ptr[index].inter_wafer_silicon_var.mean = uncert_element->valuedouble;
        } else {
            //TODO
            printf("[JSON] Component %d inter-wafer gaussian mean is not a number, using DEFAULT\n\r", chiplet_model_ptr[index].cid);
            //exit(1);
            chiplet_model_ptr[index].inter_wafer_silicon_var.mean = 0;
        }
        //2
        uncert_element = cJSON_GetObjectItemCaseSensitive(chiplet_element, "gaussian_variance");
        if (cJSON_IsNumber(uncert_element)) {
            #ifdef JSON_DEBUG
            printf("[JSON] Checking component %d inter-wafer gaussian variance\n\r", chiplet_model_ptr[index].cid);
            #endif
            chiplet_model_ptr[index].inter_wafer_silicon_var.variance = uncert_element->valuedouble;
        } else {
            //TODO
            printf("[JSON] Component %d inter-wafer gaussian variance is not a number, using DEFAULT\n\r", chiplet_model_ptr[index].cid);
            //exit(1);
            chiplet_model_ptr[index].inter_wafer_silicon_var.variance = 0;
        }
        // 3
        uncert_element = cJSON_GetObjectItemCaseSensitive(chiplet_element, "variance_limits_perc");
        if (cJSON_IsNumber((uncert_element != NULL) ? (uncert_element)->child : NULL)) {
            #ifdef JSON_DEBUG
            printf("[JSON] Checking component %d inter-wafer variance lower limit\n\r",chiplet_model_ptr[index].cid);
            #endif
            chiplet_model_ptr[index].inter_wafer_silicon_var.variance_limits_perc[0] = (uncert_element)->child->valuedouble;

            //second point, only if first exist, otherwise we have Seg Fault
            if (cJSON_IsNumber(((uncert_element)->child != NULL) ? (uncert_element)->child->next : NULL)) {
                #ifdef JSON_DEBUG
                printf("[JSON] Checking component %d inter-wafer variance upper limit\n\r", chiplet_model_ptr[index].cid);
                #endif
                chiplet_model_ptr[index].inter_wafer_silicon_var.variance_limits_perc[1] = uncert_element->child->next->valuedouble;
            } else {
                //TODO
                printf("[JSON] Component %d inter-wafer variance upper limit is not a number, using DEFAULT\n\r", chiplet_model_ptr[index].cid);
                //exit(1);
                chiplet_model_ptr[index].inter_wafer_silicon_var.variance_limits_perc[1] = 1000; //1000 = 100,000% = no limit
            }
        } else {
            //TODO
            printf("[JSON] Component %d inter-wafer variance lower limit is not a number, using DEFAULT\n\r", chiplet_model_ptr[index].cid);
            //exit(1);
            chiplet_model_ptr[index].inter_wafer_silicon_var.variance_limits_perc[0] = -1000; //1000 = 100,000% = no limit
            chiplet_model_ptr[index].inter_wafer_silicon_var.variance_limits_perc[1] = 1000; //1000 = 100,000% = no limit
        }

        // 4
        uncert_element = cJSON_GetObjectItemCaseSensitive(chiplet_element, "3sigma=30:variation_perc");
        if (cJSON_IsNumber(uncert_element)) {
            #ifdef JSON_DEBUG
            printf("[JSON] Checking component %d 3sigma=30:variation_perc\n\r", chiplet_model_ptr[index].cid);
            #endif
            chiplet_model_ptr[index].inter_wafer_silicon_var.three_sigma_prop_perc = uncert_element->valuedouble;
        } else {
            //TODO
            printf("[JSON] Component %d 3sigma=30:variation_perc is not a number, using DEFAULT\n\r", chiplet_model_ptr[index].cid);
            //exit(1);
            chiplet_model_ptr[index].inter_wafer_silicon_var.three_sigma_prop_perc = 1.0;
        }   

        //The Gaussian noise silicon variation it is normalized over a 3*sigma=30 --> variance = 100
        float gaussian_var, res;
        do {
            float sigma = (float)sqrt(chiplet_model_ptr[index].inter_wafer_silicon_var.variance);
            generateGaussianNoise(&gaussian_var, NULL, 
                    chiplet_model_ptr[index].inter_wafer_silicon_var.mean, 
                    sigma);
            res = (gaussian_var * chiplet_model_ptr[index].inter_wafer_silicon_var.three_sigma_prop_perc) / 30.0f;
        } while((res < chiplet_model_ptr[index].inter_wafer_silicon_var.variance_limits_perc[0]) ||
                  (res > chiplet_model_ptr[index].inter_wafer_silicon_var.variance_limits_perc[1]) );
        //here I repeat instead of just doing saturation, because If I saturate, I would violate the normal distribution
        //  given that at the boundaries I will have a spike = the integral of the gaussian distribution from limit to infinity.

        chiplet_model_ptr[index].inter_wafer_silicon_var_perc = res;
        #ifdef JSON_DEBUG
        printf("[JSON] \t chiplet %d variation: %f\n\r", chiplet_model_ptr[index].cid, res);
        #endif

        index++;
    }



    // parsing the cores' configuration
    jsonarray = cJSON_GetObjectItemCaseSensitive(json, "core");
    index = 0;
    cJSON_ArrayForEach(element, jsonarray) {
        cJSON *core_element;
        /* // NAME
        core_element = cJSON_GetObjectItemCaseSensitive(element, "name");
        if (cJSON_IsNumber(core_element) && (core_element->valuedouble >= 0.0f)) {
            #ifdef JSON_DEBUG
            printf("Checking leak_temp \"%lf\"\n", core_element->valuedouble);
            #endif
            core_components[index].leak_temp = core_element->valuedouble;
        } else {
            //TODO
            printf("not a number");
            exit(1);
        }
        */
        //
        core_element = cJSON_GetObjectItemCaseSensitive(element, "cid");
        if (cJSON_IsNumber(core_element) && (core_element->valueint > 0)) {
            #ifdef JSON_DEBUG
            printf("[JSON] Checking cid \"%d\"\n\r", core_element->valueint);
            #endif
            core_components[index].cid = (uint32_t)core_element->valueint;
        } else {
            //TODO
            printf("[JSON] cid is not a number\n\r");
            exit(1);
        }
        core_element = cJSON_GetObjectItemCaseSensitive(element, "leak_vdd");
        if (cJSON_IsNumber(core_element) && (core_element->valuedouble >= 0.0f)) {
            #ifdef JSON_DEBUG
            printf("[JSON] Checking leak_vdd \"%lf\"\n\r", core_element->valuedouble);
            #endif
            //simulation->elements[i].core_config.leak_temp
            core_components[index].leak_vdd = core_element->valuedouble;
        } else {
            //TODO
            printf("[JSON] leak_vdd is not a number\n\r");
            exit(1);
        }
        core_element = cJSON_GetObjectItemCaseSensitive(element, "leak_temp");
        if (cJSON_IsNumber(core_element) && (core_element->valuedouble >= 0.0f)) {
            #ifdef JSON_DEBUG
            printf("[JSON] Checking leak_temp \"%lf\"\n\r", core_element->valuedouble);
            #endif
            core_components[index].leak_temp = core_element->valuedouble;
        } else {
            //TODO
            printf("[JSON] leak_temp is not a number\n\r");
            exit(1);
        }
        core_element = cJSON_GetObjectItemCaseSensitive(element, "leak_process");
        if (cJSON_IsNumber(core_element) && (core_element->valuedouble >= 0.0f)) {
            #ifdef JSON_DEBUG
            printf("[JSON] Checking leak_process \"%lf\"\n\r", core_element->valuedouble);
            #endif
            core_components[index].leak_process = core_element->valuedouble;
        } else {
            //TODO
            printf("[JSON] leak_process is not a number\n\r");
            exit(1);
        }
        core_element = cJSON_GetObjectItemCaseSensitive(element, "instructions_power_levels");
        if (cJSON_IsNumber(core_element) && (core_element->valueint > 0)) {
            #ifdef JSON_DEBUG
            printf("[JSON] Checking wl_states \"%d\"\n\r", core_element->valueint);
            #endif
            core_components[index].wl_states = core_element->valueint;
        } else {
            //TODO
            printf("[JSON] wl_states is not a number\n\r");
            exit(1);
        }
        core_element = cJSON_GetObjectItemCaseSensitive(element, "dyn_pow_cpu_coeff");
        //need to check that dyn_elements >= WL_STATES, e possibilmente WL_STATES = dyn_elements
        cJSON* dyn_element;
        total_component_number = 0;
        cJSON_ArrayForEach(dyn_element, core_element){total_component_number++;}
        if (total_component_number < core_components[index].wl_states) {
            printf("[JSON] Not enough dyn coefficient to fill WL_STATES dim in core\n\r");
            exit(1);
        } else if (total_component_number > core_components[index].wl_states) {
            printf("[JSON] Attention,there are more dyn coeff values then WL_STATES dim in core\n\r");
        }
        int index2 = 0;
        core_components[index].dyn_pow_cpu_coeff = (float*)malloc(sizeof(float)*total_component_number); //TODO: or core_components[index].wl_states
        cJSON_ArrayForEach(dyn_element, core_element) {
            if (cJSON_IsNumber(dyn_element) && (dyn_element->valuedouble >= 0.0f)) {
                #ifdef JSON_DEBUG
                printf("[JSON] Checking dyn_coeff %d \"%lf\"\n\r", index2, dyn_element->valuedouble);
                #endif
                core_components[index].dyn_pow_cpu_coeff[index2] = dyn_element->valuedouble;
            } else {
                //TODO
                printf("[JSON] dyn_coeff %d is not a number\n\r", index2);
                exit(1);
            }

            index2++;
            if (index2 >= core_components[index].wl_states)
                break;
        }

        core_element = cJSON_GetObjectItemCaseSensitive(element, "uncertainties");
        //TODO: check?
        // ################################
        cJSON *par1 = cJSON_GetObjectItemCaseSensitive(core_element, "intra-die_silicon_variation");
        //TODO: check?

        //1
        cJSON *uncert_element = cJSON_GetObjectItemCaseSensitive(par1, "gaussian_mean");
        if (cJSON_IsNumber(uncert_element)) {
            #ifdef JSON_DEBUG
            printf("[JSON] Checking component %d intra-die gaussian mean\n\r", core_components[index].cid);
            #endif
            core_components[index].uncert.intra_die_silicon_var.mean = uncert_element->valuedouble;
        } else {
            //TODO
            printf("[JSON] Component %d intra-die gaussian mean is not a number, using DEFAULT\n\r", core_components[index].cid);
            //exit(1);
            core_components[index].uncert.intra_die_silicon_var.mean = 0;
        }
        //2
        uncert_element = cJSON_GetObjectItemCaseSensitive(par1, "gaussian_variance");
        if (cJSON_IsNumber(uncert_element)) {
            #ifdef JSON_DEBUG
            printf("[JSON] Checking component %d intra-die gaussian variance \n\r", core_components[index].cid);
            #endif
            core_components[index].uncert.intra_die_silicon_var.variance = uncert_element->valuedouble;
        } else {
            //TODO
            printf("[JSON] Component %d intra-die gaussian variance is not a number, using DEFAULT\n\r", core_components[index].cid);
            //exit(1);
            core_components[index].uncert.intra_die_silicon_var.variance = 0;
        }
        // 3
        uncert_element = cJSON_GetObjectItemCaseSensitive(par1, "variance_limits_perc");
        if (cJSON_IsNumber((uncert_element != NULL) ? (uncert_element)->child : NULL)) {
            #ifdef JSON_DEBUG
            printf("[JSON] Checking component %d intra-die variance lower limit\n\r", core_components[index].cid);
            #endif
            core_components[index].uncert.intra_die_silicon_var.variance_limits_perc[0] =  (uncert_element)->child->valuedouble;

            //second point, only if first exist, otherwise we have Seg Fault
            if (cJSON_IsNumber(((uncert_element)->child != NULL) ? (uncert_element)->child->next : NULL)) {
                #ifdef JSON_DEBUG
                printf("[JSON] Checking component %d intra-die variance upper limit\n\r", core_components[index].cid);
                #endif
                core_components[index].uncert.intra_die_silicon_var.variance_limits_perc[1] =  (uncert_element)->child->next->valuedouble;
            } else {
                //TODO
                printf("[JSON] Component %d intra-die variance upper limit is not a number, using DEFAULT\n\r", core_components[index].cid);
                //exit(1);
                core_components[index].uncert.intra_die_silicon_var.variance_limits_perc[1] = 1000; //1000 = 100,000% = no limit;
            }
        } else {
            //TODO
            printf("[JSON] Component %d intra-die variance lower limit is not a number, using DEFAULT\n\r", core_components[index].cid);
            //exit(1);
            core_components[index].uncert.intra_die_silicon_var.variance_limits_perc[0] = -1000; //1000 = 100,000% = no limit
            core_components[index].uncert.intra_die_silicon_var.variance_limits_perc[1] = 1000; //1000 = 100,000% = no limit
        }

        // 4
        uncert_element = cJSON_GetObjectItemCaseSensitive(par1, "3sigma=30:variation_perc");
        if (cJSON_IsNumber(uncert_element)) {
            #ifdef JSON_DEBUG
            printf("[JSON] Checking component %d 3sigma=30:variation_perc\n\r", core_components[index].cid);
            #endif
            core_components[index].uncert.intra_die_silicon_var.three_sigma_prop_perc = uncert_element->valuedouble;
        } else {
            //TODO
            printf("[JSON] Component %d 3sigma=30:variation_perc is not a number, using DEFAULT\n\r", core_components[index].cid);
            //exit(1);
            core_components[index].uncert.intra_die_silicon_var.three_sigma_prop_perc = 1.0;
        }
        

        // ################################
        par1 = cJSON_GetObjectItemCaseSensitive(core_element, "temperature_noise");
        //TODO: check?
        uncert_element = cJSON_GetObjectItemCaseSensitive(par1, "type");
        if (cJSON_IsString(uncert_element) && (uncert_element->valuestring != NULL)){
            #ifdef JSON_DEBUG
            printf("[JSON] Checking component %d temperature noise type\n\r", core_components[index].cid);
            //Check of the correct string will be done after
            #endif
            if (strlen(uncert_element->valuestring) < STRING_DIM_NOISE_TYPE) //strictly because string terminator
                strncpy(core_components[index].uncert.temperature_noise.type, uncert_element->valuestring, STRING_DIM_NOISE_TYPE);
            else
            {
                printf("[JSON] Component %d temperature noise type is exceeding maximum string dim of %d\n\r\t---so probably incorrect type is selected, WHITE as default\n\r", core_components[index].cid, STRING_DIM_NOISE_TYPE);
                strncpy(core_components[index].uncert.temperature_noise.type, noise_white, STRING_DIM_NOISE_TYPE);
            }
        } else {
            //TODO
            printf("[JSON] Component %d temperature noise type is not a string, using DEFAULT\n\r", core_components[index].cid);
            //exit(1);
            strncpy(core_components[index].uncert.temperature_noise.type, noise_white, STRING_DIM_NOISE_TYPE);
        }
        uncert_element = cJSON_GetObjectItemCaseSensitive(par1, "signal_to_noise_ratio");
        if (cJSON_IsNumber(uncert_element)) {
            #ifdef JSON_DEBUG
            printf("[JSON] Checking component %d temperature noise snr\n\r", core_components[index].cid);
            #endif
            core_components[index].uncert.temperature_noise.snr = uncert_element->valuedouble;
        } else {
            //TODO
            printf("[JSON] Component %d temperature noise snr is not a number, using DEFAULT\n\r", core_components[index].cid);
            //exit(1);
            core_components[index].uncert.temperature_noise.snr = 0;
        }
        // ################################
        par1 = cJSON_GetObjectItemCaseSensitive(core_element, "workload_noise");
        //TODO: check?
        uncert_element = cJSON_GetObjectItemCaseSensitive(par1, "type");
        if (cJSON_IsString(uncert_element) && (uncert_element->valuestring != NULL)){
            #ifdef JSON_DEBUG
            printf("[JSON] Checking component %d workload noise type\n\r", core_components[index].cid);
            //Check of the correct string will be done after
            #endif
            if (strlen(uncert_element->valuestring) < STRING_DIM_NOISE_TYPE) //strictly because string terminator
                strncpy(core_components[index].uncert.workload_noise.type, uncert_element->valuestring, STRING_DIM_NOISE_TYPE);
            else
            {
                printf("[JSON] Component %d workload noise type is exceeding maximum string dim of %d\n\r\t---so probably incorrect type is selected, WHITE as default\n\r", core_components[index].cid, STRING_DIM_NOISE_TYPE);
                strncpy(core_components[index].uncert.workload_noise.type, noise_white, STRING_DIM_NOISE_TYPE);
            }
        } else {
            //TODO
            printf("[JSON] Component %d workload noise type is not a string, using DEFAULT\n\r",core_components[index].cid);
            //exit(1);
            strncpy(core_components[index].uncert.workload_noise.type, noise_white, STRING_DIM_NOISE_TYPE);
        }
        uncert_element = cJSON_GetObjectItemCaseSensitive(par1, "signal_to_noise_ratio");
        if (cJSON_IsNumber(uncert_element)) {
            #ifdef JSON_DEBUG
            printf("[JSON] Checking component %d workload noise snr\n\r", core_components[index].cid);
            #endif
            core_components[index].uncert.workload_noise.snr = uncert_element->valuedouble;
        } else {
            //TODO
            printf("[JSON] Component %d workload noise snr is not a number, using DEFAULT\n\r", core_components[index].cid);
            //exit(1);
            core_components[index].uncert.workload_noise.snr = 0;
        }

        index++;
    }


    // parsing the general components' configuration
    jsonarray = cJSON_GetObjectItemCaseSensitive(json, "common");
    //Checking the number of component
    total_component_number = 0;
    cJSON_ArrayForEach(element, jsonarray) { total_component_number++; }
    nb_components += total_component_number;

    // initialization of the array of structure
    struct common_config_st* common_components = (struct common_config_st*)malloc(sizeof(struct common_config_st) * total_component_number);

    // parsing the cores' configuration
    //jsonarray = cJSON_GetObjectItemCaseSensitive(json, "core");
    index = 0;
    cJSON_ArrayForEach(element, jsonarray) {
        cJSON *common_element;
        /* // NAME
        common_element = cJSON_GetObjectItemCaseSensitive(element, "name");
        if (cJSON_IsNumber(core_element) && (common_element->valuedouble >= 0.0f)) {
            #ifdef JSON_DEBUG
            printf("Checking leak_temp \"%lf\"\n", common_element->valuedouble);
            #endif
            common_components[index].leak_temp = common_element->valuedouble;
        } else {
            //TODO
            printf("not a number");
            exit(1);
        }
        */
        //
    }

    // ELEMENT<

    // parsing the number of elements
    jsonarray = cJSON_GetObjectItemCaseSensitive(json, "Processor Config");
    //cJSON_ArrayForEach(element, jsonarray) {
        //TODO
    //}
    //Config:
    element = cJSON_GetObjectItemCaseSensitive(jsonarray, "use_all_domains");
    if (cJSON_IsString(element) && (element->valuestring != NULL)){
        #ifdef JSON_DEBUG
        printf("[JSON] Checking if to use all domains: %s\n\r", element->valuestring);
        //Check of the correct string will be done after
        #endif
        if ( (!strcmp(element->valuestring,"yes")) || 
                (!strcmp(element->valuestring, "Yes")) || 
                (!strcmp(element->valuestring,"y")) || 
                (!strcmp(element->valuestring, "Y")) )
        {
            USE_ALL_DOMAINS = 1;
        }
        else
        {
            USE_ALL_DOMAINS = 0;
        }
    } else {
        //TODO
        printf("[JSON] use_all_domains type is not a string, using DEFAULT: no\n\r");
        //exit(1);
        USE_ALL_DOMAINS = 0;
    }

    element = cJSON_GetObjectItemCaseSensitive(jsonarray, "use_constant_deviation");
    if (cJSON_IsString(element) && (element->valuestring != NULL)){
        #ifdef JSON_DEBUG
        printf("[JSON] Checking if to use constant deviation: %s\n\r", element->valuestring);
        //Check of the correct string will be done after
        #endif
        if ( (!strcmp(element->valuestring, "yes")) || 
                (!strcmp(element->valuestring, "Yes")) || 
                (!strcmp(element->valuestring, "y")) || 
                (!strcmp(element->valuestring, "Y")) )
        {
            USE_CONSTANT_DEVIATION = 1;
            #ifdef JSON_DEBUG
            printf("[JSON] \tUsing constant deviation!\n\r");
            #endif
        }
        else
        {
            USE_CONSTANT_DEVIATION = 0;
        }
    } else {
        //TODO
        printf("[JSON] use_constant_deviation type is not a string, using DEFAULT: no\n\r");
        //exit(1);
        USE_CONSTANT_DEVIATION = 0;
    }
    element = cJSON_GetObjectItemCaseSensitive(jsonarray, "power_domains");
    if (cJSON_IsNumber(element) && (element->valueint > 0)) {
        #ifdef JSON_DEBUG
        printf("[JSON] Checking power domains number \"%d\"\n\r", element->valueint);
        #endif
        simulation.nb_power_domains = element->valueint;
    } else {
        //TODO
        printf("[JSON] power domains is not a number\n\r");
        exit(1);
    }
    cJSON *chiplets = cJSON_GetObjectItemCaseSensitive(jsonarray, "chiplets");
    nb_chiplet_links = 0;
    cJSON_ArrayForEach(element, chiplets) {nb_chiplet_links++;}
    struct chiplet_link_t *chiplet_link_ptr = malloc(sizeof(struct chiplet_link_t) * nb_chiplet_links);
    index = 0;
    cJSON_ArrayForEach(element, chiplets) {
        cJSON *chiplet_element = cJSON_GetObjectItemCaseSensitive(element, "index");
        if (cJSON_IsNumber(chiplet_element) && (chiplet_element->valueint >= 0)) {
            #ifdef JSON_DEBUG
            printf("[JSON] Checking chiplet %d index\n\r", index);
            #endif
            chiplet_link_ptr[index].index = chiplet_element->valueint;
        } else {
            //TODO
            printf("[JSON] Chiplet %d index is not a number\n\r", index);
            exit(1);
        }
        chiplet_element = cJSON_GetObjectItemCaseSensitive(element, "cid");
        if (cJSON_IsNumber(chiplet_element) && (chiplet_element->valueint > 0)) {
            #ifdef JSON_DEBUG
            printf("[JSON] Checking chiplet %d cid\n\r", index);
            #endif
            chiplet_link_ptr[index].cid = chiplet_element->valueint;
        } else {
            //TODO
            printf("[JSON] Chiplet %d cid is not a number\n\r", index);
            exit(1);
        }

        index++;
    }

    cJSON *component = cJSON_GetObjectItemCaseSensitive(jsonarray, "component");
    //First thing, we investigate the number for the Malloc
    total_component_number = 0;
    cJSON_ArrayForEach(element, component) {
        cJSON *number = cJSON_GetObjectItemCaseSensitive(element, "number");
        if (cJSON_IsNumber(number) && (number->valueint > 0)) {
            #ifdef JSON_DEBUG
            printf("[JSON] Checking components total number \"%d\"\n\r", number->valueint);
            #endif
            //total_component_number += (int)lround(number->valuedouble);
            total_component_number += number->valueint;
        } else {
            //TODO
            printf("[JSON] Components number is not a number\n\r");
            exit(1);
        }
    }
    // setting the number of element in the global struct "simulation"
    simulation.nb_elements = total_component_number;
    // initialization of the structure
    simulation.elements =
        malloc(sizeof(struct element_st) * simulation.nb_elements);
    //simulation->hw_config = malloc(sizeof(struct hw_config_st));

    //Then we populate
    uint32_t sim_element_counter = 0;
    cJSON_ArrayForEach(element, component) {

        uint32_t cid, number, domain, chiplet;   
        int pos[4]; 

        cJSON *core_element = cJSON_GetObjectItemCaseSensitive(element, "cid");
        if (cJSON_IsNumber(core_element) && (core_element->valueint > 0)) {
            #ifdef JSON_DEBUG
            printf("[JSON] Checking component %d cid\n\r", sim_element_counter);
            #endif
            cid = (uint32_t)core_element->valueint;
        } else {
            //TODO
            printf("[JSON] Component %d cid is not a number\n\r", sim_element_counter);
            exit(1);
        }
        core_element = cJSON_GetObjectItemCaseSensitive(element, "number");
        //no need to check number? Already checked before //TBD
        number = (uint32_t)core_element->valueint;
        core_element = cJSON_GetObjectItemCaseSensitive(element, "domain");
        if (cJSON_IsNumber(core_element) && (core_element->valueint >= 0)) {
            #ifdef JSON_DEBUG
            printf("[JSON] Checking component %d domain\n\r", sim_element_counter);
            #endif
            domain = (uint32_t)core_element->valueint;
        } else {
            //TODO
            printf("[JSON] Component %d domain is not a number\n\r", sim_element_counter);
            exit(1);
        }
        core_element = cJSON_GetObjectItemCaseSensitive(element, "chiplet");
        if (cJSON_IsNumber(core_element) && (core_element->valueint >= 0)) {
            #ifdef JSON_DEBUG
            printf("[JSON] Checking component %d chiplet\n\r", sim_element_counter);
            #endif
            chiplet = (uint32_t)core_element->valueint;
        } else {
            //TODO
            printf("[JSON] Component %d chiplet is not a number\n\r", sim_element_counter);
            exit(1);
        }



        for (int i = 0; i < number; i++)
        {
            /* Core: 1-255 (0FFF)
             * HBM: 256-511 (1FFF)
             *
             *
             * etc.
             */
            //simulation.elements[sim_element_counter].cid = (uint32_t)cid->valueint;
            simulation.elements[sim_element_counter].id = (uint32_t)i; //TOFO fix id. Give it a proper meaning.

            //TODO: chiplet
            float chiplet_silicon_var = 0.0f;
            for (int j = 0; j < nb_chiplet_links; j++)
            {
                if ( chiplet_link_ptr[j].index == chiplet)
                {
                    for (int k = 0; k < nb_chiplets; k++)
                    {
                        if ( chiplet_link_ptr[j].cid == chiplet_model_ptr[k].cid)
                        {
                            chiplet_silicon_var = chiplet_model_ptr[k].inter_wafer_silicon_var_perc;
                            break;
                        }
                    }                     
                    break;
                }
            }

            //TODO: Use this to also Connect Compute power
            //TODO use this also to populate the union
            if (cid < JSON_CORE_CID) {
                simulation.elements[sim_element_counter].type = JT_CORE;
                simulation.nb_cores++;

                //union config
                //simulation.elements[sim_element_counter].core_config.name =
                simulation.elements[sim_element_counter].core_config.cid = cid;
                //searching the component
                for (int j = 0; j < nb_core_components; j++)
                {
                    if (core_components[j].cid == simulation.elements[sim_element_counter].core_config.cid)
                    {
                        //The Gaussian noise silicon variation it is normalized over a 3*sigma=30 --> variance = 100
                        //TODO: to improve this, I may make a PAPER study on how the inter-die discrepancies are distributed
                        //      and instead of making it random, I will grab a ncore string of gaussian distributed values
                        //      and then distribute imperfection based on physic and research
                        float gaussian_var, res;
                        do {
                            float sigma = (float)sqrt(core_components[j].uncert.intra_die_silicon_var.variance);
                            generateGaussianNoise(&gaussian_var, NULL, 
                                    core_components[j].uncert.intra_die_silicon_var.mean, 
                                    sigma);
                            res = (gaussian_var * core_components[j].uncert.intra_die_silicon_var.three_sigma_prop_perc) / 30.0f;
                        } while((res < core_components[j].uncert.intra_die_silicon_var.variance_limits_perc[0]) ||
                                  (res > core_components[j].uncert.intra_die_silicon_var.variance_limits_perc[1]) );
                        //here I repeat instead of just doing saturation, because If I saturate, I would violate the normal distribution
                        //  given that at the boundaries I will have a spike = the integral of the gaussian distribution from limit to infinity.

                        if(!USE_CONSTANT_DEVIATION)
                        {
                            #ifdef JSON_DEBUG
                            printf("[JSON] \t core %d/%d variation: %f -- total: %f\n\r", i, core_components[j].cid, res, (1.0f+chiplet_silicon_var+res)*100.0f);
                            #endif
                            simulation.elements[sim_element_counter].core_config.leak_vdd = core_components[j].leak_vdd * (1.0f+chiplet_silicon_var+res);
                            simulation.elements[sim_element_counter].core_config.leak_temp = core_components[j].leak_temp * (1.0f+chiplet_silicon_var+res);
                            simulation.elements[sim_element_counter].core_config.leak_process = core_components[j].leak_process * (1.0f+chiplet_silicon_var+res);
                            simulation.elements[sim_element_counter].core_config.wl_states = core_components[j].wl_states;
                            simulation.elements[sim_element_counter].core_config.dyn_pow_cpu_coeff =
                                            (float*)malloc(sizeof(float)*core_components[j].wl_states);
                            for (int k = 0; k < core_components[j].wl_states; k++)
                                simulation.elements[sim_element_counter].core_config.dyn_pow_cpu_coeff[k] = core_components[j].dyn_pow_cpu_coeff[k] * (1.0f+chiplet_silicon_var+res);
                                //printf("%f\n", core_components[j].dyn_pow_cpu_coeff[k]);
                        }
                        else //not USE_CONSTANT_DEVIATION
                        {
                            #ifdef JSON_DEBUG
                            printf("[JSON] \t core %d/%d variation: %f -- total: %f\n\r", i, core_components[j].cid, res, constant_parameter_deviation[sim_element_counter]);
                            #endif
                            simulation.elements[sim_element_counter].core_config.leak_vdd = core_components[j].leak_vdd * constant_parameter_deviation[sim_element_counter] / 100.0f;
                            simulation.elements[sim_element_counter].core_config.leak_temp = core_components[j].leak_temp * constant_parameter_deviation[sim_element_counter] / 100.0f;
                            simulation.elements[sim_element_counter].core_config.leak_process = core_components[j].leak_process * constant_parameter_deviation[sim_element_counter] / 100.0f;
                            simulation.elements[sim_element_counter].core_config.wl_states = core_components[j].wl_states;
                            simulation.elements[sim_element_counter].core_config.dyn_pow_cpu_coeff =
                                            (float*)malloc(sizeof(float)*core_components[j].wl_states);
                            for (int k = 0; k < core_components[j].wl_states; k++)
                                simulation.elements[sim_element_counter].core_config.dyn_pow_cpu_coeff[k] = core_components[j].dyn_pow_cpu_coeff[k] * constant_parameter_deviation[sim_element_counter] / 100.0f;
                                //printf("%f\n", core_components[j].dyn_pow_cpu_coeff[k]);

                        }//not USE_CONSTANT_DEVIATION

                        //compute_power
                        simulation.elements[sim_element_counter].compute_power = compute_core_power_cid2; //TODO qua è hardcoded

                        simulation.elements[sim_element_counter].core_config.workload =
                                (float*)malloc(sizeof(float)*core_components[j].wl_states); 

                        break;
                    }
                }

            } else if (cid < JSON_HBM_CID) {
                simulation.elements[sim_element_counter].type = JT_HBM;


            } else {
                simulation.elements[sim_element_counter].type = JT_COMMON;
            }

            if (!USE_ALL_DOMAINS)
            {
                simulation.elements[sim_element_counter].domain = domain;
            }
            else //not USE_ALL_DOMAINS
            {
                simulation.elements[sim_element_counter].domain = sim_element_counter;
                simulation.nb_power_domains = simulation.nb_cores;
            } //not USE_ALL_DOMAINS

            /*
            cJSON *pos_point;
            pos_point = cJSON_GetObjectItemCaseSensitive(element, "p1");
            if (cJSON_IsNumber(pos_point) && (pos_point->valueint >= 0)) {
                #ifdef JSON_DEBUG
                printf("number \"%d\"\n", pos_point->valueint);
                #endif
            } else {
                //TODO
                printf("not a number");
                exit(1);
            }
            if (cJSON_IsNumber(pos_point->next) && (pos_point->next->valueint >= 0)) {
                #ifdef JSON_DEBUG
                printf("number \"%d\"\n", pos_point->next->valueint);
                #endif
            } else {
                //TODO
                printf("not a number");
                exit(1);
            }
            simulation.elements[sim_element_counter].position[0].x = pos_point->valueint;
            simulation.elements[sim_element_counter].position[0].y = pos_point->next->valueint;
            pos_point = cJSON_GetObjectItemCaseSensitive(element, "p3");
            if (cJSON_IsNumber(pos_point) && (pos_point->valueint >= 0)) {
                #ifdef JSON_DEBUG
                printf("number \"%d\"\n", pos_point->valueint);
                #endif
            } else {
                //TODO
                printf("not a number");
                exit(1);
            }
            if (cJSON_IsNumber(pos_point->next) && (pos_point->next->valueint >= 0)) {
                #ifdef JSON_DEBUG
                printf("number \"%d\"\n", pos_point->next->valueint);
                #endif
            } else {
                //TODO
                printf("not a number");
                exit(1);
            }
            simulation.elements[sim_element_counter].position[1].x = pos_point->valueint;
            simulation.elements[sim_element_counter].position[1].y = pos_point->next->valueint;
            */

            sim_element_counter++;
        }

    }


    // freeing the memory after using the json scructure
    jsonarray = cJSON_GetObjectItemCaseSensitive(json, "core");
    index=0;
    cJSON_ArrayForEach(element, jsonarray) {
        free(core_components[index].dyn_pow_cpu_coeff );
        index++;
    }

    free(chiplet_link_ptr);

    free(chiplet_model_ptr);
    free(core_components);
    free(common_components);

    cJSON_Delete(json);
    close(fd);

    
    return status;
}


// core power model function
float compute_core_power_cid1(struct element_st *self, float freq, float voltage, float temp,
                                float process,
                                float* workload_perc)
{
    float p_stat = 0.0;
    float p_dyn = 0.0;
    float tempC = temp - 273.15f;
    // computing the static power using precomputed coefficients called leak_*
    p_stat = ((voltage * (voltage / self->core_config.leak_vdd) * 1000.0f) +
              (tempC > 85.0f ? ((tempC * self->core_config.leak_temp) - (85.0f * self->core_config.leak_temp)) : 0.0f) +
              process * self->core_config.leak_process) / 1000.0f;
    // computing the dynamic power
    for (int i = 0; i < self->core_config.wl_states; i++) {
        //printf("%f - ", workload_perc[i]);
        p_dyn += workload_perc[i] * freq *
                 (voltage * self->core_config.dyn_pow_cpu_coeff[i] / 0.75) / 1000.0f;
    }
    //printf("\n");
    //printf("p_stat: %f ", p_stat);
    //printf(" - p_dyn: %f\n\r", p_dyn);
    // returning the total power
    return p_dyn + p_stat;
}

// core power model function
float compute_core_power_cid2(struct element_st *self, float freq, float voltage, float temp,
                                float process,
                                float* workload_perc)
{
    float p_stat = 0.0;
    float p_dyn = 0.0;
    float tempC = temp - 273.15f;
    // computing the static power using precomputed coefficients called leak_*
    p_stat = ( (voltage * self->core_config.leak_vdd ) +
              process * self->core_config.leak_process) / 1000.0f;

    //Themal relation of the leakage, including thermal inversion
    p_stat *= exp( (4507.748f * voltage + 29.903f * (tempC>90.0f?90.0:tempC) - 6033.035f) / 1000.0f );
    //printf("p: %f, exp: %f\n", p_stat, exp( (4507.748f * voltage + 29.903f * tempC - 6033.035f) / 1000.0f ));

    // computing the dynamic power
    for (int i = 0; i < self->core_config.wl_states; i++) {
        //printf("%f - ", workload_perc[i]);
        p_dyn += self->core_config.workload[i] * freq *
                 (voltage * voltage * self->core_config.dyn_pow_cpu_coeff[i] / 0.9 / 0.9) / 1000.0f;
    }
    //printf("\n");
    //printf("p_stat: %f ", p_stat);
    //printf(" - p_dyn: %f\n\r", p_dyn);
    // returning the total power

    if (global_finished[self->id] == 1)
    {
        p_dyn = 0;
        p_stat = 0.3;
    }
    
    return p_dyn + p_stat;
}
