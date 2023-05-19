/* Copied Unibo exercise */
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
