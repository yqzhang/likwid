/*
 * =======================================================================================
 *
 *      Filename:  hashTable.c
 *
 *      Description: Hashtable implementation based on SGLIB.
 *                   Used for Marker API result handling.
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
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

#include <sglib.h>
#include <bstrlib.h>
#include <types.h>
#include <hashTable.h>
#include <likwid.h>

typedef struct {
    pthread_t tid;
    uint32_t coreId;
    LikwidThreadResults* hashTable[HASH_TABLE_SIZE];
    uint32_t currentMaxSize;
    uint32_t numberOfRegions;
} ThreadList;


static ThreadList* threadList[MAX_NUM_THREADS];

static unsigned int hashFunction(LikwidThreadResults* item)
{
    const char* str =  bdata(item->label);
    unsigned int len = blength(item->label);
    unsigned int b    = 378551;
    unsigned int a    = 63689;
    unsigned int hash = 0;
    unsigned int i    = 0;

    for(i = 0; i < len; str++, i++)
    {
        hash = hash * a + (*str);
        a    = a * b;
    }

    return hash;
}

/* ======================================================================== */
#define SLIST_COMPARATOR(e1, e2)    bstrncmp((e1)->label,(e2)->label,100)

SGLIB_DEFINE_LIST_PROTOTYPES(LikwidThreadResults,SLIST_COMPARATOR , next)
SGLIB_DEFINE_LIST_FUNCTIONS(LikwidThreadResults,SLIST_COMPARATOR , next)
SGLIB_DEFINE_HASHED_CONTAINER_PROTOTYPES(LikwidThreadResults, HASH_TABLE_SIZE, hashFunction)
SGLIB_DEFINE_HASHED_CONTAINER_FUNCTIONS(LikwidThreadResults, HASH_TABLE_SIZE, hashFunction)

void
hashTable_init()
{
    for (int i=0; i<MAX_NUM_THREADS; i++)
    {
        threadList[i] = NULL;
    }
}


int
hashTable_get(bstring label, LikwidThreadResults** resEntry)
{
    LikwidThreadResults li;
    int coreID = likwid_getProcessorId();

    ThreadList* resPtr = threadList[coreID];
    li.label = label;

    /* check if thread was already initialized */
    if (resPtr == NULL)
    {
        resPtr = (ThreadList*) malloc(sizeof(ThreadList));
        /* initialize structure */
        resPtr->tid =  pthread_self();
        resPtr->coreId  = coreID;
        resPtr->numberOfRegions = 0;
        sglib_hashed_LikwidThreadResults_init(resPtr->hashTable);
        threadList[coreID] = resPtr;
    }

    /* if region is not known create new region and add to hashtable */
    if (((*resEntry) = sglib_hashed_LikwidThreadResults_find_member(resPtr->hashTable, &li)) == NULL) 
    {
        (*resEntry) = (LikwidThreadResults*) malloc(sizeof(LikwidThreadResults));
        (*resEntry)->label = bstrcpy (label);
        (*resEntry)->time = 0.0;
        (*resEntry)->count = 0;
        resPtr->numberOfRegions++; 
        for (int i=0; i< NUM_PMC; i++) (*resEntry)->PMcounters[i] = 0.0;
        sglib_hashed_LikwidThreadResults_add(resPtr->hashTable, (*resEntry));
    }

    return coreID;
}

void
hashTable_finalize(int* numThreads, int* numRegions, LikwidResults** results)
{
    int init = 0;
    int threadId = 0;
    int regionId = 0;
    uint32_t numberOfThreads = 0;
    uint32_t numberOfRegions = 0;
    struct sglib_hashed_LikwidThreadResults_iterator hash_it;

    /* determine number of threads */
    for (int i=0; i<MAX_NUM_THREADS; i++)
    {
        if (threadList[i] != NULL)
        {
            numberOfThreads++;
            if (!init)
            {
                /* determine number of regions */
                numberOfRegions = threadList[i]->numberOfRegions;
                init = 1;
            } 
            else
            {
                if (numberOfRegions != threadList[i]->numberOfRegions)
                {
                    printf("Different number of regions!! %d\n",threadList[i]->numberOfRegions);
                }
            }
        }
    }

    init = 0;

    for (int core=0; core<MAX_NUM_THREADS; core++)
    {
        ThreadList* resPtr = threadList[core];
        if (resPtr != NULL)
        {

            resPtr->numberOfRegions=0;
            LikwidThreadResults* hash  = NULL;

            if (!init)
            {
                init =1;
                for(hash=sglib_hashed_LikwidThreadResults_it_init(&hash_it,resPtr->hashTable);
                        hash!=NULL; hash=sglib_hashed_LikwidThreadResults_it_next(&hash_it))
                {
                    resPtr->numberOfRegions++;
                }

                if( resPtr->numberOfRegions != numberOfRegions)
                {
                    printf("Different number of regions!!\n");
                }

                /* allocate data structure */
                (*results) = (LikwidResults*) malloc(numberOfRegions * sizeof(LikwidResults));

                for ( uint32_t i=0; i < numberOfRegions; i++ )
                {
                    (*results)[i].time = (double*) malloc(numberOfThreads * sizeof(double));
                    (*results)[i].count = (uint32_t*) malloc(numberOfThreads * sizeof(uint32_t));
                    (*results)[i].counters = (double**) malloc(numberOfThreads * sizeof(double*));

                    for ( uint32_t j=0; j < numberOfThreads; j++ )
                    {
                        (*results)[i].counters[j] = (double*) malloc(NUM_PMC * sizeof(double));
                    }
                }
            }

            regionId = 0;
            /* iterate over all regions in thread */
            for ( hash=sglib_hashed_LikwidThreadResults_it_init(&hash_it,resPtr->hashTable);
                    hash!=NULL; hash=sglib_hashed_LikwidThreadResults_it_next(&hash_it) )
            {
                (*results)[regionId].tag = bstrcpy (hash->label);
                (*results)[regionId].count[threadId] = hash->count;
                (*results)[regionId].time[threadId] = hash->time;

                for ( int j=0; j < NUM_PMC; j++ )
                {
                    (*results)[regionId].counters[threadId][j] = hash->PMcounters[j];
                }

                regionId++;
            }

            threadId++;
        }
    }

    (*numThreads) = numberOfThreads;
    (*numRegions) = numberOfRegions;
}


