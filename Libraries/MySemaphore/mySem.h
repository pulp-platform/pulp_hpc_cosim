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

/* Inspired by the following exercise: */
/* https://www.cs.unibo.it/~ghini/didattica/sistemioperativi/PTHREAD/POSIX_SEMAPHORES/MYSEM/ */

#ifndef __MYSEM_H__
#define __MYSEM_H__

#include <unistd.h>     /* Symbolic Constants */
#include <sys/types.h>  /* Primitive System Data Types */
#include <errno.h>      /* Errors */
#include <stdio.h>      /* Input/Output */
#include <stdlib.h>     /* General Utilities */
#include <string.h>     /* String handling */
#include <pthread.h>    /* POSIX Threads */

enum mySem_type
{
	mySem_Default,
	mySem_Binary
};

typedef struct mySem_t {
	pthread_mutex_t mutex;
	pthread_cond_t  cond;
	int counter;
	int type;
} mySem_t;

int mySem_init( mySem_t *mysem, int pshared, unsigned int value, int sem_type );
int mySem_wait( mySem_t *mysem );
int mySem_post( mySem_t *mysem );
int mySem_destroy( mySem_t *mysem);

#endif	/* __MYSEM_H__ */
