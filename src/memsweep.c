/*
 * =======================================================================================
 *
 *      Filename:  memsweep.c
 *
 *      Description:  Implementation of sweeper module.
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
#include <sys/mman.h>

#include <error.h>
#include <types.h>
#include <memsweep.h>
#include <numa.h>
#include <affinity.h>

/* #####   EXPORTED VARIABLES   ########################################### */


/* #####   MACROS  -  LOCAL TO THIS SOURCE FILE   ######################### */


/* #####   VARIABLES  -  LOCAL TO THIS SOURCE FILE   ###################### */

static uint64_t  memoryFraction = 80ULL;


/* #####   FUNCTION DEFINITIONS  -  LOCAL TO THIS SOURCE FILE   ########### */

static void* 
allocateOnNode(size_t size, int domainId)
{
	char *ptr; 

	ptr = mmap(0, size, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, 0, 0);  

	if (ptr == (char *)-1)
    {
        ERROR;
    }

    numa_membind(ptr, size, domainId);

    return ptr;
}

static void 
initMemory(size_t size, char* ptr, int domainId)
{
    affinity_pinProcess(numa_info.nodes[domainId].processors[0]);

    for (size_t i=0; i < size; i += PAGE_ALIGNMENT)
    {
        ptr[i] = (char) 0xEF;
    }
}

static int
findProcessor(uint32_t nodeId, uint32_t coreId)
{
    int i;

    for (i=0; i<numa_info.nodes[nodeId].numberOfProcessors; i++)
    {
        if (numa_info.nodes[nodeId].processors[i] == coreId)
        {
            return 1;
        }
    }
    return 0;
}




/* #####   FUNCTION DEFINITIONS  -  EXPORTED FUNCTIONS   ################## */

void
memsweep_setMemoryFraction(uint64_t fraction)
{
    memoryFraction = fraction;
}


void
memsweep_node(void)
{
    for ( int i=0; i < numa_info.numberOfNodes; i++)
    {
        memsweep_domain(i);
    }
}


void
memsweep_domain(int domainId)
{
    char* ptr = NULL;
    size_t size = numa_info.nodes[domainId].totalMemory * 1024ULL * memoryFraction / 100ULL;
    printf("Sweeping domain %d: Using %g MB of %g MB\n",
            domainId,
            size / (1024.0 * 1024.0),
            numa_info.nodes[domainId].totalMemory/ 1024.0);
    ptr = (char*) allocateOnNode(size, domainId);
    initMemory(size, ptr, domainId);
    munmap(ptr, size);
}

void
memsweep_threadGroup(int* processorList, int numberOfProcessors)
{
    uint32_t i;
    int j;
    int ret=0;
    int numberOfNodes = 0;
    unsigned long mask = 0UL;

    for (i=0; i<numa_info.numberOfNodes; i++)
    {
        for (j=0; j<numberOfProcessors; j++)
        {
            if (findProcessor(i,processorList[j]))
            {
                memsweep_domain(i);
                break;
            }
        }
    }
}




