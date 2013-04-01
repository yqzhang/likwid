/*
 * =======================================================================================
 *
 *      Filename:  multiplex.c
 *
 *      Description:  
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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/time.h>

#include <timer.h>
#include <perfmon.h>
#include <multiplex.h>

#if 0
static int currentCollection = -1;
static MultiplexCollections* multiplex_set = NULL;
static CyclesData timeData;
static int  multiplex_useMarker = 0;

void
multiplex_printCounters ()
{



}



void
multiplex_swapEventSet ()
{
    int threadId;
    PerfmonEventSet* collection;

    /* collection from last run */
    collection = multiplex_set->collections + currentCollection;

    for (threadId = 0; threadId < perfmon_numThreads; threadId++)
    {
        /* Stop counters */
        if (!multiplex_useMarker) perfmon_stopCountersThread(threadId);
        /* Accumulate counters */
        for (int i=0; i<collection->numberOfEvents; i++)
        {
//            collection->events[i].result[threadId] += 
 //               (double) perfmon_threadData[threadId].counters[collection->events[i].index].counterData;
        }
    }

    /* switch to next collection */
    if( currentCollection == multiplex_set->numberOfCollections-1)
    {
        currentCollection = 0;
    }
    else
    {
        currentCollection++;
    }
    collection = multiplex_set->collections + currentCollection;

    for (threadId = 0; threadId < perfmon_numThreads; threadId++)
    {
        /* Reconfigure counters */
        for (int i=0; i<collection->numberOfEvents; i++)
        {
            perfmon_setupCounterThread(threadId,
                    collection->events[i].event.eventId,
                    collection->events[i].event.umask,
                    collection->events[i].index);
        }

        /* Start counters */
       if (!multiplex_useMarker)  perfmon_startCountersThread(threadId);
    }
}

void
multiplex_init(MultiplexCollections* set)
{
    int i;

    multiplex_set = set;

    for (i=0;i<multiplex_set->numberOfCollections; i++)
    {
//        perfmon_initEventset(multiplex_set->collections+i);
    }
}

void
multiplex_start()
{
    struct itimerval val;
    struct sigaction sa;

//    multiplex_useMarker = useMarker;

    val.it_interval.tv_sec = 0;
    val.it_interval.tv_usec = 500;
    val.it_value.tv_sec = 0; 
    val.it_value.tv_usec = 100;

    sa.sa_handler = multiplex_printCounters;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGALRM, &sa, NULL) == -1)
    {
        /* Handle error */;
        perror("sigaction");
    }

    perfmon_startCounters();
    setitimer(ITIMER_REAL, &val,0);
    timer_startCycles(&timeData);
}

void
multiplex_stop()
{
    struct itimerval val;

    val.it_interval.tv_sec = 0;
    val.it_interval.tv_usec = 0;
    val.it_value.tv_sec = 0; 
    val.it_value.tv_usec = 0;

    timer_stopCycles(&timeData);
    setitimer(ITIMER_REAL, &val,0);
    perfmon_stopCounters();

    multiplex_set->time = timer_printCyclesTime(&timeData);
}

#endif


