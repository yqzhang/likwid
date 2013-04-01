/*
 * =======================================================================================
 *
 *      Filename:  strUtil.c
 *
 *      Description:  Utility routines for strings. Depends on bstring lib.
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
#include <sys/types.h>
#include <unistd.h>
#include <sched.h>

#include <error.h>
#include <types.h>
#include <bstrlib.h>
#include <strUtil.h>
#include <affinity.h>
#include <cpuid.h>

/* #####   FUNCTION DEFINITIONS  -  LOCAL TO THIS SOURCE FILE   ########### */
static int
cpu_count(cpu_set_t* set)
{
    uint32_t i;
    int s = 0;
    const __cpu_mask *p = set->__bits;
    const __cpu_mask *end = &set->__bits[sizeof(cpu_set_t) / sizeof (__cpu_mask)];

    while (p < end)
    {
        __cpu_mask l = *p++;

        if (l == 0)
        {
            continue;
        }

        for (i=0; i< (sizeof(__cpu_mask)*8); i++)
        {
            if (l&(1UL<<i))
            {
                s++;
            }
        }
    }

    return s;
}

/* #####   FUNCTION DEFINITIONS  -  EXPORTED FUNCTIONS   ################## */
int
str2int(const char* str)
{
    char* endptr;
    errno = 0;
    unsigned long val;
    val = strtoul(str, &endptr, 10);

    if ((errno == ERANGE && val == LONG_MAX )
            || (errno != 0 && val == 0)) 
    {
        ERROR;
    }

    if (endptr == str) 
    {
        ERROR_PLAIN_PRINT(No digits were found);
    }

    return (int) val;
}

uint32_t
bstr_to_cpuset_physical(uint32_t* threads,  const_bstring q)
{
    int i;
    unsigned int rangeBegin;
    unsigned int rangeEnd;
    uint32_t numThreads=0;
    struct bstrList* tokens;
    struct bstrList* subtokens;

    tokens = bsplit(q,',');

    for (i=0;i<tokens->qty;i++)
    {
        subtokens = bsplit(tokens->entry[i],'-');

        if (numThreads > MAX_NUM_THREADS)
        {
            ERROR_PRINT(Number Of threads %d too large, numThreads);
        }

        if( subtokens->qty == 1 )
        {
            threads[numThreads] = str2int((char *) subtokens->entry[0]->data);
            numThreads++;
        }
        else if ( subtokens->qty == 2 )
        {
            rangeBegin = str2int((char*) subtokens->entry[0]->data);
            rangeEnd = str2int((char*) subtokens->entry[1]->data);

            if (!(rangeBegin <= rangeEnd))
            {
                ERROR_PRINT(Range End %d bigger than begin %d,rangeEnd,rangeBegin);
            }

            while (rangeBegin <= rangeEnd) {
                threads[numThreads] = rangeBegin;
                numThreads++;
                rangeBegin++;
            }
        }
        else
        {
            ERROR_PLAIN_PRINT(Parse Error);
        }
        bstrListDestroy(subtokens);
    }

    bstrListDestroy(tokens);

    return numThreads;
}

