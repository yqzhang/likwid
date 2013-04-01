/*
 * =======================================================================================
 *
 *      Filename:  libperfctr.c
 *
 *      Description:  Marker API interface of module perfmon
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
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sched.h>
#include <pthread.h>

#include <error.h>
#include <types.h>
#include <bstrlib.h>
#include <cpuid.h>
#include <numa.h>
#include <affinity.h>
#include <lock.h>
#include <tree.h>
#include <accessClient.h>
#include <msr.h>
#include <pci.h>
#include <power.h>
#include <timer.h>
#include <hashTable.h>
#include <registers.h>
#include <likwid.h>

/* #####   VARIABLES  -  LOCAL TO THIS SOURCE FILE   ###################### */

static int socket_lock[MAX_NUM_NODES];
static int thread_socketFD[MAX_NUM_THREADS];

/* #####   MACROS  -  LOCAL TO THIS SOURCE FILE   ######################### */

#define gettid() syscall(SYS_gettid)

/* #####   FUNCTION DEFINITIONS  -  LOCAL TO THIS SOURCE FILE   ########### */

static int getProcessorID(cpu_set_t* cpu_set)
{
    int processorId;

    for (processorId=0;processorId<MAX_NUM_THREADS;processorId++){
        if (CPU_ISSET(processorId,cpu_set))
        {  
            break;
        }
    }
    return processorId;
}

/* #####   FUNCTION DEFINITIONS  -  EXPORTED FUNCTIONS   ################## */

void
likwid_markerInit(void)
{
    char* modeStr = getenv("LIKWID_MODE");
    int cpuId = likwid_getProcessorId();
    cpuid_init();
    numa_init();
    affinity_init();
    timer_init();
    hashTable_init();

    for(int i=0; i<MAX_NUM_THREADS; i++) thread_socketFD[i] = -1;
    for(int i=0; i<MAX_NUM_NODES; i++) socket_lock[i] = LOCK_INIT;

    if ( modeStr != NULL )
    {
        accessClient_mode = atoi(modeStr);
    }

    if (accessClient_mode != DAEMON_AM_DIRECT)
    {
        accessClient_init(&thread_socketFD[cpuId]);
    }

    /* FIXME: Is pci_init missing? */
    msr_init(thread_socketFD[cpuId]);

    switch ( cpuid_info.model ) 
    {
        case IVYBRIDGE:
        case SANDYBRIDGE:
        case SANDYBRIDGE_EP:
            power_init(0); /* FIXME: Let power Module choose cpuID itself here */
            break;

        case NEHALEM_WESTMERE_M:
        case NEHALEM_WESTMERE:
        case NEHALEM_BLOOMFIELD:
        case NEHALEM_LYNNFIELD:
            break;

        default:
            break;

    }
}

void
likwid_markerThreadInit(void)
{
    int cpuId = likwid_getProcessorId();

    if (accessClient_mode != DAEMON_AM_DIRECT)
    {
        if (thread_socketFD[cpuId] == -1)
        {
            accessClient_init(&thread_socketFD[cpuId]);
        }
    }
}

/* File format 
 * 1 numberOfThreads numberOfRegions
 * 2 regionID:regionTag0
 * 3 regionID:regionTag1
 * 4 regionID threadID countersvalues(space sparated)
 * 5 regionID threadID countersvalues
 */
void
likwid_markerClose(void)
{
    FILE *file = NULL;
    LikwidResults* results = NULL;
    int numberOfThreads;
    int numberOfRegions;

    hashTable_finalize(&numberOfThreads, &numberOfRegions, &results);

    file = fopen(getenv("LIKWID_FILEPATH"),"w");

    if (file != NULL)
    {
        fprintf(file,"%d %d\n",numberOfThreads,numberOfRegions);

        for (int i=0; i<numberOfRegions; i++)
        {
            fprintf(file,"%d:%s\n",i,bdata(results[i].tag));
        }

        for (int i=0; i<numberOfRegions; i++)
        {
            for (int j=0; j<numberOfThreads; j++)
            {
                fprintf(file,"%d ",i);
                fprintf(file,"%d ",j);
                fprintf(file,"%u ",results[i].count[j]);
                fprintf(file,"%e ",results[i].time[j]);

                for (int k=0; k<NUM_PMC; k++)
                {
                    fprintf(file,"%e ",results[i].counters[j][k]);
                }
                fprintf(file,"\n");
            }
        }
        fclose(file);
    }

    for (int i=0;i<numberOfRegions; i++)
    {
        for (int j=0;j<numberOfThreads; j++)
        {
            free(results[i].counters[j]);
        }
        free(results[i].time);
        bdestroy(results[i].tag);
        free(results[i].count);
        free(results[i].counters);
    }

    if (results != NULL)
    {
        free(results);
    }

    msr_finalize();
    pci_finalize();

    for (int i=0; i<MAX_NUM_THREADS; i++)
    {
        accessClient_finalize(thread_socketFD[i]);
        thread_socketFD[i] = -1;
    }
}

