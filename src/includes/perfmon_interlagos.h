/*
 * =======================================================================================
 *
 *      Filename:  perfmon_interlagos.h
 *
 *      Description:  AMD Interlagos specific subroutines
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

#include <perfmon_interlagos_events.h>
#include <perfmon_interlagos_groups.h>

#define NUM_COUNTERS_INTERLAGOS 10

static int perfmon_numCountersInterlagos = NUM_COUNTERS_INTERLAGOS;
static int perfmon_numGroupsInterlagos = NUM_GROUPS_INTERLAGOS;
static int perfmon_numArchEventsInterlagos = NUM_ARCH_EVENTS_INTERLAGOS;

static PerfmonCounterMap interlagos_counter_map[NUM_COUNTERS_INTERLAGOS] = {
    {"PMC0",PMC0},
    {"PMC1",PMC1},
    {"PMC2",PMC2},
    {"PMC3",PMC3},
    {"PMC4",PMC4},
    {"PMC5",PMC5},
    {"UPMC0",PMC6},
    {"UPMC1",PMC7},
    {"UPMC2",PMC8},
    {"UPMC3",PMC9}
};

void perfmon_init_interlagos(PerfmonThread *thread)
{
    uint64_t flags = 0x0ULL;
    int cpu_id = thread->processorId;

    /* Core counters */
    thread->counters[PMC0].configRegister = MSR_AMD15_PERFEVTSEL0;
    thread->counters[PMC0].counterRegister = MSR_AMD15_PMC0;
    thread->counters[PMC0].type = PMC;
    thread->counters[PMC1].configRegister = MSR_AMD15_PERFEVTSEL1;
    thread->counters[PMC1].counterRegister = MSR_AMD15_PMC1;
    thread->counters[PMC1].type = PMC;
    thread->counters[PMC2].configRegister = MSR_AMD15_PERFEVTSEL2;
    thread->counters[PMC2].counterRegister = MSR_AMD15_PMC2;
    thread->counters[PMC2].type = PMC;
    thread->counters[PMC3].configRegister = MSR_AMD15_PERFEVTSEL3;
    thread->counters[PMC3].counterRegister = MSR_AMD15_PMC3;
    thread->counters[PMC3].type = PMC;
    thread->counters[PMC4].configRegister = MSR_AMD15_PERFEVTSEL4;
    thread->counters[PMC4].counterRegister = MSR_AMD15_PMC4;
    thread->counters[PMC4].type = PMC;
    thread->counters[PMC5].configRegister = MSR_AMD15_PERFEVTSEL5;
    thread->counters[PMC5].counterRegister = MSR_AMD15_PMC5;
    thread->counters[PMC5].type = PMC;

    /* Northbridge counters */
    thread->counters[PMC6].configRegister = MSR_AMD15_NB_PERFEVTSEL0;
    thread->counters[PMC6].counterRegister = MSR_AMD15_NB_PMC0;
    thread->counters[PMC6].type = UNCORE;
    thread->counters[PMC7].configRegister = MSR_AMD15_NB_PERFEVTSEL1;
    thread->counters[PMC7].counterRegister = MSR_AMD15_NB_PMC1;
    thread->counters[PMC7].type = UNCORE;
    thread->counters[PMC8].configRegister = MSR_AMD15_NB_PERFEVTSEL2;
    thread->counters[PMC8].counterRegister = MSR_AMD15_NB_PMC2;
    thread->counters[PMC8].type = UNCORE;
    thread->counters[PMC9].configRegister = MSR_AMD15_NB_PERFEVTSEL3;
    thread->counters[PMC9].counterRegister = MSR_AMD15_NB_PMC3;
    thread->counters[PMC9].type = UNCORE;

    msr_write(cpu_id, MSR_AMD15_PERFEVTSEL0, 0x0ULL);
    msr_write(cpu_id, MSR_AMD15_PERFEVTSEL1, 0x0ULL);
    msr_write(cpu_id, MSR_AMD15_PERFEVTSEL2, 0x0ULL);
    msr_write(cpu_id, MSR_AMD15_PERFEVTSEL3, 0x0ULL);
    msr_write(cpu_id, MSR_AMD15_PERFEVTSEL4, 0x0ULL);
    msr_write(cpu_id, MSR_AMD15_PERFEVTSEL5, 0x0ULL);

    if ((socket_lock[affinity_core2node_lookup[cpu_id]] == cpu_id) ||
            lock_acquire(
                (int*) &socket_lock[affinity_core2node_lookup[cpu_id]], cpu_id)
            )
    {
        msr_write(cpu_id, MSR_AMD15_NB_PERFEVTSEL0, 0x0ULL);
        msr_write(cpu_id, MSR_AMD15_NB_PERFEVTSEL1, 0x0ULL);
        msr_write(cpu_id, MSR_AMD15_NB_PERFEVTSEL2, 0x0ULL);
        msr_write(cpu_id, MSR_AMD15_NB_PERFEVTSEL3, 0x0ULL);
    }

    flags |= (1<<16);  /* user mode flag */
    msr_write(cpu_id, MSR_AMD15_PERFEVTSEL0, flags);
    msr_write(cpu_id, MSR_AMD15_PERFEVTSEL1, flags);
    msr_write(cpu_id, MSR_AMD15_PERFEVTSEL2, flags);
    msr_write(cpu_id, MSR_AMD15_PERFEVTSEL3, flags);
    msr_write(cpu_id, MSR_AMD15_PERFEVTSEL4, flags);
    msr_write(cpu_id, MSR_AMD15_PERFEVTSEL5, flags);
}


