/*
 * =======================================================================================
 *
 *      Filename:  threads.c
 *
 *      Description:  High level interface to pthreads
 *
 *      Version:   3.0
 *      Released:  29.11.2012
 *
 *      Author:  Jan Treibig (jt), jan.treibig@gmail.com
 *      Project:  likwid
 *
 *      Copyright (c) 2012, Jan Treibig
 *      All rights reserved.
 *
 *      Redistribution and use in source and binary forms, with or without
 *      modification, are permitted provided that the following conditions are met:
 *
 *      * Redistributions of source code must retain the above copyright notice, this
 *        list of conditions and the following disclaimer.
 *
 *      * Redistributions in binary form must reproduce the above copyright notice,
 *        this list of conditions and the following disclaimer in the documentation
 *        and/or other materials provided with the distribution.
 *
 *      THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 *      ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 *      WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *      DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 *      FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 *      DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 *      SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *      CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 *      OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *      OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * =======================================================================================
 */

/* #####   HEADER FILE INCLUDES   ######################################### */

#include <stdlib.h>
#include <stdio.h>

#include <error.h>
#include <types.h>
#include <threads.h>


/* #####   EXPORTED VARIABLES   ########################################### */

pthread_barrier_t threads_barrier;
ThreadData* threads_data;
ThreadGroup* threads_groups;

/* #####   VARIABLES  -  LOCAL TO THIS SOURCE FILE   ###################### */

static pthread_t* threads = NULL;
static pthread_attr_t attr;
static int numThreads = 0;


/* #####   FUNCTION DEFINITIONS  -  EXPORTED FUNCTIONS   ################## */

void
threads_init(int numberOfThreads)
{
    int i;
    numThreads = numberOfThreads;

    threads = (pthread_t*) malloc(numThreads * sizeof(pthread_t));
    threads_data = (ThreadData*) malloc(numThreads * sizeof(ThreadData));

    for(i = 0; i < numThreads; i++)
    {
        threads_data[i].numberOfThreads = numThreads;
        threads_data[i].globalNumberOfThreads = numThreads;
        threads_data[i].globalThreadId = i;
        threads_data[i].threadId = i;
    }

    pthread_barrier_init(&threads_barrier, NULL, numThreads);
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
}


void 
threads_create(void *(*startRoutine)(void*))
{
    int i;

    for(i = 0; i < numThreads; i++)
    {
        pthread_create(&threads[i],
                &attr,
                startRoutine,
                (void*) &threads_data[i]); 
    }
}

void 
threads_createGroups(int numberOfGroups)
{
    int i;
    int j;
    int numThreadsPerGroup;
    int globalId = 0;

    if (numThreads % numberOfGroups)
    {
        ERROR_PRINT(Not enough threads %d to create %d groups,numThreads,numberOfGroups);
    }
    else 
    {
        numThreadsPerGroup = numThreads / numberOfGroups;
    }

    threads_groups = (ThreadGroup*) malloc(numberOfGroups *
            sizeof(ThreadGroup));

    for (i = 0; i < numberOfGroups; i++)
    {
        threads_groups[i].numberOfThreads = numThreadsPerGroup;
        threads_groups[i].threadIds = (int*) malloc(numThreadsPerGroup *
                sizeof(int));

        for (j = 0; j < numThreadsPerGroup; j++)
        {
            threads_data[globalId].threadId = j;
            threads_data[globalId].groupId = i;
            threads_data[globalId].numberOfGroups = numberOfGroups;
            threads_data[globalId].numberOfThreads = numThreadsPerGroup;
            threads_groups[i].threadIds[j] = globalId++;
        }
    }
}


void 
threads_registerDataAll(ThreadUserData* data, threads_copyDataFunc func)
{
    int i;

    if (func == NULL)
    {
        for(i = 0; i < numThreads; i++)
        {
            threads_data[i].data = (*data);
        }
    }
    else
    {
        for(i = 0; i < numThreads; i++)
        {
            func( data, &threads_data[i].data);
        }
    }
}

void
threads_registerDataThread(int threadId,
        ThreadUserData* data,
        threads_copyDataFunc func)
{
    if (func == NULL)
    {
        threads_data[threadId].data = (*data);
    }
    else
    {
        func( data, &threads_data[threadId].data);
    }
}

void
threads_registerDataGroup(int groupId,
        ThreadUserData* data,
        threads_copyDataFunc func)
{
    int i;

    if (func == NULL)
    {
        for (i = 0; i < threads_groups[groupId].numberOfThreads; i++)
        {
            threads_data[threads_groups[groupId].threadIds[i]].data = (*data);
        }
    }
    else
    {
        for (i = 0; i < threads_groups[groupId].numberOfThreads; i++)
        {
            func( data,
                    &threads_data[threads_groups[groupId].threadIds[i]].data);
        }
    }
}

void
threads_destroy(void)
{
    int i;

    for(i=0; i < numThreads; i++)
    {
        pthread_join(threads[i], NULL);
    }

    pthread_attr_destroy(&attr);
}

