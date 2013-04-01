/*
 * =======================================================================================
 *
 *      Filename:  allocator.c
 *
 *      Description:  Implementation of allocator module.
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

#include <error.h>
#include <types.h>
#include <allocator.h>
#include <affinity.h>

/* #####   EXPORTED VARIABLES   ########################################### */


/* #####   MACROS  -  LOCAL TO THIS SOURCE FILE   ######################### */



/* #####   VARIABLES  -  LOCAL TO THIS SOURCE FILE   ###################### */

static int numberOfAllocatedVectors = 0;
static void** allocations;


/* #####   FUNCTION DEFINITIONS  -  LOCAL TO THIS SOURCE FILE   ########### */


/* #####   FUNCTION DEFINITIONS  -  EXPORTED FUNCTIONS   ################## */

void
allocator_init(int numVectors)
{
    allocations = (void**) malloc(numVectors * sizeof(void*));
}


void
allocator_finalize()
{
    int i;

    for (i=0; i<numberOfAllocatedVectors; i++)
    {
        free(allocations[i]);
    }
}

void
allocator_allocateVector(
        void** ptr,
        int alignment,
        uint64_t size,
        int offset,
        DataType type,
        bstring domainString)
{
    size_t bytesize = 0;
    const AffinityDomain* domain;
    int errorCode;

    switch ( type )
    {
        case SINGLE:
            bytesize = (size+offset) * sizeof(float);
            break;

        case DOUBLE:
            bytesize = (size+offset) * sizeof(double);
            break;
    }

    errorCode =  posix_memalign(ptr, alignment, bytesize);

    if (errorCode)
    {
        if (errorCode == EINVAL) 
        {
            fprintf(stderr,
                    "Alignment parameter is not a power of two\n");
            exit(EXIT_FAILURE);
        }
        if (errorCode == ENOMEM) 
        {
            fprintf(stderr,
                    "Insufficient memory to fulfill the request\n");
            exit(EXIT_FAILURE);
        }
    }

    if ((*ptr) == NULL)
    {
            fprintf(stderr, "posix_memalign failed!\n");
            exit(EXIT_FAILURE);

    }

    allocations[numberOfAllocatedVectors] = *ptr;
    numberOfAllocatedVectors++;
    domain = affinity_getDomain(domainString);
    affinity_pinProcess(domain->processorList[0]);

    printf("Allocate: Process running on core %d - Vector length %llu Offset %d\n",
            affinity_processGetProcessorId(),
            size,
            offset);

    switch ( type )
    {
        case SINGLE:
            {
                float* sptr = (float*) (*ptr);
                sptr += offset;

                for ( uint64_t i=0; i < size; i++ )
                {
                    sptr[i] = 0.0;
                }
                *ptr = (void*) sptr;

            }
            break;

        case DOUBLE:
            {
                double* dptr = (double*) (*ptr);
                dptr += offset;

                for ( uint64_t i=0; i < size; i++ )
                {
                    dptr[i] = 0.0;
                }
                *ptr = (void*) dptr;
            }
            break;
    }
}

