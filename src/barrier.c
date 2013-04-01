/*
 * =======================================================================================
 *
 *      Filename:  barrier.c
 *
 *      Description:  Implementation of threaded spin loop barrier
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
#include <barrier.h>

/* #####   EXPORTED VARIABLES   ########################################### */


/* #####   MACROS  -  LOCAL TO THIS SOURCE FILE   ######################### */

#define CACHELINE_SIZE 64

/* #####   VARIABLES  -  LOCAL TO THIS SOURCE FILE   ###################### */

static BarrierGroup* groups;
static int currentGroupId = 0;
static int maxGroupId = 0;

/* #####   FUNCTION DEFINITIONS  -  LOCAL TO THIS SOURCE FILE   ########### */


/* #####   FUNCTION DEFINITIONS  -  EXPORTED FUNCTIONS   ################## */

int
barrier_registerGroup(int numThreads)
{
    int ret;

    if (currentGroupId > maxGroupId)
    {
        ERROR_PRINT(Group ID %d larger than maxGroupID %d,currentGroupId,maxGroupId);
    }

    groups[currentGroupId].numberOfThreads = numThreads;
    ret = posix_memalign(
            (void**) &groups[currentGroupId].groupBval,
            CACHELINE_SIZE, 
            numThreads * 32 * sizeof(int));

    if (ret < 0)
    {
        ERROR;
    }


    return currentGroupId++;
}

void
barrier_registerThread(BarrierData* barr, int groupId, int threadId)
{
    int ret;
    int i;
    int j = 1;
    if (groupId > currentGroupId)
    {
        ERROR_PLAIN_PRINT(Group not yet registered);
    }
    if (threadId > groups[groupId].numberOfThreads)
    {
        ERROR_PRINT(Thread ID %d too large,threadId);
    }

    barr->numberOfThreads = groups[groupId].numberOfThreads;
    barr->offset = 0;
    barr->val = 1;
    barr->bval =  groups[groupId].groupBval;
    ret = posix_memalign(
            (void**) &(barr->index),
            CACHELINE_SIZE, 
            barr->numberOfThreads * sizeof(int));

    if (ret < 0)
    {
        ERROR;
    }


    barr->index[0] = threadId;

    for (i = 0; i < barr->numberOfThreads; i++)
    {
        if (!(i == threadId))
        {
            barr->index[j++] = i;
        }
    }
}


void
barrier_init(int numberOfGroups) 
{ 
    maxGroupId = numberOfGroups-1;
    groups = (BarrierGroup*) malloc(numberOfGroups * sizeof(BarrierGroup));
}

void
barrier_synchronize(BarrierData* barr)
{
    int i;

    barr->bval[barr->index[0] * 32 +  barr->offset * 16] = barr->val;

    for (i = 1; i < barr->numberOfThreads; i++)
    {
        while (barr->bval[barr->index[i] * 32 + barr->offset * 16] != barr->val)
        {
            __asm__ ("pause");
        }
    } 
    
    if (barr->offset)
    {
        barr->val = !barr->val;
    }
    barr->offset = !barr->offset;
}


