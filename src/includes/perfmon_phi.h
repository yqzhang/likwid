/*
 * =======================================================================================
 *
 *      Filename:  perfmon_phi.h
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

#include <perfmon_phi_events.h>
#include <perfmon_phi_groups.h>

#define NUM_COUNTERS_PHI 2

static int perfmon_numCountersPhi = NUM_COUNTERS_PHI;
static int perfmon_numGroupsPhi = NUM_GROUPS_PHI;
static int perfmon_numArchEventsPhi = NUM_ARCH_EVENTS_PHI;

static PerfmonCounterMap phi_counter_map[NUM_COUNTERS_PHI] = {
    {"PMC0",PMC0},
    {"PMC1",PMC1}
};

void 
perfmon_init_phi(PerfmonThread *thread)
{
    uint32_t flags = 0x0UL;
    int cpu_id = thread->processorId;

    thread->counters[PMC0].configRegister = MSR_MIC_PERFEVTSEL0;
    thread->counters[PMC0].counterRegister = MSR_MIC_PMC0;
    thread->counters[PMC0].type = PMC;
    thread->counters[PMC1].configRegister = MSR_MIC_PERFEVTSEL1;
    thread->counters[PMC1].counterRegister = MSR_MIC_PMC1;
    thread->counters[PMC1].type = PMC;

    msr_write(cpu_id, MSR_MIC_PERFEVTSEL0, 0x0UL);
    msr_write(cpu_id, MSR_MIC_PERFEVTSEL1, 0x0UL);
    msr_write(cpu_id, MSR_MIC_PMC0, 0x0ULL);
    msr_write(cpu_id, MSR_MIC_PMC1, 0x0ULL);
    msr_write(cpu_id, MSR_MIC_SPFLT_CONTROL, 0x0ULL);
    msr_write(cpu_id, MSR_MIC_PERF_GLOBAL_CTRL, 0x0ULL);
    msr_write(cpu_id, MSR_MIC_PERF_GLOBAL_OVF_CTRL, 0x0ULL);

    flags |= (1<<16);  /* user mode flag */
    flags |= (1<<22);  /* enable flag */

    msr_write(cpu_id, MSR_MIC_PERFEVTSEL0, flags);
    msr_write(cpu_id, MSR_MIC_PERFEVTSEL1, flags);
}

void
perfmon_setupCounterThread_phi(int thread_id,
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
perfmon_startCountersThread_phi(int thread_id)
{
    uint64_t flags = 0ULL;
    int processorId = perfmon_threadData[thread_id].processorId;

    msr_write(processorId, MSR_MIC_PERF_GLOBAL_CTRL, 0x0ULL);

    for ( int i=0; i<NUM_COUNTERS_PHI; i++ ) 
    {
        if (perfmon_threadData[thread_id].counters[i].init == TRUE) 
        {
            msr_write(processorId, perfmon_threadData[thread_id].counters[i].counterRegister , 0x0ULL);
            flags |= (1<<(i));  /* enable counter */
        }
    }

    if (perfmon_verbose)
    {
        printf("perfmon_start_counters: Write Register 0x%X , Flags: 0x%llX \n",MSR_MIC_PERF_GLOBAL_CTRL, LLU_CAST flags);
    }

    msr_write(processorId, MSR_MIC_PERF_GLOBAL_CTRL, flags);
    flags |= (1ULL<<63);
    msr_write(processorId, MSR_MIC_SPFLT_CONTROL, flags);
    msr_write(processorId, MSR_MIC_PERF_GLOBAL_OVF_CTRL, 0x000000003ULL);
}

void 
perfmon_stopCountersThread_phi(int thread_id)
{
    uint64_t flags;
    int cpu_id = perfmon_threadData[thread_id].processorId;

    msr_write(cpu_id, MSR_MIC_SPFLT_CONTROL, 0x0ULL);
    msr_write(cpu_id, MSR_MIC_PERF_GLOBAL_CTRL, 0x0ULL);

    for ( int i=0; i<NUM_COUNTERS_PHI; i++ ) 
    {
        if (perfmon_threadData[thread_id].counters[i].init == TRUE) 
        {
            perfmon_threadData[thread_id].counters[i].counterData =
				msr_read(cpu_id, perfmon_threadData[thread_id].counters[i].counterRegister);
        }
    }

    flags = msr_read(cpu_id,MSR_MIC_PERF_GLOBAL_STATUS);
    printf ("Status: 0x%llX \n", LLU_CAST flags);

    if((flags & 0x3)) 
    {
        printf ("Overflow occured \n");
    }
}


void 
perfmon_readCountersThread_phi(int thread_id)
{
    uint64_t eax = 0ULL, edx = 0ULL;
    int cpu_id = perfmon_threadData[thread_id].processorId;

    for ( int i=0; i<NUM_COUNTERS_PHI; i++ ) 
    {
        if (perfmon_threadData[thread_id].counters[i].init == TRUE) 
        {
#if 0
#ifdef __MIC
            __asm__ volatile("movl %2, %%ecx \n\t"
                    "rdpmc\n\t"
                    "movl %%eax, %0\n\t"
                    "movl %%edx, %1\n\t"
                    : "=r" ((uint32_t) eax) , "=r" ((uint32_t) edx)
                    : "r" (i) 
                    : "%eax", "%ecx", "%edx");

                perfmon_threadData[thread_id].counters[i].counterData =
                    (edx<<32) + eax;
#endif
#endif

                perfmon_threadData[thread_id].counters[i].counterData =
                    msr_read(cpu_id, perfmon_threadData[thread_id].counters[i].counterRegister);
        }
    }
}


