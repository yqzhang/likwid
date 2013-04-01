/*
 * =======================================================================================
 *
 *      Filename:  likwid-features.c
 *
 *      Description:  An application to read out and set the feature flag
 *                  register on Intel Core 2 processors.
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
#include <sched.h>
#include <sys/types.h>
#include <unistd.h>
#include <ctype.h>

#include <types.h>
#include <strUtil.h>
#include <accessClient.h>
#include <msr.h>
#include <cpuid.h>
#include <cpuFeatures.h>

#define HELP_MSG \
printf("\nlikwid-features --  Version  %d.%d \n\n",VERSION,RELEASE); \
printf("A tool to print and toggle the feature flag msr on Intel CPUS.\n"); \
printf("Supported Features: HW_PREFETCHER, CL_PREFETCHER, DCU_PREFETCHER, IP_PREFETCHER.\n\n"); \
printf("Options:\n"); \
printf("-h\t Help message\n"); \
printf("-v\t Version information\n"); \
printf("-s <FEATURE>\t set cpu feature \n"); \
printf("-u <FEATURE>\t unset cpu feature \n"); \
printf("-c <ID>\t core id\n\n")

#define VERSION_MSG \
printf("likwid-features  %d.%d \n\n",VERSION,RELEASE)

int main (int argc, char** argv)
{ 
    int socket_fd = -1;
    int optSetFeature = 0;
    int cpuId = 0;
    int c;
    bstring argString;
    CpuFeature feature = HW_PREFETCHER ;

    while ((c = getopt (argc, argv, "c:s:u:hv")) != -1)
    {
        switch (c)
        {
            case 'h':
                HELP_MSG;
                exit (EXIT_SUCCESS);    
            case 'v':
                VERSION_MSG;
                exit (EXIT_SUCCESS);    
            case 'u':
                optSetFeature = 2;
            case 's':
                if (! (argString = bSecureInput(20,optarg)))
                {
                    fprintf(stderr,"Failed to read argument string!\n");
                    exit(EXIT_FAILURE);
                }

                if (biseqcstr(argString,"HW_PREFETCHER")) 
                {
                    feature = HW_PREFETCHER;
                }
                else if (biseqcstr(argString,"CL_PREFETCHER")) 
                {
                    feature = CL_PREFETCHER;
                }
                else if (biseqcstr(argString,"DCU_PREFETCHER")) 
                {
                    feature = DCU_PREFETCHER;
                }
                else if (biseqcstr(argString,"IP_PREFETCHER")) 
                {
                    feature = IP_PREFETCHER;
                }
                else
                {
                    fprintf(stderr,"Feature not supported!\n");
                    exit(EXIT_FAILURE);
                }


                if (!optSetFeature)
                {
                    optSetFeature = 1;
                }
                break;
            case 'c':
                if (! (argString = bSecureInput(10,optarg)))
                {
                    fprintf(stderr,"Failed to read argument string!\n");
                    exit(EXIT_FAILURE);
                }

                cpuId = str2int((char*) argString->data);

                break;
            case '?':
                if (isprint (optopt))
                {
                    fprintf (stderr, "Unknown option `-%c'.\n", optopt);
                }
                else
                {
                    fprintf (stderr,
                            "Unknown option character `\\x%x'.\n",
                            optopt);
                }
                return EXIT_FAILURE;
            default:
                HELP_MSG;
                exit (EXIT_SUCCESS);    
        }
    }

    cpuid_init();

    printf(HLINE);
    printf("CPU name:\t%s \n",cpuid_info.name);
    printf("CPU core id:\t%d \n", cpuId);

    if (cpuid_info.family != P6_FAMILY)
    {
        fprintf (stderr, "likwid-features only supports Intel P6 based processors!\n");
        exit(EXIT_FAILURE);
    }

    if (cpuId >= (int) cpuid_topology.numHWThreads)
    {
        fprintf (stderr, "This processor has only %d HWthreads! \n",cpuid_topology.numHWThreads);
        exit(EXIT_FAILURE);
    }

    accessClient_init(&socket_fd);
    msr_init(socket_fd);
	cpuFeatures_init(cpuId);
    cpuFeatures_print(cpuId);

    if (optSetFeature == 1)
    {
        printf(SLINE);
        cpuFeatures_enable(cpuId, feature);
        printf(SLINE);
    }
    else if (optSetFeature == 2)
    {
        printf(SLINE);
        cpuFeatures_disable(cpuId, feature);
        printf(SLINE);
    }

    msr_finalize();
    return EXIT_SUCCESS;
}