#define READ_MEM_CHANNEL(channel, reg, cid) \
    counter_result = pci_tread(socket_fd, cpu_id, channel, reg##_A); \
counter_result = (counter_result<<32) +   \
pci_tread(socket_fd, cpu_id, channel, reg##_B);   \
results->StartPMcounters[cid] = (double) counter_result


void
likwid_markerStartRegion(const char* regionTag)
{
    bstring tag = bfromcstralloc(100, regionTag);
    LikwidThreadResults* results;
    int cpu_id = hashTable_get(tag, &results);
    bdestroy(tag);
    int socket_fd = thread_socketFD[cpu_id];

    if (accessClient_mode != DAEMON_AM_DIRECT)
    {
        if (socket_fd == -1)
        {
            printf("ERROR: Invalid socket file handle on processor %d. Did you call likwid_markerThreadInit() ?\n", cpu_id);
        }
    }

    results->count++;

    switch ( cpuid_info.family ) 
    {
        case P6_FAMILY:

            switch ( cpuid_info.model ) 
            {
                case PENTIUM_M_BANIAS:
                    break;

                case PENTIUM_M_DOTHAN:
                    break;

                case CORE_DUO:
                    break;

                case XEON_MP:

                case CORE2_65:

                case CORE2_45:

                    results->StartPMcounters[0] = (double) msr_tread(socket_fd, cpu_id, MSR_PERF_FIXED_CTR0);
                    results->StartPMcounters[1] = (double) msr_tread(socket_fd, cpu_id, MSR_PERF_FIXED_CTR1);
                    results->StartPMcounters[2] = (double) msr_tread(socket_fd, cpu_id, MSR_PMC0);
                    results->StartPMcounters[3] = (double) msr_tread(socket_fd, cpu_id, MSR_PMC1);
                    break;

                case NEHALEM_WESTMERE_M:

                case NEHALEM_WESTMERE:

                case NEHALEM_BLOOMFIELD:

                case NEHALEM_LYNNFIELD:

                    results->StartPMcounters[0] = (double) msr_tread(socket_fd, cpu_id, MSR_PERF_FIXED_CTR0);
                    results->StartPMcounters[1] = (double) msr_tread(socket_fd, cpu_id, MSR_PERF_FIXED_CTR1);
                    results->StartPMcounters[2] = (double) msr_tread(socket_fd, cpu_id, MSR_PERF_FIXED_CTR2);
                    results->StartPMcounters[3] = (double) msr_tread(socket_fd, cpu_id, MSR_PMC0);
                    results->StartPMcounters[4] = (double) msr_tread(socket_fd, cpu_id, MSR_PMC1);
                    results->StartPMcounters[5] = (double) msr_tread(socket_fd, cpu_id, MSR_PMC2);
                    results->StartPMcounters[6] = (double) msr_tread(socket_fd, cpu_id, MSR_PMC3);

                    if ((socket_lock[affinity_core2node_lookup[cpu_id]] == cpu_id) ||
                            lock_acquire((int*) &socket_lock[affinity_core2node_lookup[cpu_id]], cpu_id))
                    {
                        results->StartPMcounters[7] = (double) msr_tread(socket_fd, cpu_id, MSR_UNCORE_PMC0);
                        results->StartPMcounters[8] = (double) msr_tread(socket_fd, cpu_id, MSR_UNCORE_PMC1);
                        results->StartPMcounters[9] = (double) msr_tread(socket_fd, cpu_id, MSR_UNCORE_PMC2);
                        results->StartPMcounters[10] = (double) msr_tread(socket_fd, cpu_id, MSR_UNCORE_PMC3);
                        results->StartPMcounters[11] = (double) msr_tread(socket_fd, cpu_id, MSR_UNCORE_PMC4);
                        results->StartPMcounters[12] = (double) msr_tread(socket_fd, cpu_id, MSR_UNCORE_PMC5);
                        results->StartPMcounters[13] = (double) msr_tread(socket_fd, cpu_id, MSR_UNCORE_PMC6);
                        results->StartPMcounters[14] = (double) msr_tread(socket_fd, cpu_id, MSR_UNCORE_PMC7);
                    }
                    break;

                case SANDYBRIDGE:
                case SANDYBRIDGE_EP:

                    results->StartPMcounters[0] = (double) msr_tread(socket_fd, cpu_id, MSR_PERF_FIXED_CTR0);
                    results->StartPMcounters[1] = (double) msr_tread(socket_fd, cpu_id, MSR_PERF_FIXED_CTR1);
                    results->StartPMcounters[2] = (double) msr_tread(socket_fd, cpu_id, MSR_PERF_FIXED_CTR2);
                    results->StartPMcounters[3] = (double) msr_tread(socket_fd, cpu_id, MSR_PMC0);
                    results->StartPMcounters[4] = (double) msr_tread(socket_fd, cpu_id, MSR_PMC1);
                    results->StartPMcounters[5] = (double) msr_tread(socket_fd, cpu_id, MSR_PMC2);
                    results->StartPMcounters[6] = (double) msr_tread(socket_fd, cpu_id, MSR_PMC3);

                    if ((socket_lock[affinity_core2node_lookup[cpu_id]] == cpu_id) ||
                            lock_acquire((int*) &socket_lock[affinity_core2node_lookup[cpu_id]], cpu_id))
                    {
                        results->StartPMcounters[7] = (double) power_tread(socket_fd, cpu_id, MSR_PKG_ENERGY_STATUS);
                        results->StartPMcounters[10] = (double) power_tread(socket_fd, cpu_id, MSR_DRAM_ENERGY_STATUS);

#ifdef SNB_UNCORE
                        if (accessClient_mode != DAEMON_AM_DIRECT)
                        {
                            uint64_t counter_result;
                            /* 4 counters per channel, 4 channels, 2 reads per counter to get result */
                            /* channel 0 */
                            READ_MEM_CHANNEL(PCI_IMC_DEVICE_CH_0, PCI_UNC_MC_PMON_CTR_0, 11);
                            READ_MEM_CHANNEL(PCI_IMC_DEVICE_CH_0, PCI_UNC_MC_PMON_CTR_1, 12);
                            READ_MEM_CHANNEL(PCI_IMC_DEVICE_CH_0, PCI_UNC_MC_PMON_CTR_2, 13);
                            READ_MEM_CHANNEL(PCI_IMC_DEVICE_CH_0, PCI_UNC_MC_PMON_CTR_3, 14);

                            /* channel 1 */
                            READ_MEM_CHANNEL(PCI_IMC_DEVICE_CH_1, PCI_UNC_MC_PMON_CTR_0, 15);
                            READ_MEM_CHANNEL(PCI_IMC_DEVICE_CH_1, PCI_UNC_MC_PMON_CTR_1, 16);
                            READ_MEM_CHANNEL(PCI_IMC_DEVICE_CH_1, PCI_UNC_MC_PMON_CTR_2, 17);
                            READ_MEM_CHANNEL(PCI_IMC_DEVICE_CH_1, PCI_UNC_MC_PMON_CTR_3, 18);

                            /* channel 2 */
                            READ_MEM_CHANNEL(PCI_IMC_DEVICE_CH_2, PCI_UNC_MC_PMON_CTR_0, 19);
                            READ_MEM_CHANNEL(PCI_IMC_DEVICE_CH_2, PCI_UNC_MC_PMON_CTR_1, 20);
                            READ_MEM_CHANNEL(PCI_IMC_DEVICE_CH_2, PCI_UNC_MC_PMON_CTR_2, 21);
                            READ_MEM_CHANNEL(PCI_IMC_DEVICE_CH_2, PCI_UNC_MC_PMON_CTR_3, 22);

                            /* channel 3 */
                            READ_MEM_CHANNEL(PCI_IMC_DEVICE_CH_3, PCI_UNC_MC_PMON_CTR_0, 23);
                            READ_MEM_CHANNEL(PCI_IMC_DEVICE_CH_3, PCI_UNC_MC_PMON_CTR_1, 24);
                            READ_MEM_CHANNEL(PCI_IMC_DEVICE_CH_3, PCI_UNC_MC_PMON_CTR_2, 25);
                            READ_MEM_CHANNEL(PCI_IMC_DEVICE_CH_3, PCI_UNC_MC_PMON_CTR_3, 26);
                        }
#endif
                    }

                    break;

                case NEHALEM_EX:

                case WESTMERE_EX:

                    results->StartPMcounters[0] = (double) msr_tread(socket_fd, cpu_id, MSR_PERF_FIXED_CTR0);
                    results->StartPMcounters[1] = (double) msr_tread(socket_fd, cpu_id, MSR_PERF_FIXED_CTR1);
                    results->StartPMcounters[2] = (double) msr_tread(socket_fd, cpu_id, MSR_PERF_FIXED_CTR2);
                    results->StartPMcounters[3] = (double) msr_tread(socket_fd, cpu_id, MSR_PMC0);
                    results->StartPMcounters[4] = (double) msr_tread(socket_fd, cpu_id, MSR_PMC1);
                    results->StartPMcounters[5] = (double) msr_tread(socket_fd, cpu_id, MSR_PMC2);
                    results->StartPMcounters[6] = (double) msr_tread(socket_fd, cpu_id, MSR_PMC3);


                    if ((socket_lock[affinity_core2node_lookup[cpu_id]] == cpu_id) ||
                            lock_acquire((int*) &socket_lock[affinity_core2node_lookup[cpu_id]], cpu_id))
                    {
                        results->StartPMcounters[7] = (double) msr_tread(socket_fd, cpu_id, MSR_M0_PMON_CTR0);
                        results->StartPMcounters[8] = (double) msr_tread(socket_fd, cpu_id, MSR_M0_PMON_CTR1);
                        results->StartPMcounters[13] = (double) msr_tread(socket_fd, cpu_id, MSR_M1_PMON_CTR0);
                        results->StartPMcounters[14] = (double) msr_tread(socket_fd, cpu_id, MSR_M1_PMON_CTR1);
                        results->StartPMcounters[20] = (double) msr_tread(socket_fd, cpu_id, MSR_B0_PMON_CTR1);
                        results->StartPMcounters[24] = (double) msr_tread(socket_fd, cpu_id, MSR_B1_PMON_CTR1);
                        results->StartPMcounters[27] = (double) msr_tread(socket_fd, cpu_id, MSR_R0_PMON_CTR0);
                        results->StartPMcounters[28] = (double) msr_tread(socket_fd, cpu_id, MSR_R0_PMON_CTR1);
                        results->StartPMcounters[35] = (double) msr_tread(socket_fd, cpu_id, MSR_R1_PMON_CTR8);
                        results->StartPMcounters[36] = (double) msr_tread(socket_fd, cpu_id, MSR_R1_PMON_CTR9);
                    }

                    break;

                default:
                    ERROR_PLAIN_PRINT(Unsupported Processor);
                    break;
            }
            break;

        case MIC_FAMILY:

            results->StartPMcounters[0] = (double) msr_tread(socket_fd, cpu_id, MSR_MIC_PMC0);
            results->StartPMcounters[1] = (double) msr_tread(socket_fd, cpu_id, MSR_MIC_PMC1);
            break;

        case K8_FAMILY:

        case K10_FAMILY:

            results->StartPMcounters[0] = (double) msr_tread(socket_fd, cpu_id, MSR_AMD_PMC0);
            results->StartPMcounters[1] = (double) msr_tread(socket_fd, cpu_id, MSR_AMD_PMC1);
            results->StartPMcounters[2] = (double) msr_tread(socket_fd, cpu_id, MSR_AMD_PMC2);
            results->StartPMcounters[3] = (double) msr_tread(socket_fd, cpu_id, MSR_AMD_PMC3);
            break;

        case K15_FAMILY:

            results->StartPMcounters[0] = (double) msr_tread(socket_fd, cpu_id, MSR_AMD15_PMC0);
            results->StartPMcounters[1] = (double) msr_tread(socket_fd, cpu_id, MSR_AMD15_PMC1);
            results->StartPMcounters[2] = (double) msr_tread(socket_fd, cpu_id, MSR_AMD15_PMC2);
            results->StartPMcounters[3] = (double) msr_tread(socket_fd, cpu_id, MSR_AMD15_PMC3);
            results->StartPMcounters[4] = (double) msr_tread(socket_fd, cpu_id, MSR_AMD15_PMC4);
            results->StartPMcounters[5] = (double) msr_tread(socket_fd, cpu_id, MSR_AMD15_PMC5);

            if ((socket_lock[affinity_core2node_lookup[cpu_id]] == cpu_id) ||
                    lock_acquire((int*) &socket_lock[affinity_core2node_lookup[cpu_id]], cpu_id))
            {
                results->StartPMcounters[6] = (double) msr_tread(socket_fd, cpu_id, MSR_AMD15_NB_PMC0);
                results->StartPMcounters[7] = (double) msr_tread(socket_fd, cpu_id, MSR_AMD15_NB_PMC1);
                results->StartPMcounters[8] = (double) msr_tread(socket_fd, cpu_id, MSR_AMD15_NB_PMC2);
                results->StartPMcounters[9] = (double) msr_tread(socket_fd, cpu_id, MSR_AMD15_NB_PMC3);
            }
            break;

        default:
            ERROR_PLAIN_PRINT(Unsupported Processor);
            break;
    }

    timer_startCycles(&(results->startTime));
}

#define READ_END_MEM_CHANNEL(channel, reg, cid) \
    counter_result = pci_tread(socket_fd, cpu_id, channel, reg##_A); \
counter_result = (counter_result<<32) +   \
pci_tread(socket_fd, cpu_id, channel, reg##_B);   \
results->PMcounters[cid] += (double) counter_result - results->StartPMcounters[cid]



void
likwid_markerStopRegion(const char* regionTag)
{
    bstring tag = bfromcstralloc(100, regionTag);
    LikwidThreadResults* results;

    int cpu_id = hashTable_get(tag, &results);
    timer_stopCycles(&(results->startTime));
    results->time += timer_printCyclesTime(&(results->startTime));
    int socket_fd = thread_socketFD[cpu_id];
    bdestroy(tag);

    switch ( cpuid_info.family ) 
    {
        case P6_FAMILY:

            switch ( cpuid_info.model ) 
            {
                case PENTIUM_M_BANIAS:
                    break;

                case PENTIUM_M_DOTHAN:
                    break;

                case CORE_DUO:
                    break;

                case XEON_MP:

                case CORE2_65:

                case CORE2_45:

                    results->PMcounters[0] += ((double) msr_tread(socket_fd, cpu_id, MSR_PERF_FIXED_CTR0) - results->StartPMcounters[0]);
                    results->PMcounters[1] += ((double) msr_tread(socket_fd, cpu_id, MSR_PERF_FIXED_CTR1) - results->StartPMcounters[1] );
                    results->PMcounters[2] += ((double) msr_tread(socket_fd, cpu_id, MSR_PMC0) - results->StartPMcounters[2]);
                    results->PMcounters[3] += ((double) msr_tread(socket_fd, cpu_id, MSR_PMC1) - results->StartPMcounters[3]);
                    break;

                case NEHALEM_WESTMERE_M:

                case NEHALEM_WESTMERE:

                case NEHALEM_BLOOMFIELD:

                case NEHALEM_LYNNFIELD:

                    results->PMcounters[0] += (double) msr_tread(socket_fd, cpu_id, MSR_PERF_FIXED_CTR0) - results->StartPMcounters[0] ;
                    results->PMcounters[1] += (double) msr_tread(socket_fd, cpu_id, MSR_PERF_FIXED_CTR1) - results->StartPMcounters[1] ;
                    results->PMcounters[2] += (double) msr_tread(socket_fd, cpu_id, MSR_PERF_FIXED_CTR2) - results->StartPMcounters[2] ;
                    results->PMcounters[3] += (double) msr_tread(socket_fd, cpu_id, MSR_PMC0) - results->StartPMcounters[3] ;
                    results->PMcounters[4] += (double) msr_tread(socket_fd, cpu_id, MSR_PMC1) - results->StartPMcounters[4] ;
                    results->PMcounters[5] += (double) msr_tread(socket_fd, cpu_id, MSR_PMC2) - results->StartPMcounters[5] ;
                    results->PMcounters[6] += (double) msr_tread(socket_fd, cpu_id, MSR_PMC3) - results->StartPMcounters[6] ;

                    if ((socket_lock[affinity_core2node_lookup[cpu_id]] == cpu_id))
                    {
                        results->PMcounters[7]  += (double) msr_tread(socket_fd, cpu_id, MSR_UNCORE_PMC0) - results->StartPMcounters[7];
                        results->PMcounters[8]  += (double) msr_tread(socket_fd, cpu_id, MSR_UNCORE_PMC1) - results->StartPMcounters[8];
                        results->PMcounters[9]  += (double) msr_tread(socket_fd, cpu_id, MSR_UNCORE_PMC2) - results->StartPMcounters[9];
                        results->PMcounters[10] += (double) msr_tread(socket_fd, cpu_id, MSR_UNCORE_PMC3) - results->StartPMcounters[10];
                        results->PMcounters[11] += (double) msr_tread(socket_fd, cpu_id, MSR_UNCORE_PMC4) - results->StartPMcounters[11];
                        results->PMcounters[12] += (double) msr_tread(socket_fd, cpu_id, MSR_UNCORE_PMC5) - results->StartPMcounters[12];
                        results->PMcounters[13] += (double) msr_tread(socket_fd, cpu_id, MSR_UNCORE_PMC6) - results->StartPMcounters[13];
                        results->PMcounters[14] += (double) msr_tread(socket_fd, cpu_id, MSR_UNCORE_PMC7) - results->StartPMcounters[14];
                    }
                    break;

                case SANDYBRIDGE:

                case SANDYBRIDGE_EP:

                    results->PMcounters[0] += (double) msr_tread(socket_fd, cpu_id, MSR_PERF_FIXED_CTR0) - results->StartPMcounters[0];
                    results->PMcounters[1] += (double) msr_tread(socket_fd, cpu_id, MSR_PERF_FIXED_CTR1) - results->StartPMcounters[1];
                    results->PMcounters[2] += (double) msr_tread(socket_fd, cpu_id, MSR_PERF_FIXED_CTR2) - results->StartPMcounters[2];
                    results->PMcounters[3] += (double) msr_tread(socket_fd, cpu_id, MSR_PMC0) - results->StartPMcounters[3];
                    results->PMcounters[4] += (double) msr_tread(socket_fd, cpu_id, MSR_PMC1) - results->StartPMcounters[4];
                    results->PMcounters[5] += (double) msr_tread(socket_fd, cpu_id, MSR_PMC2) - results->StartPMcounters[5];
                    results->PMcounters[6] += (double) msr_tread(socket_fd, cpu_id, MSR_PMC3) - results->StartPMcounters[6];

                    if ((socket_lock[affinity_core2node_lookup[cpu_id]] == cpu_id))
                    {
                        results->PMcounters[7] += power_info.energyUnit *
                            ( power_tread(socket_fd, cpu_id, MSR_PKG_ENERGY_STATUS) - results->StartPMcounters[7]);
                        results->PMcounters[10] += power_info.energyUnit *
                            ( power_tread(socket_fd, cpu_id, MSR_DRAM_ENERGY_STATUS) - results->StartPMcounters[10]);

#ifdef SNB_UNCORE
                        if (accessClient_mode != DAEMON_AM_DIRECT)
                        {
                            uint64_t counter_result;

                            /* channel 0 */
                            READ_END_MEM_CHANNEL(PCI_IMC_DEVICE_CH_0, PCI_UNC_MC_PMON_CTR_0, 11);
                            READ_END_MEM_CHANNEL(PCI_IMC_DEVICE_CH_0, PCI_UNC_MC_PMON_CTR_1, 12);
                            READ_END_MEM_CHANNEL(PCI_IMC_DEVICE_CH_0, PCI_UNC_MC_PMON_CTR_2, 13);
                            READ_END_MEM_CHANNEL(PCI_IMC_DEVICE_CH_0, PCI_UNC_MC_PMON_CTR_3, 14);

                            /* channel 1 */
                            READ_END_MEM_CHANNEL(PCI_IMC_DEVICE_CH_1, PCI_UNC_MC_PMON_CTR_0, 15);
                            READ_END_MEM_CHANNEL(PCI_IMC_DEVICE_CH_1, PCI_UNC_MC_PMON_CTR_1, 16);
                            READ_END_MEM_CHANNEL(PCI_IMC_DEVICE_CH_1, PCI_UNC_MC_PMON_CTR_2, 17);
                            READ_END_MEM_CHANNEL(PCI_IMC_DEVICE_CH_1, PCI_UNC_MC_PMON_CTR_3, 18);

                            /* channel 2 */
                            READ_END_MEM_CHANNEL(PCI_IMC_DEVICE_CH_2, PCI_UNC_MC_PMON_CTR_0, 19);
                            READ_END_MEM_CHANNEL(PCI_IMC_DEVICE_CH_2, PCI_UNC_MC_PMON_CTR_1, 20);
                            READ_END_MEM_CHANNEL(PCI_IMC_DEVICE_CH_2, PCI_UNC_MC_PMON_CTR_2, 21);
                            READ_END_MEM_CHANNEL(PCI_IMC_DEVICE_CH_2, PCI_UNC_MC_PMON_CTR_3, 22);

                            /* channel 3 */
                            READ_END_MEM_CHANNEL(PCI_IMC_DEVICE_CH_3, PCI_UNC_MC_PMON_CTR_0, 23);
                            READ_END_MEM_CHANNEL(PCI_IMC_DEVICE_CH_3, PCI_UNC_MC_PMON_CTR_1, 24);
                            READ_END_MEM_CHANNEL(PCI_IMC_DEVICE_CH_3, PCI_UNC_MC_PMON_CTR_2, 25);
                            READ_END_MEM_CHANNEL(PCI_IMC_DEVICE_CH_3, PCI_UNC_MC_PMON_CTR_3, 26);
                        }
#endif
                    }

                    break;

                case IVYBRIDGE:

                    results->PMcounters[0] += (double) msr_tread(socket_fd, cpu_id, MSR_PERF_FIXED_CTR0) - results->StartPMcounters[0];
                    results->PMcounters[1] += (double) msr_tread(socket_fd, cpu_id, MSR_PERF_FIXED_CTR1) - results->StartPMcounters[1];
                    results->PMcounters[2] += (double) msr_tread(socket_fd, cpu_id, MSR_PERF_FIXED_CTR2) - results->StartPMcounters[2];
                    results->PMcounters[3] += (double) msr_tread(socket_fd, cpu_id, MSR_PMC0) - results->StartPMcounters[3];
                    results->PMcounters[4] += (double) msr_tread(socket_fd, cpu_id, MSR_PMC1) - results->StartPMcounters[4];
                    results->PMcounters[5] += (double) msr_tread(socket_fd, cpu_id, MSR_PMC2) - results->StartPMcounters[5];
                    results->PMcounters[6] += (double) msr_tread(socket_fd, cpu_id, MSR_PMC3) - results->StartPMcounters[6];

                    if ((socket_lock[affinity_core2node_lookup[cpu_id]] == cpu_id))
                    {
                        results->PMcounters[7] += power_info.energyUnit *
                            ( power_tread(socket_fd, cpu_id, MSR_PKG_ENERGY_STATUS) - results->StartPMcounters[7]);
                        results->PMcounters[10] += power_info.energyUnit *
                            ( power_tread(socket_fd, cpu_id, MSR_DRAM_ENERGY_STATUS) - results->StartPMcounters[10]);
                    }

                    break;

                case NEHALEM_EX:

                case WESTMERE_EX:

                    results->PMcounters[0] += (double) msr_tread(socket_fd, cpu_id, MSR_PERF_FIXED_CTR0) - results->StartPMcounters[0];
                    results->PMcounters[1] += (double) msr_tread(socket_fd, cpu_id, MSR_PERF_FIXED_CTR1) - results->StartPMcounters[1];
                    results->PMcounters[2] += (double) msr_tread(socket_fd, cpu_id, MSR_PERF_FIXED_CTR2) - results->StartPMcounters[2];
                    results->PMcounters[3] += (double) msr_tread(socket_fd, cpu_id, MSR_PMC0) - results->StartPMcounters[3];
                    results->PMcounters[4] += (double) msr_tread(socket_fd, cpu_id, MSR_PMC1) - results->StartPMcounters[4];
                    results->PMcounters[5] += (double) msr_tread(socket_fd, cpu_id, MSR_PMC2) - results->StartPMcounters[5];
                    results->PMcounters[6] += (double) msr_tread(socket_fd, cpu_id, MSR_PMC3) - results->StartPMcounters[6];

                    if ((socket_lock[affinity_core2node_lookup[cpu_id]] == cpu_id))
                    {
                        results->PMcounters[7]  += (double) msr_tread(socket_fd, cpu_id, MSR_M0_PMON_CTR0) - results->StartPMcounters[7];
                        results->PMcounters[8]  += (double) msr_tread(socket_fd, cpu_id, MSR_M0_PMON_CTR1) - results->StartPMcounters[8];
                        results->PMcounters[13] += (double) msr_tread(socket_fd, cpu_id, MSR_M1_PMON_CTR0) - results->StartPMcounters[13];
                        results->PMcounters[14] += (double) msr_tread(socket_fd, cpu_id, MSR_M1_PMON_CTR1) - results->StartPMcounters[14];
                        results->PMcounters[20] += (double) msr_tread(socket_fd, cpu_id, MSR_B0_PMON_CTR1) - results->StartPMcounters[20];
                        results->PMcounters[24] += (double) msr_tread(socket_fd, cpu_id, MSR_B1_PMON_CTR1) - results->StartPMcounters[24];
                        results->PMcounters[27] += (double) msr_tread(socket_fd, cpu_id, MSR_R0_PMON_CTR0) - results->StartPMcounters[27];
                        results->PMcounters[28] += (double) msr_tread(socket_fd, cpu_id, MSR_R0_PMON_CTR1) - results->StartPMcounters[28];
                        results->PMcounters[35] += (double) msr_tread(socket_fd, cpu_id, MSR_R1_PMON_CTR8) - results->StartPMcounters[35];
                        results->PMcounters[36] += (double) msr_tread(socket_fd, cpu_id, MSR_R1_PMON_CTR9) - results->StartPMcounters[36];
                    }

                    break;

                default:
                    ERROR_PLAIN_PRINT(Unsupported Processor);
                    break;
            }
            break;

        case MIC_FAMILY:

            results->PMcounters[0] += (double) msr_tread(socket_fd, cpu_id, MSR_MIC_PMC0) - results->StartPMcounters[0] ;
            results->PMcounters[1] += (double) msr_tread(socket_fd, cpu_id, MSR_MIC_PMC1) - results->StartPMcounters[1] ;
            break;

        case K8_FAMILY:

        case K10_FAMILY:

            results->PMcounters[0] += (double) msr_tread(socket_fd, cpu_id, MSR_AMD_PMC0) - results->StartPMcounters[0] ;
            results->PMcounters[1] += (double) msr_tread(socket_fd, cpu_id, MSR_AMD_PMC1) - results->StartPMcounters[1] ;
            results->PMcounters[2] += (double) msr_tread(socket_fd, cpu_id, MSR_AMD_PMC2) - results->StartPMcounters[2] ;
            results->PMcounters[3] += (double) msr_tread(socket_fd, cpu_id, MSR_AMD_PMC3) - results->StartPMcounters[3] ;
            break;

        case K15_FAMILY:

            results->PMcounters[0] += (double) msr_tread(socket_fd, cpu_id, MSR_AMD15_PMC0) - results->StartPMcounters[0] ;
            results->PMcounters[1] += (double) msr_tread(socket_fd, cpu_id, MSR_AMD15_PMC1) - results->StartPMcounters[1] ;
            results->PMcounters[2] += (double) msr_tread(socket_fd, cpu_id, MSR_AMD15_PMC2) - results->StartPMcounters[2] ;
            results->PMcounters[3] += (double) msr_tread(socket_fd, cpu_id, MSR_AMD15_PMC3) - results->StartPMcounters[3] ;
            results->PMcounters[4] += (double) msr_tread(socket_fd, cpu_id, MSR_AMD15_PMC4) - results->StartPMcounters[4] ;
            results->PMcounters[5] += (double) msr_tread(socket_fd, cpu_id, MSR_AMD15_PMC5) - results->StartPMcounters[5] ;

            if ((socket_lock[affinity_core2node_lookup[cpu_id]] == cpu_id))
            {
                results->PMcounters[6] += (double) msr_tread(socket_fd, cpu_id, MSR_AMD15_NB_PMC0) - results->StartPMcounters[6] ;
                results->PMcounters[7] += (double) msr_tread(socket_fd, cpu_id, MSR_AMD15_NB_PMC1) - results->StartPMcounters[7] ;
                results->PMcounters[8] += (double) msr_tread(socket_fd, cpu_id, MSR_AMD15_NB_PMC2) - results->StartPMcounters[8] ;
                results->PMcounters[9] += (double) msr_tread(socket_fd, cpu_id, MSR_AMD15_NB_PMC3) - results->StartPMcounters[9] ;
            }
            break;

        default:
            ERROR_PLAIN_PRINT(Unsupported Processor);
            break;
    }

}

int  likwid_getProcessorId()
{
    cpu_set_t  cpu_set;
    CPU_ZERO(&cpu_set);
    sched_getaffinity(gettid(),sizeof(cpu_set_t), &cpu_set);

    return getProcessorID(&cpu_set);
}


/* deprecated */
#if 0
int  likwid_processGetProcessorId()
{
    cpu_set_t cpu_set;
    CPU_ZERO(&cpu_set);
    sched_getaffinity(getpid(),sizeof(cpu_set_t), &cpu_set);

    return getProcessorID(&cpu_set);
}


int  likwid_threadGetProcessorId()
{
    cpu_set_t  cpu_set;
    CPU_ZERO(&cpu_set);
    sched_getaffinity(gettid(),sizeof(cpu_set_t), &cpu_set);

    return getProcessorID(&cpu_set);
}
#endif


#ifdef HAS_SCHEDAFFINITY
int  likwid_pinThread(int processorId)
{
    int ret;
    cpu_set_t cpuset;
    pthread_t thread;

    thread = pthread_self();
    CPU_ZERO(&cpuset);
    CPU_SET(processorId, &cpuset);
    ret = pthread_setaffinity_np(thread, sizeof(cpu_set_t), &cpuset);

    if (ret != 0)
    {   
        ERROR;
        return FALSE;
    }   

    return TRUE;
}
#endif


int  likwid_pinProcess(int processorId)
{
    int ret;
    cpu_set_t cpuset;

    CPU_ZERO(&cpuset);
    CPU_SET(processorId, &cpuset);
    ret = sched_setaffinity(0, sizeof(cpu_set_t), &cpuset);

    if (ret < 0)
    {
        ERROR;
        return FALSE;
    }   

    return TRUE;
}


