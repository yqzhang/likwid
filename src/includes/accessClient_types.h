/*
 * =======================================================================================
 *
 *      Filename:  accessClient_types.h
 *
 *      Description:  Types file for accessClient module.
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

#ifndef ACCESSCLIENT_TYPES_H
#define ACCESSCLIENT_TYPES_H

#include <stdint.h>

/* This naming with AccessType and AccessMode is admittedly a bit confusing */
typedef enum {
    DAEMON_AM_DIRECT = 0, 
    DAEMON_AM_ACCESS_D,
    DAEMON_AM_SYSACCESS_D
} AccessMode;

typedef enum {
    DAEMON_READ = 0,
    DAEMON_WRITE,
    DAEMON_EXIT,
    DAEMON_MARK_CLIENT_LOWPRIO
} AccessType;

typedef enum {
    DAEMON_AD_PCI_R3QPI_LINK_0 = 0,
    DAEMON_AD_PCI_R3QPI_LINK_1,
    DAEMON_AD_PCI_R2PCIE,
    DAEMON_AD_PCI_IMC_CH_0,
    DAEMON_AD_PCI_IMC_CH_1,
    DAEMON_AD_PCI_IMC_CH_2,
    DAEMON_AD_PCI_IMC_CH_3,
    DAEMON_AD_PCI_HA,
    DAEMON_AD_PCI_QPI_PORT_0,
    DAEMON_AD_PCI_QPI_PORT_1,
    DAEMON_AD_PCI_QPI_MASK_PORT_0,
    DAEMON_AD_PCI_QPI_MASK_PORT_1,
    DAEMON_AD_PCI_QPI_MISC_PORT_0,
    DAEMON_AD_PCI_QPI_MISC_PORT_1,
    DAEMON_AD_MSR
} AccessDevice;

typedef enum {
    ERR_NOERROR = 0,  /* no error */
    ERR_UNKNOWN,      /* unknown command */
    ERR_RESTREG,      /* attempt to access restricted MSR */
    ERR_OPENFAIL,     /* failure to open msr files */
    ERR_RWFAIL,       /* failure to read/write msr */
    ERR_DAEMONBUSY    /* daemon already has another client */
} AccessErrorType;

typedef struct {
    uint32_t cpu;
    uint32_t reg;
    uint64_t data;
    AccessDevice device;
    AccessType type;
    AccessErrorType errorcode; /* Only in replies - 0 if no error. */
} AccessDataRecord;

#endif /*ACCESSCLIENT_TYPES_H*/