void
perfmon_setupCounterThread_interlagos(
        int thread_id,
        PerfmonEvent* event,
        PerfmonCounterIndex index)
{
    uint64_t flags;
    uint64_t reg = perfmon_threadData[thread_id].counters[index].configRegister;
    int cpu_id = perfmon_threadData[thread_id].processorId;
    perfmon_threadData[thread_id].counters[index].init = TRUE;

    /* only one thread accesses Uncore */
    if ( (perfmon_threadData[thread_id].counters[index].type == UNCORE) &&
            !(socket_lock[affinity_core2node_lookup[cpu_id]] == cpu_id) )
    {
        return;
    }

    flags = msr_read(cpu_id,reg);
    flags &= ~(0xFFFFU); 

	/* AMD uses a 12 bit Event mask: [35:32][7:0] */
	flags |= ((uint64_t)(event->eventId>>8)<<32) + (event->umask<<8) + (event->eventId & ~(0xF00U));

    if (perfmon_verbose)
    {
        printf("[%d] perfmon_setup_counter: Write Register 0x%llX , Flags: 0x%llX \n",
               cpu_id,
               LLU_CAST reg,
               LLU_CAST flags);
    }
    msr_write(cpu_id, reg , flags);
}


void
perfmon_startCountersThread_interlagos(int thread_id)
{
    int haveLock = 0;
    uint64_t flags;
    int cpu_id = perfmon_threadData[thread_id].processorId;

    if ((socket_lock[affinity_core2node_lookup[cpu_id]] == cpu_id))
    {
        haveLock = 1;
    }

    for ( int i=0; i<NUM_COUNTERS_INTERLAGOS; i++ ) 
    {
        if (perfmon_threadData[thread_id].counters[i].init == TRUE) 
        {
            if (perfmon_threadData[thread_id].counters[i].type == PMC)
            {
                msr_write(cpu_id, perfmon_threadData[thread_id].counters[i].counterRegister , 0x0ULL);
                flags = msr_read(cpu_id,perfmon_threadData[thread_id].counters[i].configRegister);
                flags |= (1<<22);  /* enable flag */

                if (perfmon_verbose) 
                {
                    printf("perfmon_start_counters: Write Register 0x%llX , Flags: 0x%llX \n",
                            LLU_CAST perfmon_threadData[thread_id].counters[i].configRegister,
                            LLU_CAST flags);
                }

                msr_write(cpu_id, perfmon_threadData[thread_id].counters[i].configRegister , flags);

            }
            else if (perfmon_threadData[thread_id].counters[i].type == UNCORE)
            {
                if(haveLock)
                {
                    msr_write(cpu_id, perfmon_threadData[thread_id].counters[i].counterRegister , 0x0ULL);
                    flags = msr_read(cpu_id,perfmon_threadData[thread_id].counters[i].configRegister);
                    flags |= (1<<22);  /* enable flag */

                    if (perfmon_verbose) 
                    {
                        printf("perfmon_start_counters: Write Register 0x%llX , Flags: 0x%llX \n",
                                LLU_CAST perfmon_threadData[thread_id].counters[i].configRegister,
                                LLU_CAST flags);
                    }

                    msr_write(cpu_id, perfmon_threadData[thread_id].counters[i].configRegister , flags);
                }
            }
        }
    }
}

