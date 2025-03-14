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

#ifndef _THREAD_SAFE
	#define _THREAD_SAFE
#endif

#include <unistd.h>     /* Symbolic Constants */
#include <sys/types.h>  /* Primitive System Data Types */
#include <errno.h>      /* Errors */
#include <stdio.h>      /* Input/Output */
#include <stdlib.h>     /* General Utilities */
#include <string.h>     /* String handling */
#include <pthread.h>    /* POSIX Threads */

//#include "printerror.h"
#include "mySem.h"

//#include "main.h"

int mySem_init( mySem_t *mysem, int pshared, unsigned int value, int sem_type ) {

	int rc;
	int return_value = 0;
	pthread_mutexattr_t lock_attr;

	rc = pthread_mutexattr_init(&lock_attr);
	if (rc) {printf("Error attr for Mutex Init: %d\n\r", rc);}
	rc = pthread_mutexattr_settype(&lock_attr, PTHREAD_MUTEX_ERRORCHECK);
	if (rc) {printf("Error attr for Mutex Init: %d\n\r", rc);}
	rc = pthread_mutexattr_setprotocol(&lock_attr, PTHREAD_PRIO_INHERIT);
	if (rc) {printf("Error attr for Mutex Init: %d\n\r", rc);}

	rc = pthread_mutex_init( &(mysem->mutex), &lock_attr );
	if( rc ) {printf("pthread_mutex_init failed - %d\n\r", rc); exit(1);}
	return_value = rc;
	rc = pthread_cond_init( &(mysem->cond) , NULL );
	if( rc ) {printf("pthread_cond_init failed - %d\n\r", rc); exit(1);}
	return_value = (return_value | rc);
	mysem->counter=value;
	mysem->type=sem_type;

	return(return_value);
}

int mySem_destroy( mySem_t *mysem ) {
	int rc;
	int return_value = 0;

	rc = pthread_mutex_destroy( &(mysem->mutex) );
	if(rc) {printf("pthread_mutex_destroy failed - %d\n\r", rc); exit(1);}
	return_value = rc;
	rc = pthread_cond_destroy( &(mysem->cond) );
	if(rc) {printf("pthread_cond_destroy failed - %d\n\r", rc); exit(1);}
	return_value = (return_value | rc)  ;

	return(return_value);
}

int mySem_wait( mySem_t *mysem ) {
	int rc;
	int return_value = 0;

	rc = pthread_mutex_lock( &(mysem->mutex) );
	if(rc) {printf("pthread_mutex_lock failed (wait) - %d\n\r", rc); exit(1);}
	return_value = rc;
	while( mysem->counter <= 0 ) {
		rc = pthread_cond_wait( &(mysem->cond), &(mysem->mutex) );
		if(rc) {printf("pthread_cond_wait failed - %d\n\r", rc); exit(1);}
		return_value = (return_value | rc);
	}
	if (mysem->type == mySem_Binary)
	{
		if (mysem->counter > 1) return_value = (return_value | -1024);
		mysem->counter=0;
	}
	else
		mysem->counter--;

	rc = pthread_mutex_unlock( &(mysem->mutex) );
	if(rc) {printf("pthread_mutex_unlock failed (wait) - %d\n\r", rc); exit(1);}
	return_value = (return_value | rc);

	return(return_value);
}

int mySem_post( mySem_t *mysem ) {
	int rc;
	int return_value = 0;

	rc = pthread_mutex_lock( &(mysem->mutex) );
	if(rc) {printf("pthread_mutex_lock failed (post) - %d\n\r", rc);
	/*for (int i = 0; i < 4; ++i)
	{
		for (int j = 0; j < 40; ++j)
		{
			printf("Thread: %d, Id: %d, nsec: %ld \n\r", i, j, threadtime[i][j].tv_nsec);
		}
		printf("\n\r");
	}*/
	exit(1);}
	return_value = rc;
	if (mysem->type == mySem_Binary)
	{
		if ((mysem->counter < 0) || (mysem->counter > 1)) return_value = (return_value | -1024);
		mysem->counter=1;
	}
	else
		mysem->counter++;
	if( mysem->counter > 0 ) {
		rc = pthread_cond_signal( &(mysem->cond) );
		if(rc) {printf("pthread_cond_signal failed - %d\n\r", rc); exit(1);}
		return_value = (return_value | rc);
	}
	rc = pthread_mutex_unlock( &(mysem->mutex) );
	if(rc) {printf("pthread_mutex_unlock failed (post) - %d\n\r", rc); exit(1);}
	return_value = (return_value | rc);

	return(return_value);
}
