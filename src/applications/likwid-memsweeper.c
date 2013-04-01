/*
 * =======================================================================================
 *
 *      Filename:  likwid-memsweeper.c
 *
 *      Description:  An application to clean up NUMA memory domains.
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
#include <error.h>
#include <cpuid.h>
#include <numa.h>
#include <affinity.h>
#include <memsweep.h>

/* #####   MACROS  -  LOCAL TO THIS SOURCE FILE   ######################### */

#define HELP_MSG \
printf("\nlikwid-memsweeper --  Version  %d.%d \n\n",VERSION,RELEASE); \
printf("A tool clean up NUMA memory domains.\n"); \
printf("Options:\n"); \
printf("-h\t Help message\n"); \
printf("-v\t Version information\n"); \
printf("-c\t specify NUMA domain ID to clean up\n"); \
printf("Usage: likwid-memsweeper \n"); \
printf("To clean specific domain: likwid-memsweeper -c 2 \n");

#define VERSION_MSG \
printf("likwid-memsweeper  %d.%d \n\n",VERSION,RELEASE)


int main (int argc, char** argv)
{ 
    int domainId = -1;
    int c;
    bstring argString;

    while ((c = getopt (argc, argv, "+c:hv")) != -1)
    {
        switch (c)
        {
            case 'h':
                HELP_MSG;
                exit (EXIT_SUCCESS);    
            case 'v':
                VERSION_MSG;
                exit (EXIT_SUCCESS);    
            case 'c':
                if (! (argString = bSecureInput(10,optarg)))
                {
                    fprintf(stderr,"Failed to read argument string!\n");
                    exit(EXIT_FAILURE);
                }

                domainId = str2int((char*) argString->data);

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
    numa_init();

    if (domainId < 0) 
    {
        memsweep_node();
    }
    else
    {
        memsweep_domain(domainId);
    }

    return EXIT_SUCCESS;
}