void 
perfmon_stopCountersThread_interlagos(int thread_id)
{
    uint64_t flags;
    int haveLock = 0;
    int cpu_id = perfmon_threadData[thread_id].processorId;

    if ((socket_lock[affinity_core2node_lookup[cpu_id]] == cpu_id))
    {
        haveLock = 1;
    }

    for ( int i=0; i<NUM_COUNTERS_INTERLAGOS; i++ ) 
    {
        if (perfmon_threadData[thread_id].counters[i].init == TRUE) 
        {
            if (perfmon_threadData[thread_id].counters[i].type == PMC)
            {
                flags = msr_read(cpu_id,perfmon_threadData[thread_id].counters[i].configRegister);
                flags &= ~(1<<22);  /* clear enable flag */
                msr_write(cpu_id, perfmon_threadData[thread_id].counters[i].configRegister , flags);
                perfmon_threadData[thread_id].counters[i].counterData =
                    msr_read(cpu_id, perfmon_threadData[thread_id].counters[i].counterRegister);

                if (perfmon_verbose)
                {
                    printf("perfmon_stop_counters: Write Register 0x%llX , Flags: 0x%llX \n",
                            LLU_CAST perfmon_threadData[thread_id].counters[i].configRegister,
                            LLU_CAST flags);
                    printf("perfmon_stop_counters: Read Register 0x%llX , Flags: 0x%llX \n",
                            LLU_CAST perfmon_threadData[thread_id].counters[i].counterRegister,
                            LLU_CAST perfmon_threadData[thread_id].counters[i].counterData);
                }

            }
            else if (perfmon_threadData[thread_id].counters[i].type == UNCORE)
            {
                if(haveLock)
                {
                    flags = msr_read(cpu_id,perfmon_threadData[thread_id].counters[i].configRegister);
                    flags &= ~(1<<22);  /* clear enable flag */
                    msr_write(cpu_id, perfmon_threadData[thread_id].counters[i].configRegister , flags);
                    if (perfmon_verbose)
                    {
                        printf("perfmon_stop_counters: Write Register 0x%llX , Flags: 0x%llX \n",
                                LLU_CAST perfmon_threadData[thread_id].counters[i].configRegister,
                                LLU_CAST flags);
                    }
                    perfmon_threadData[thread_id].counters[i].counterData =
                        msr_read(cpu_id, perfmon_threadData[thread_id].counters[i].counterRegister);

                }
            }
        }
    }
}


void 
perfmon_readCountersThread_interlagos(int thread_id)
{
    int haveLock = 0;
    int cpu_id = perfmon_threadData[thread_id].processorId;

    if (socket_lock[affinity_core2node_lookup[cpu_id]] == cpu_id)
    {
        haveLock = 1;
    }


    for (int i=0;i<NUM_COUNTERS_INTERLAGOS;i++) 
    {
        if (perfmon_threadData[thread_id].counters[i].init == TRUE) 
        { 
            if (perfmon_threadData[thread_id].counters[i].type == UNCORE)
            {
                if(haveLock)
                {

                    perfmon_threadData[thread_id].counters[i].counterData =
                        msr_read(cpu_id, perfmon_threadData[thread_id].counters[i].counterRegister);
                }
            }
            else
            {
                perfmon_threadData[thread_id].counters[i].counterData =
                    msr_read(cpu_id, perfmon_threadData[thread_id].counters[i].counterRegister);
            }
        }
    }
}


