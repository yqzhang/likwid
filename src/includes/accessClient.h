/*
 * =======================================================================================
 *
 *      Filename:  accessClient.h
 *
 *      Description:  Header File accessClient Module. 
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

#ifndef ACCESSCLIENT_H
#define ACCESSCLIENT_H

#include <types.h>

extern int accessClient_mode;

/* This needs to be called BEFORE msr_init and
 * sets how the module tries to access the MSR registers. */
extern void accessClient_setaccessmode(int mode);

/* This needs to be called BEFORE msr_init and
 * sets the priority the module reports to the daemon.
 * This is a noop in any msr access mode except sysmsrd. */
extern void accessClient_setlowaccesspriority(void);

/* Initializes the MSR module, trying to open either the MSR files or
 * the connection to the msr daemon. */
extern void accessClient_init(int* socket_fd);
extern void accessClient_initThread(int* socket_fd);
extern void accessClient_finalize(int socket_fd);
extern uint64_t accessClient_read(int socket_fd, int cpu, int device, uint32_t reg);
extern void accessClient_write(int socket_fd, int cpu, int device, uint32_t reg, uint64_t data);

#endif /* ACCESSCLIENT_H */
