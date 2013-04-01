/*
 * =======================================================================================
 *
 *      Filename:  msr.c
 *
 *      Description:  Implementation of msr module.
 *                   Provides API to read and write values to the model
 *                   specific registers on x86 processors using the msr
 *                   sys interface of the Linux 2.6 kernel. This module 
 *                   is based on the msr-util tools.
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
#include <stdint.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <types.h>
#include <error.h>
#include <cpuid.h>
#include <accessClient.h>
#include <msr.h>

/* #####   MACROS  -  LOCAL TO THIS SOURCE FILE   ######################### */
#define MAX_LENGTH_MSR_DEV_NAME  20
#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

/* #####   VARIABLES  -  LOCAL TO THIS SOURCE FILE   ###################### */
static int FD[MAX_NUM_THREADS];
static int socket_fd = -1;

/* #####   FUNCTION DEFINITIONS  -  LOCAL TO THIS SOURCE FILE   ########### */


/* #####   FUNCTION DEFINITIONS  -  EXPORTED FUNCTIONS   ################## */


void
msr_init(int initSocket_fd)
{
    if (accessClient_mode == DAEMON_AM_DIRECT) 
    {

        int  fd;
#ifdef __MIC
        char* msr_file_name = "/dev/msr0";
#else
        char* msr_file_name = "/dev/cpu/0/msr";
#endif

        fd = open(msr_file_name, O_RDWR);

        if (fd < 0)
        {
            fprintf(stderr, "ERROR\n");
            fprintf(stderr, "rdmsr: failed to open '%s': %s!\n",msr_file_name , strerror(errno));
            fprintf(stderr, "       Please check if the msr module is loaded and the device file has correct permissions.\n");
            fprintf(stderr, "       Alternatively you might want to look into (sys)daemonmode.\n\n");
            exit(127);
        }

        close(fd);

        /* NOTICE: This assumes consecutive processor Ids! */
        for ( uint32_t i=0; i < cpuid_topology.numHWThreads; i++ )
        {
            char* msr_file_name = (char*) malloc(MAX_LENGTH_MSR_DEV_NAME * sizeof(char));
#ifdef __MIC
            sprintf(msr_file_name,"/dev/msr%d",i);
#else
            sprintf(msr_file_name,"/dev/cpu/%d/msr",i);
#endif

            FD[i] = open(msr_file_name, O_RDWR);

            if ( FD[i] < 0 )
            {
                ERROR;
            }
        }
    }
    else
    {
        socket_fd = initSocket_fd;
    }
}

void
msr_finalize(void)
{
    if (accessClient_mode == DAEMON_AM_DIRECT)
    {
        for ( uint32_t i=0; i < cpuid_topology.numHWThreads; i++ )
        {
            close(FD[i]);
        }
    }
    else
    {
        socket_fd = -1;
    }
}


uint64_t 
msr_tread(const int tsocket_fd, const int cpu, uint32_t reg)
{
    if (accessClient_mode == DAEMON_AM_DIRECT) 
    {
        uint64_t data;

        if ( pread(FD[cpu], &data, sizeof data, reg) != sizeof data ) 
        {
            ERROR_PRINT("cpu %d reg %x",cpu, reg);
        }

        return data;
    }
    else
    { /* daemon or sysdaemon-mode */
        return accessClient_read(tsocket_fd, cpu, DAEMON_AD_MSR, reg);
    }
}


void 
msr_twrite(const int tsocket_fd, const int cpu, uint32_t reg, uint64_t data)
{
    if (accessClient_mode == DAEMON_AM_DIRECT) 
    {
        if (pwrite(FD[cpu], &data, sizeof data, reg) != sizeof data) 
        {
            ERROR_PRINT("cpu %d reg %x",cpu, reg);
        }
    }
    else
    { /* daemon or sysdaemon-mode */
        accessClient_write(tsocket_fd, cpu, DAEMON_AD_MSR, reg, data);
    }
}


uint64_t 
msr_read( const int cpu, uint32_t reg)
{
    if (accessClient_mode == DAEMON_AM_DIRECT) 
    {
        uint64_t data;

        if ( pread(FD[cpu], &data, sizeof data, reg) != sizeof data ) 
        {
            ERROR_PRINT("cpu %d reg %x",cpu, reg);
        }

        return data;
    }
    else
    { /* daemon or sysdaemon-mode */
        return accessClient_read(socket_fd, cpu, DAEMON_AD_MSR, reg);
    }
}


void 
msr_write( const int cpu, uint32_t reg, uint64_t data)
{
    if (accessClient_mode == DAEMON_AM_DIRECT) 
    {
        if (pwrite(FD[cpu], &data, sizeof data, reg) != sizeof data) 
        {
            ERROR_PRINT("cpu %d reg %x",cpu, reg);
        }
    }
    else
    { /* daemon or sysdaemon-mode */
        accessClient_write(socket_fd, cpu, DAEMON_AD_MSR, reg, data);
    }
}


