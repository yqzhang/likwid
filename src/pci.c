/*
 * =======================================================================================
 *
 *      Filename:  pci.c
 *
 *      Description:  Implementation of pci module.
 *                   Provides API to read and write values to the hardware
 *                   performance monitoring registers in PCI Cfg space
 *                   for Intel Sandy Bridge Processors.
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
#include <accessClient.h>
#include <bstrlib.h>
#include <error.h>
#include <pci.h>
#include <cpuid.h>
#include <affinity.h>

/* #####   MACROS  -  LOCAL TO THIS SOURCE FILE   ######################### */
#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

#define PCI_ROOT_PATH  "/proc/bus/pci/"

/* #####   VARIABLES  -  LOCAL TO THIS SOURCE FILE   ###################### */

static int socket_fd = -1;
static int FD[MAX_NUM_NODES][MAX_NUM_DEVICES];
static char* pci_DevicePath[MAX_NUM_DEVICES] = {
 "13.5", "13.6", "13.1", "10.0", "10.1", "10.4",
 "10.5", "0e.1", "08.2", "09.2", "08.6", "09.6",
 "08.0", "09.0" };

/* Socket to bus mapping
 * Socket  Bus (2S)  Bus (4s)
 *   0        0xff      0x3f
 *   1        0x7f      0x7f
 *   2                  0xbf
 *   3                  0xff
 */

static char* socket_bus[] = { "7f/", "ff/",  "bf/",  "ff/" };

/* #####   FUNCTION DEFINITIONS  -  LOCAL TO THIS SOURCE FILE   ########### */


/* #####   FUNCTION DEFINITIONS  -  EXPORTED FUNCTIONS   ################## */

void
pci_init(int initSocket_fd)
{
    bstring filepath =  bfromcstr ( PCI_ROOT_PATH );
    bcatcstr(filepath, socket_bus[0]);
    bcatcstr(filepath, pci_DevicePath[0] );

    int fd = open( bdata(filepath), O_RDONLY);

    if (fd < 0)
    {
        fprintf(stderr, "INFO\n");
        fprintf(stderr, "       This system has no support for PCI based Uncore counters.\n");
        fprintf(stderr, "       This means you cannot use performance groups as MEM, which require Uncore counters.\n\n");
        return;
    }

    close(fd);

    if (accessClient_mode == DAEMON_AM_DIRECT) 
    {
        if(geteuid() != 0)
        {
            fprintf(stderr, "WARNING\n");
            fprintf(stderr, "       Direct access to the PCI Cfg Adressspace is only allowed for uid root!\n");
            fprintf(stderr, "       This means you can use performance groups as MEM only as root in direct mode.\n");
            fprintf(stderr, "       Alternatively you might want to look into (sys)daemonmode.\n\n");
        }

        for (int j=0; j<MAX_NUM_NODES; j++)
        {
            for (int i=0; i<MAX_NUM_DEVICES; i++)
            {
                FD[j][i] = 0;
            }
        }

        if ( cpuid_topology.numSockets == 2 )
        {
            /* Already correctly initialized */
        }
        else if ( cpuid_topology.numSockets == 4 )
        {
            strcpy(socket_bus[1],"3f/");
        }
    }
    else /* daemon or sysdaemon-mode */
    { 
        socket_fd = initSocket_fd;
    }
}


void
pci_finalize()
{
    if (accessClient_mode != DAEMON_AM_DIRECT)
    {
        for (int j=0; j<MAX_NUM_NODES; j++)
        {
            for (int i=0; i<MAX_NUM_DEVICES; i++)
            {
                if (FD[j][i])
                {
                    close(FD[i][i]);
                }
            }
        }
    }
    else
    {
        socket_fd = -1;
    }
}


uint32_t
pci_read(int cpu, PciDeviceIndex device, uint32_t reg)
{
    int socketId = affinity_core2node_lookup[cpu];

    if (accessClient_mode == DAEMON_AM_DIRECT)
    {
        uint32_t data;

        if ( !FD[socketId][device] )
        {
            bstring filepath =  bfromcstr ( PCI_ROOT_PATH );
            bcatcstr(filepath, socket_bus[socketId]);
            bcatcstr(filepath, pci_DevicePath[device] );
            FD[socketId][device] = open( bdata(filepath), O_RDWR);

            if ( FD[socketId][device] < 0)
            {
                fprintf(stderr, "ERROR\n");
                fprintf(stderr, "failed to open pci device: %s!\n", strerror(errno));
                exit(127);
            }
        }

        if ( pread(FD[socketId][device], &data, sizeof data, reg) != sizeof data ) 
        {
            ERROR_PRINT("cpu %d reg %x",cpu, reg);
        }

        return data;
    }
    else
    { /* daemon or sysdaemon-mode */
        return (uint32_t) accessClient_read(socket_fd, socketId, device, reg);
    }
}



