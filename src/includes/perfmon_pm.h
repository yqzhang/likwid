/*
 * =======================================================================================
 *
 *      Filename:  perfmon_pm.h
 *
 *      Description:  Header File of perfmon module.
 *                    Configures and reads out performance counters
 *                    on x86 based architectures. Supports multi threading.
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

#include <bstrlib.h>
#include <types.h>
#include <registers.h>
#include <perfmon_pm_events.h>

#define NUM_COUNTERS_PM 2
#define NUM_GROUPS_PM 5

static int perfmon_numCounters_pm = NUM_COUNTERS_PM;
static int perfmon_numGroups_pm = NUM_GROUPS_PM;
static int perfmon_numArchEvents_pm = NUM_ARCH_EVENTS_PM;

static PerfmonCounterMap pm_counter_map[NUM_COUNTERS_PM] = {
    {"PMC0",PMC0},
    {"PMC1",PMC1}
};

static PerfmonGroupMap pm_group_map[NUM_GROUPS_PM] = {
	{"FLOPS_DP",FLOPS_DP,"Double Precision MFlops/s","EMON_SSE_SSE2_COMP_INST_RETIRED_PACKED_DP:PMC0,EMON_SSE_SSE2_COMP_INST_RETIRED_SCALAR_DP:PMC1"},
	{"FLOPS_SP",FLOPS_SP,"Single Precision MFlops/s","EMON_SSE_SSE2_COMP_INST_RETIRED_ALL_SP:PMC0,EMON_SSE_SSE2_COMP_INST_RETIRED_SCALAR_SP:PMC1"},
	{"L2",L2,"L2 cache bandwidth in MBytes/s","L2_LINES_IN_ALL_ALL:PMC0,L2_LINES_OUT_ALL_ALL:PMC1"},
	{"BRANCH",BRANCH,"Branch prediction miss rate","BR_INST_EXEC:PMC0,BR_INST_MISSP_EXEC:PMC1"},
	{"CPI",CPI,"Cycles per instruction","UOPS_RETIRED:PMC0"}
};

void 
perfmon_init_pm(PerfmonThread *thread)
{
    uint64_t flags = 0x0ULL;
    int cpu_id = thread->processorId;

    thread->counters[PMC0].configRegister = MSR_PERFEVTSEL0;
    thread->counters[PMC0].counterRegister = MSR_PMC0;
    thread->counters[PMC0].type = PMC;
    thread->counters[PMC1].configRegister = MSR_PERFEVTSEL1;
    thread->counters[PMC1].counterRegister = MSR_PMC1;
    thread->counters[PMC1].type = PMC;

    msr_write(cpu_id, MSR_PERFEVTSEL0, 0x0ULL);
    msr_write(cpu_id, MSR_PERFEVTSEL1, 0x0ULL);

    /* Preinit of two PMC counters */
    flags |= (1<<16);  /* user mode flag */
    flags |= (1<<19);  /* pin control flag */
    //    flags |= (1<<22);  /* enable flag */

    msr_write(cpu_id, MSR_PERFEVTSEL0, flags);
    msr_write(cpu_id, MSR_PERFEVTSEL1, flags);
}

void
perfmon_setupCounterThread_pm(int thread_id,
        PerfmonEvent* event,
        PerfmonCounterIndex index)
{
    uint64_t flags;
    uint64_t reg = perfmon_threadData[thread_id].counters[index].configRegister;
    int cpu_id = perfmon_threadData[thread_id].processorId;

    if (perfmon_threadData[thread_id].counters[index].type == PMC)
    {

        perfmon_threadData[thread_id].counters[index].init = TRUE;
        flags = msr_read(cpu_id,reg);
        flags &= ~(0xFFFFU); 

        /* Intel with standard 8 bit event mask: [7:0] */
        flags |= (event->umask<<8) + event->eventId;

        msr_write(cpu_id, reg , flags);

        if (perfmon_verbose)
        {
            printf("[%d] perfmon_setup_counter: Write Register 0x%llX , Flags: 0x%llX \n",
                    cpu_id,
                    LLU_CAST reg,
                    LLU_CAST flags);
        }
    }
}


void
perfmon_startCountersThread_pm(int thread_id)
{
    uint64_t flags = 0ULL;
    int processorId = perfmon_threadData[thread_id].processorId;

    if (perfmon_threadData[thread_id].counters[0].init == TRUE) {
        msr_write(processorId, perfmon_threadData[thread_id].counters[0].counterRegister , 0x0ULL);
        msr_write(processorId, perfmon_threadData[thread_id].counters[1].counterRegister , 0x0ULL);

        /* on p6 only MSR_PERFEVTSEL0 has the enable bit
         * it enables both counters as long MSR_PERFEVTSEL1 
         * has a valid configuration */
        flags = msr_read(processorId, MSR_PERFEVTSEL0);
        flags |= (1<<22);  /* enable flag */

        if (perfmon_verbose)
        {
            printf("perfmon_start_counters: Write Register 0x%X , Flags: 0x%llX \n",MSR_PERFEVTSEL0, LLU_CAST flags);
        }

        msr_write(processorId, MSR_PERFEVTSEL0, flags);
    }

}

void 
perfmon_stopCountersThread_pm(int thread_id)
{
    int i;
    int cpu_id = perfmon_threadData[thread_id].processorId;

    msr_write(cpu_id, MSR_PERFEVTSEL0, 0x0ULL);
    msr_write(cpu_id, MSR_PERFEVTSEL1, 0x0ULL);

    for (i=0;i<NUM_COUNTERS_PM;i++) 
    {
        if (perfmon_threadData[thread_id].counters[i].init == TRUE) 
        {
            perfmon_threadData[thread_id].counters[i].counterData =
				msr_read(cpu_id, perfmon_threadData[thread_id].counters[i].counterRegister);
        }
    }
}

void
perfmon_printDerivedMetrics_pm(PerfmonGroup group)
{

    switch ( group ) 
    {
        case FLOPS_DP:

        case FLOPS_SP:

        case L2:

        case BRANCH:

        case _NOGROUP:
            fprintf (stderr, "The Pentium M supports only two counters. Therefore derived metrics are not computed due to missing runtime!\n" );
            break;

        default:
            fprintf (stderr, "perfmon_printDerivedMetricsCore2: Unknown group! Exiting!\n" );
            exit (EXIT_FAILURE);
            break;
    }
}


