
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
 * wl_config.h
 *
 *  Created on: 19 giu 2020
 *      Author: giova
 */

#ifndef INC_WL_CONFIG_H_
#define INC_WL_CONFIG_H_

#include "cmdconf.h"

#include <stdint.h>

#define N_WL_SEQ                8 // 7

typedef struct _wl_block_struct {
	uint32_t state;
	uint32_t cycles;
} wl_block_t;

typedef struct _Macroblock {
	wl_block_t* block_name;
	uint32_t times;
} vMacroblock_t;

typedef struct _Seqblock {
	vMacroblock_t* block_name;
	uint32_t times;
} vSeqblock_t;

//typedef struct _wl_Mb_translation {
//	;
//	uint16_t times;
//} wl_Mb_modelled_t;


//To simplify and better thing, all blocks and Macroblock should end with 0/0

void initWlTransl(void);
void execWlTransl(uint32_t* icycles_per_step);






#endif //lib