void
pci_write(int cpu, PciDeviceIndex device, uint32_t reg, uint32_t data)
{
    int socketId = affinity_core2node_lookup[cpu];

    if (accessClient_mode == DAEMON_AM_DIRECT)
    {

        if ( !FD[socketId][device] )
        {
            bstring filepath =  bfromcstr ( PCI_ROOT_PATH );
            bcatcstr(filepath, socket_bus[socketId]);
            bcatcstr(filepath, pci_DevicePath[device] );
            FD[socketId][device] = open( bdata(filepath), O_RDWR);

            if ( FD[socketId][device] < 0)
            {
                fprintf(stderr, "ERROR\n");
                fprintf(stderr, "failed to open pci device: %s!\n", strerror(errno));
                exit(127);
            }
        }

        if (pwrite(FD[socketId][device], &data, sizeof data, reg) != sizeof data) 
        {
            ERROR_PRINT("cpu %d reg %x",cpu, reg);
        }

        //    printf("WRITE Device %s cpu %d reg 0x%x data 0x%x \n",bdata(filepath), cpu, reg, data);
    }
    else
    { /* daemon or sysdaemon-mode */
        accessClient_write(socket_fd, socketId, device, reg, (uint64_t) data);
    }
}

uint32_t
pci_tread(const int tsocket_fd, const int cpu, PciDeviceIndex device, uint32_t reg)
{
    int socketId = affinity_core2node_lookup[cpu];

    if (accessClient_mode == DAEMON_AM_DIRECT)
    {
        uint32_t data;

        if ( !FD[socketId][device] )
        {
            bstring filepath =  bfromcstr ( PCI_ROOT_PATH );
            bcatcstr(filepath, socket_bus[socketId]);
            bcatcstr(filepath, pci_DevicePath[device] );
            //        printf("Generate PATH = %s \n",bdata(filepath));

            FD[socketId][device] = open( bdata(filepath), O_RDWR);

            if ( FD[socketId][device] < 0)
            {
                fprintf(stderr, "ERROR\n");
                fprintf(stderr, "failed to open pci device: %s!\n", strerror(errno));
                exit(127);
            }
        }

        if ( pread(FD[socketId][device], &data, sizeof data, reg) != sizeof data ) 
        {
            ERROR_PRINT("cpu %d reg %x",cpu, reg);
        }
        //    printf("READ Device %s cpu %d reg 0x%x data 0x%x \n",bdata(filepath), cpu, reg, data);

        return data;
    }
    else
    { /* daemon or sysdaemon-mode */
        return accessClient_read(tsocket_fd, socketId, device, reg);
    }
}

void
pci_twrite( const int tsocket_fd, const int cpu, PciDeviceIndex device, uint32_t reg, uint32_t data)
{
    int socketId = affinity_core2node_lookup[cpu];

    if (accessClient_mode == DAEMON_AM_DIRECT)
    {

        if ( !FD[socketId][device] )
        {
            bstring filepath =  bfromcstr ( PCI_ROOT_PATH );
            bcatcstr(filepath, socket_bus[socketId]);
            bcatcstr(filepath, pci_DevicePath[device] );
            //        printf("Generate PATH = %s \n",bdata(filepath));

            FD[socketId][device] = open( bdata(filepath), O_RDWR);

            if ( FD[socketId][device] < 0)
            {
                fprintf(stderr, "ERROR\n");
                fprintf(stderr, "failed to open pci device: %s!\n", strerror(errno));
                exit(127);
            }
        }

        if (pwrite(FD[socketId][device], &data, sizeof data, reg) != sizeof data) 
        {
            ERROR_PRINT("cpu %d reg %x",cpu, reg);
        }

        //    printf("WRITE Device %s cpu %d reg 0x%x data 0x%x \n",bdata(filepath), cpu, reg, data);
    }
    else
    { /* daemon or sysdaemon-mode */
        accessClient_write(tsocket_fd, socketId, device, reg, data);
    }
}