uint32_t
bstr_to_cpuset_logical(uint32_t* threads,  const_bstring q)
{
    int i;
    uint32_t j;
    uint32_t tmpThreads[MAX_NUM_THREADS];
    int globalNumThreads=0;
    uint32_t numThreads=0;
    struct bstrList* tokens;
    struct bstrList* subtokens;
    const AffinityDomain* domain;

    tokens = bsplit(q,'@');

    for (i=0;i<tokens->qty;i++)
    {
        subtokens = bsplit(tokens->entry[i],':');

        if ( subtokens->qty == 2 )
        {
            domain =  affinity_getDomain(subtokens->entry[0]);

            if (!domain)
            {
                ERROR_PRINT(Unknown domain ##%s##,bdata(subtokens->entry[0]));
            }

            numThreads = bstr_to_cpuset_physical(tmpThreads, subtokens->entry[1]);

            for (j=0; j<numThreads; j++)
            {
                if (! (tmpThreads[j] >= domain->numberOfProcessors))
                {
                    threads[globalNumThreads++] = domain->processorList[tmpThreads[j]];
                }
                else
                {
                    ERROR_PRINT(Too many threads requested. Avaialable 0-%d,domain->numberOfProcessors-1);
                }
            }
        }
        else
        {
            ERROR_PLAIN_PRINT(Parse Error);
        }
        bstrListDestroy(subtokens);
    }

    bstrListDestroy(tokens);

    return globalNumThreads;
}

#define CPUSET_ERROR  \
    if (cpuid_isInCpuset()) {  \
        ERROR_PLAIN_PRINT(You are running inside a cpuset. In cpusets only logical pinning inside set is allowed!);  \
    }



int
bstr_to_cpuset(int* threadsIN,  const_bstring q)
{
    uint32_t i;
    int num=0;
    int cpuMapping[cpuid_topology.numHWThreads];
    cpu_set_t cpu_set;
    uint32_t numThreads;
    bstring domainStr = bformat("NSCM");
    struct bstrList* tokens;
    CPU_ZERO(&cpu_set);
    sched_getaffinity(0,sizeof(cpu_set_t), &cpu_set);
    uint32_t* threads = (uint32_t*) threadsIN;


    if (binchr (q, 0, domainStr) !=  BSTR_ERR)
    {
        CPUSET_ERROR;
        numThreads =  bstr_to_cpuset_logical(threads,q);
    }
    else if (bstrchr (q, 'L') !=  BSTR_ERR)
    {
        uint32_t count = cpu_count(&cpu_set);
        printf("Using logical numbering within cpuset %d\n",count);
        tokens = bsplit(q,':');
        numThreads = bstr_to_cpuset_physical(threads,tokens->entry[1]);

        for (i=0; i <  cpuid_topology.numHWThreads; i++)
        {
            if (CPU_ISSET(i,&cpu_set))
            {
                cpuMapping[num++]=i;
            }
        }

        for (i=0; i < numThreads; i++)
        {
            if (!(threads[i] > count))
            {
                threads[i] = cpuMapping[threads[i]];
            }
            else
            {
                ERROR_PRINT(Request cpu out of range of max %d,count);
            }
        }
        bstrListDestroy(tokens);
    }
    else
    {
        CPUSET_ERROR;
        numThreads = bstr_to_cpuset_physical(threads,q);
    }

    bdestroy(domainStr);
    return (int) numThreads;
}


void
bstr_to_eventset(StrUtilEventSet* set, const_bstring q)
{
    int i;
    struct bstrList* tokens;
    struct bstrList* subtokens;

    tokens = bsplit(q,',');
    set->numberOfEvents = tokens->qty;
    set->events = (StrUtilEvent*)
        malloc(set->numberOfEvents * sizeof(StrUtilEvent));

    for (i=0;i<tokens->qty;i++)
    {
        subtokens = bsplit(tokens->entry[i],':');

        if ( subtokens->qty != 2 )
        {
            ERROR_PLAIN_PRINT(Error in parsing event string);
        }
        else
        {
            set->events[i].eventName = bstrcpy(subtokens->entry[0]);
            set->events[i].counterName = bstrcpy(subtokens->entry[1]);
        }

        bstrListDestroy(subtokens);
    }

    bstrListDestroy(tokens);
}

FILE* 
bstr_to_outstream(const_bstring argString, bstring filter)
{
    int i;
    char* cstr;
    FILE* STREAM;
    int numberOfFields;
    struct bstrList* tokens;
    bstring base;
    bstring suffix = bfromcstr(".");
    bstring filename;

    /* configure filter */
    {
        tokens = bsplit(argString,'.');

        if (tokens->qty < 2)
        {
            fprintf(stderr, "Outputfile has no filetype suffix!\n");
            fprintf(stderr, "Add suffix .txt for raw output or any supported filter suffix.\n");
            exit(EXIT_FAILURE);
        }

        base = bstrcpy(tokens->entry[0]);

        if (biseqcstr(tokens->entry[1],"txt"))
        {
            bassigncstr(filter, "NO");
        }
        else
        {
            bassigncstr(filter, TOSTRING(LIKWIDFILTERPATH));
            bconchar(filter,'/');
            bconcat(filter,tokens->entry[1]);
        }

        bconcat(suffix,tokens->entry[1]);
        bstrListDestroy(tokens);
    }

    tokens = bsplit(base,'_');
    numberOfFields = tokens->qty;

    if (tokens->qty < 1)
    {
        ERROR_PLAIN_PRINT(Error in parsing file string);
    }

    filename = bstrcpy(tokens->entry[0]);

    for (i=1; i<tokens->qty; i++)
    {
        if (biseqcstr(tokens->entry[i],"%j"))
        {
            cstr = getenv("PBS_JOBID");
            if (cstr != NULL) 
            {
                bcatcstr(filename, "_");
                bcatcstr(filename, cstr);
            }
        }
        else if (biseqcstr(tokens->entry[i],"%r"))
        {
            cstr = getenv("PMI_RANK");
            if (cstr == NULL) 
            {
                cstr = getenv("OMPI_COMM_WORLD_RANK");
            }
            if (cstr != NULL) 
            {
                bcatcstr(filename, "_");
                bcatcstr(filename, cstr);
            }
        }
        else if (biseqcstr(tokens->entry[i],"%h"))
        {
            cstr = (char*) malloc(HOST_NAME_MAX * sizeof(char));
            gethostname(cstr,HOST_NAME_MAX);
            bcatcstr(filename, "_");
            bcatcstr(filename, cstr);
            free(cstr);
        }
        else if (biseqcstr(tokens->entry[i],"%p"))
        {
            bstring pid = bformat("_%d",getpid());
            bconcat(filename, pid);
            bdestroy(pid);
        }
        else 
        {
            ERROR_PLAIN_PRINT(Unsupported placeholder in filename!);
        }
    }

    if (biseqcstr(filter,"NO"))
    {
        bconcat(filename, suffix);
    }
    else
    {
        bcatcstr(filter, " ");
        bcatcstr(filename, ".tmp");
        bconcat(filter, filename);
    }

    bstrListDestroy(tokens);
    STREAM = fopen(bdata(filename),"w");
    bdestroy(filename);
    bdestroy(suffix);
    bdestroy(base);

    return STREAM;
}


uint64_t bstr_to_doubleSize(const_bstring str, DataType type)
{
    bstring unit = bmidstr(str, blength(str)-2, 2);
    bstring sizeStr = bmidstr(str, 0, blength(str)-2);
    uint64_t sizeU = str2int(bdata(sizeStr));
    uint64_t junk = 0;
    uint64_t bytesize = 0;

    switch (type)
    {
        case SINGLE:
            bytesize = 4;
            break;

        case DOUBLE:
            bytesize = 8;
            break;
    }

    if (biseqcstr(unit, "kB")) {
        junk = (sizeU *1024)/bytesize;
    } else if (biseqcstr(unit, "MB")) {
        junk = (sizeU *1024*1024)/bytesize;
    } else if (biseqcstr(unit, "GB")) {
        junk = (sizeU *1024*1024*1024)/bytesize;
    }

    return junk;
}

void bstr_to_interval(const_bstring str, struct timespec* interval)
{
    int size;
    int pos;
    bstring ms = bformat("ms");

    if ((pos = bstrrchr (str, 's')) != BSTR_ERR)
    {
        if (pos != (blength(str)-1))
        {
            ERROR_PLAIN_PRINT(Parsing of daemon interval failed);
        }

        /* unit is ms */
        if (binstrr (str, blength(str), ms) != BSTR_ERR)
        {
            bstring sizeStr = bmidstr(str, 0, blength(str)-2);
            size = str2int(bdata(sizeStr));
            if (size >= 1000)
            {
                interval->tv_sec = size/1000;
                interval->tv_nsec = (size%1000) * 1.E06;
            }
            else
            {
                interval->tv_sec = 0L;
                interval->tv_nsec = size * 1.E06;
            }
        }
        /* unit is s */
        else 
        {
            bstring sizeStr = bmidstr(str, 0, blength(str)-1);
            size = str2int(bdata(sizeStr));
            interval->tv_sec = size;
            interval->tv_nsec = 0L;
        }
    }
    else
    {
        ERROR_PLAIN_PRINT(Parsing of daemon interval failed);
    }
}


void bstr_to_workgroup(Workgroup* group,
        const_bstring str,
        DataType type,
        int numberOfStreams)
{
    uint32_t i;
    int parseStreams = 0;
    bstring threadInfo;
    bstring streams= bformat("0");
    struct bstrList* tokens;
    struct bstrList* subtokens;
    const AffinityDomain* domain;

    /* split the workgroup into the thread and the streams part */
    tokens = bsplit(str,'-');

    if (tokens->qty == 2)
    {
        threadInfo = bstrcpy(tokens->entry[0]);
        streams = bstrcpy(tokens->entry[1]);
        parseStreams = 1;
    } 
    else if (tokens->qty == 1)
    {
        threadInfo = bstrcpy(tokens->entry[0]);
    }
    else
    {
        ERROR_PLAIN_PRINT(Error in parsing workgroup string);
    }

    bstrListDestroy (tokens);
    tokens = bsplit(threadInfo,':');

    if (tokens->qty == 3)
    {
        domain = affinity_getDomain(tokens->entry[0]);

        if (domain == NULL)
        {
            fprintf(stderr, "Error: Domain %s not available on current machine.\nTry likwid-bench -p for supported domains.",
                    bdata(tokens->entry[0]));
            exit(EXIT_FAILURE);
        }

        group->size = bstr_to_doubleSize(tokens->entry[1], type);
        group->numberOfThreads = str2int(bdata(tokens->entry[2]));

        if (group->numberOfThreads > domain->numberOfProcessors)
        {
            fprintf(stderr, "Error: Domain %s supports only up to %d threads.\n",
                    bdata(tokens->entry[0]),domain->numberOfProcessors);
            exit(EXIT_FAILURE);
        }

        group->processorIds = (int*) malloc(group->numberOfThreads * sizeof(int));

        for (i=0; i<group->numberOfThreads; i++)
        {
            group->processorIds[i] = domain->processorList[i];
        }
    } 
    else if (tokens->qty == 2)
    {
        domain = affinity_getDomain(tokens->entry[0]);
        group->size = bstr_to_doubleSize(tokens->entry[1], type);
        group->numberOfThreads = domain->numberOfProcessors;

        group->processorIds = (int*) malloc(group->numberOfThreads * sizeof(int));

        for (i=0; i<group->numberOfThreads; i++)
        {
            group->processorIds[i] = domain->processorList[i];
        }
    }
    else
    {
        ERROR_PLAIN_PRINT(Error in parsing workgroup string);
    }

    bstrListDestroy(tokens);

    /* parse stream list */
    if (parseStreams)
    {
        tokens = bsplit(streams,',');

        if (tokens->qty < numberOfStreams)
        {
            ERROR_PRINT(Testcase requires at least %d streams, numberOfStreams);
        }

        group->streams = (Stream*) malloc(numberOfStreams * sizeof(Stream));

        for (i=0;i<(uint32_t) tokens->qty;i++)
        {
            subtokens = bsplit(tokens->entry[i],':');

            if ( subtokens->qty == 3 )
            {
               int index = str2int(bdata(subtokens->entry[0]));
               if (index >= numberOfStreams)
               {
                   ERROR_PRINT(Stream Index %d out of range,index);
               }
               group->streams[index].domain = bstrcpy(subtokens->entry[1]);
               group->streams[index].offset = str2int(bdata(subtokens->entry[2]));
            }
            else if ( subtokens->qty == 2 )
            {
                int index = str2int(bdata(subtokens->entry[0]));
                if (index >= numberOfStreams)
                {
                   ERROR_PRINT(Stream Index %d out of range,index);
                }
                group->streams[index].domain = bstrcpy(subtokens->entry[1]);
                group->streams[index].offset = 0;
            }
            else
            {
                ERROR_PLAIN_PRINT(Error in parsing event string);
            }

            bstrListDestroy(subtokens);
        }

        bstrListDestroy(tokens);
    }
    else
    {
        group->streams = (Stream*) malloc(numberOfStreams * sizeof(Stream));

        for (i=0; i< (uint32_t)numberOfStreams; i++)
        {
            group->streams[i].domain = domain->tag;
            group->streams[i].offset = 0;
        }
    }

    group->size /= numberOfStreams;
}


#define INIT_SECURE_INPUT_LENGTH 256

bstring
bSecureInput (int maxlen, char* vgcCtx) {
    int i, m, c = 1;
    bstring b, t;
    int termchar = 0;

    if (!vgcCtx) return NULL;

    b = bfromcstralloc (INIT_SECURE_INPUT_LENGTH, "");

    for (i=0; ; i++)
    {
        if (termchar == c)
        {
            break;
        }
        else if ((maxlen > 0) && (i >= maxlen))
        {
            b = NULL;
            return b;
        }
        else
        {
            c = *(vgcCtx++);
        }

        if (EOF == c)
        {
            break;
        }

        if (i+1 >= b->mlen) {

            /* Double size, but deal with unusual case of numeric
               overflows */

            if ((m = b->mlen << 1)   <= b->mlen     &&
                    (m = b->mlen + 1024) <= b->mlen &&
                    (m = b->mlen + 16)   <= b->mlen &&
                    (m = b->mlen + 1)    <= b->mlen)
            {
                t = NULL;
            }
            else
            {
                t = bfromcstralloc (m, "");
            }

            if (t)
            {
                memcpy (t->data, b->data, i);
            }

            bdestroy (b); /* Clean previous buffer */
            b = t;
            if (!b)
            {
                return b;
            }
        }

        b->data[i] = (unsigned char) c;
    }

    i--;
    b->slen = i;
    b->data[i] = (unsigned char) '\0';
    return b;
}


int bJustifyCenter (bstring b, int width) 
{
    unsigned char space  = ' ';
    int alignSpace = (width - b->slen) / 2;
    int restSpace = (width - b->slen) % 2;
    if (width <= 0) return -__LINE__;

    if (b->slen <= width)
    {
        binsertch (b, 0, alignSpace, space);
    }

    binsertch (b, b->slen , alignSpace+restSpace, space);

    return BSTR_OK;
}


