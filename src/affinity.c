/*
 * =======================================================================================
 *
 *      Filename:  affinity.c
 *
 *      Description:  Implementation of affinity module.
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
#include <string.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sched.h>
#include <time.h>
#include <pthread.h>

#include <error.h>
#include <types.h>
#include <numa.h>
#include <affinity.h>
#include <cpuid.h>
#include <tree.h>

/* #####   EXPORTED VARIABLES   ########################################### */

int affinity_core2node_lookup[MAX_NUM_THREADS];

/* #####   MACROS  -  LOCAL TO THIS SOURCE FILE   ######################### */

#define gettid() syscall(SYS_gettid)


/* #####   VARIABLES  -  LOCAL TO THIS SOURCE FILE   ###################### */

static int  affinity_numberOfDomains = 0;
static AffinityDomain*  domains;

/* #####   FUNCTION DEFINITIONS  -  LOCAL TO THIS SOURCE FILE   ########### */

static int
getProcessorID(cpu_set_t* cpu_set)
{
    int processorId;

    for ( processorId = 0; processorId < MAX_NUM_THREADS; processorId++ )
    {
        if ( CPU_ISSET(processorId,cpu_set) )
        {
            break;
        }
    }
    return processorId;
}

/* this routine expects a core node and will traverse the tree over all threads
 * with physical cores first order */
static void
treeFillNextEntries(int numberOfEntries, int* processorIds, TreeNode** tree)
{
    int tmplist[numberOfEntries];
    int mapping[numberOfEntries];
    int counter = numberOfEntries;
    TreeNode* threadNode;
    int threadsPerCore = cpuid_topology.numThreadsPerCore;
    int row, column;
    int numberOfCores = numberOfEntries/threadsPerCore;

    while ( *tree != NULL )
    {
        if ( !counter ) break;

        threadNode = tree_getChildNode(*tree);

        while ( threadNode != NULL )
        {
            tmplist[numberOfEntries-counter] = threadNode->id;
            threadNode = tree_getNextNode(threadNode);
            counter--;
        }
        *tree = tree_getNextNode(*tree);
    }

    for ( int i=0; i < numberOfEntries; i++ )
    {
        column = i%threadsPerCore;
        row = i/threadsPerCore;
        mapping[column*numberOfCores+row]=i;
    }

    for ( int i=0; i < numberOfEntries; i++ )
    {
        processorIds[i] = tmplist[mapping[i]];
    }

}

static void
treeFillEntriesNode(int* processorIds)
{
    int tmplist[cpuid_topology.numHWThreads];
    int mapping[cpuid_topology.numHWThreads];
    int counter = cpuid_topology.numHWThreads;
    TreeNode* threadNode;
    TreeNode* coreNode;
    TreeNode* socketNode;
    int threadsPerCore = cpuid_topology.numThreadsPerCore;
    int row, column;
    int numberOfCores = cpuid_topology.numHWThreads/threadsPerCore;

    socketNode = tree_getChildNode(cpuid_topology.topologyTree);

    while ( socketNode != NULL )
    {
        coreNode = tree_getChildNode(socketNode);

        while ( coreNode != NULL )
        {
            threadNode = tree_getChildNode(coreNode);

            while ( threadNode != NULL )
            {
                tmplist[cpuid_topology.numHWThreads-counter] = threadNode->id;
                threadNode = tree_getNextNode(threadNode);
                counter--;
            }
            coreNode = tree_getNextNode(coreNode);
        }
        socketNode = tree_getNextNode(socketNode);
    }

    for ( uint32_t i=0; i < cpuid_topology.numHWThreads; i++ )
    {
        column = i%threadsPerCore;
        row = i/threadsPerCore;
        mapping[column*numberOfCores+row]=i;
    }

    for ( uint32_t i=0; i < cpuid_topology.numHWThreads; i++ )
    {
        processorIds[i] = tmplist[mapping[i]];
    }
}


/* #####   FUNCTION DEFINITIONS  -  EXPORTED FUNCTIONS   ################## */

void
affinity_init()
{
    int  numberOfDomains = 1;
    int currentDomain;
    int numberOfCacheDomains;
    int numberOfNumaDomains;
    TreeNode* socketNode;
    TreeNode* coreNode;

    int numberOfCoresPerCache =
        cpuid_topology.cacheLevels[cpuid_topology.numCacheLevels-1].threads/
        cpuid_topology.numThreadsPerCore;

    int numberOfProcessorsPerCache =
        cpuid_topology.cacheLevels[cpuid_topology.numCacheLevels-1].threads;

    /* determine total number of domains */
    numberOfDomains += cpuid_topology.numSockets;

    /* for the cache domain take only into account last level cache and assume
     * all sockets to be uniform. */

    /* determine how many last level shared caches exist per socket */
    numberOfCacheDomains = cpuid_topology.numSockets *
        (cpuid_topology.numCoresPerSocket/numberOfCoresPerCache);

    numberOfDomains += numberOfCacheDomains;

    /* determine how many numa domains exist */
    numberOfNumaDomains = numa_info.numberOfNodes;
    numberOfDomains += numberOfNumaDomains;

    domains = (AffinityDomain*) malloc(numberOfDomains * sizeof(AffinityDomain));
    currentDomain = 0;

    /* Node domain */
    domains[0].numberOfProcessors = cpuid_topology.numHWThreads;
    domains[0].processorList = (int*) malloc(cpuid_topology.numHWThreads*sizeof(int));
    domains[0].tag = bformat("N");

    for ( int i=1; i < numberOfDomains; i++ )
    {
        if ( i < (int) (cpuid_topology.numSockets+1) )
        {
            domains[i].numberOfProcessors = cpuid_topology.numCoresPerSocket *
                cpuid_topology.numThreadsPerCore;

            domains[i].processorList = (int*) malloc(cpuid_topology.numCoresPerSocket*
                    cpuid_topology.numThreadsPerCore * sizeof(int));

            domains[i].tag = bformat("S%d",i-1);
        }
        else if ( i < (int) ((cpuid_topology.numSockets + numberOfCacheDomains)+1) )
        {
            domains[i].processorList = (int*)
                malloc(numberOfProcessorsPerCache*sizeof(int));

            domains[i].tag = bformat("C%d",currentDomain++);
            domains[i].numberOfProcessors = numberOfProcessorsPerCache;

        }
        else 
        {
            domains[i].processorList = (int*)
                malloc(numa_info.nodes[currentDomain].numberOfProcessors * sizeof(int));

            domains[i].numberOfProcessors = numa_info.nodes[currentDomain].numberOfProcessors; 

            for ( int j=0; j < numa_info.nodes[currentDomain].numberOfProcessors; j++ )
            {
                domains[i].processorList[j] = (int) numa_info.nodes[currentDomain].processors[j];
            }

            domains[i].tag = bformat("M%d",currentDomain++);
        }

        /* reset domain counter */
        if ( i == (int) ((cpuid_topology.numSockets + numberOfCacheDomains)) )
        {
            currentDomain = 0;
        }
    }

    treeFillEntriesNode(domains[0].processorList);
    currentDomain = 0;

    /* create socket domains */
    socketNode = tree_getChildNode(cpuid_topology.topologyTree);

    while ( socketNode != NULL )
    {
        currentDomain++;
        coreNode = tree_getChildNode(socketNode);
        treeFillNextEntries(domains[currentDomain].numberOfProcessors,
                domains[currentDomain].processorList, &coreNode);

        socketNode = tree_getNextNode(socketNode);
    }

    /* create last level cache domains */
    socketNode = tree_getChildNode(cpuid_topology.topologyTree);

    while ( socketNode != NULL )
    {
        coreNode = tree_getChildNode(socketNode);

        for ( int i=0; i < (int) (cpuid_topology.numCoresPerSocket/numberOfCoresPerCache); i++ )
        {
            currentDomain++;
            treeFillNextEntries(domains[currentDomain].numberOfProcessors,
                    domains[currentDomain].processorList, &coreNode);
        }

        socketNode = tree_getNextNode(socketNode);
    }

    /* FIXME Quick and dirty hack to get physical cores first on Intel Numa domains */
    if ( (cpuid_info.family == P6_FAMILY) && (cpuid_topology.numThreadsPerCore > 1))
    {
        for ( int socketId=0; socketId < cpuid_topology.numSockets; socketId++)
        {
            bstring tmpDomain = bformat("M%d",socketId);

            for ( int i=0; i < numberOfDomains; i++ )
            {
                if ( biseq(tmpDomain, domains[i].tag) )
                {
                    for (int procId=0; procId < domains[socketId+1].numberOfProcessors; procId++)
                    {
                        domains[i].processorList[procId] = domains[socketId+1].processorList[procId];
                    }
                }
            }

            bdestroy(tmpDomain);
        }
    }

    /* This is redundant ;-). Create thread to node lookup */
    for ( uint32_t i = 0; i < numa_info.numberOfNodes; i++ )
    {
        for ( int j = 0; j < numa_info.nodes[i].numberOfProcessors; j++ )
        {
            affinity_core2node_lookup[numa_info.nodes[i].processors[j]] = i;
        }
    }

    affinity_numberOfDomains = numberOfDomains;
}


void
affinity_finalize()
{
    for ( int i=0; i < affinity_numberOfDomains; i++ )
    {
        free(domains[i].processorList);
    }
    free(domains);
}


int
affinity_processGetProcessorId()
{
    int ret;
    cpu_set_t cpu_set;
    CPU_ZERO(&cpu_set);
    ret = sched_getaffinity(getpid(),sizeof(cpu_set_t), &cpu_set);

    if (ret < 0)
    {
        ERROR;
    }

    return getProcessorID(&cpu_set);
}


int
affinity_threadGetProcessorId()
{
    cpu_set_t  cpu_set;
    CPU_ZERO(&cpu_set);
    sched_getaffinity(gettid(),sizeof(cpu_set_t), &cpu_set);

    return getProcessorID(&cpu_set);
}

#ifdef HAS_SCHEDAFFINITY
void
affinity_pinThread(int processorId)
{
	cpu_set_t cpuset;
    pthread_t thread;

    thread = pthread_self();
    CPU_ZERO(&cpuset);
    CPU_SET(processorId, &cpuset);
    pthread_setaffinity_np(thread, sizeof(cpu_set_t), &cpuset);
}
#else
void
affinity_pinThread(int processorId)
{
}
#endif


void
affinity_pinProcess(int processorId)
{
	cpu_set_t cpuset;

	CPU_ZERO(&cpuset);
	CPU_SET(processorId, &cpuset);
	sched_setaffinity(0, sizeof(cpu_set_t), &cpuset);
}


const AffinityDomain*
affinity_getDomain(bstring domain)
{

    for ( int i=0; i < affinity_numberOfDomains; i++ )
    {
        if ( biseq(domain, domains[i].tag) )
        {
            return domains+i;
        }
    }

    return NULL;
}

void
affinity_printDomains()
{
    for ( int i=0; i < affinity_numberOfDomains; i++ )
    {
        printf("Domain %d:\n",i);
        printf("\tTag %s:",bdata(domains[i].tag));

        for ( uint32_t j=0; j < domains[i].numberOfProcessors; j++ )
        {
            printf(" %d",domains[i].processorList[j]);
        }
        printf("\n");
    }
}

