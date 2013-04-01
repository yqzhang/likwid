/*
 * =======================================================================================
 *
 *      Filename:  power.h
 *
 *      Description:  Header File Power Module. 
 *                    Implements Intel RAPL Interface.
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

#ifndef POWER_H
#define POWER_H

#include <types.h>
#include <registers.h>
#include <bitUtil.h>
#include <msr.h>

extern PowerInfo power_info;
extern  const uint32_t power_regs[4];

extern void power_init(int cpuId);
static inline void power_start(PowerData* data, int cpuId, PowerType type);
static inline void power_stop(PowerData* data, int cpuId, PowerType type);
static inline uint32_t power_read(int cpuId, uint64_t reg);
static inline uint32_t power_tread(int socket_fd, int cpuId, uint64_t reg);
static inline double power_printEnergy(PowerData* data);

static double
power_printEnergy(PowerData* data)
{
    return  (double) ((data->after - data->before) * power_info.energyUnit);
}

static void
power_start(PowerData* data, int cpuId, PowerType type)
{
    data->before = extractBitField(msr_read(cpuId, power_regs[type]),32,0);
}

static void
power_stop(PowerData* data, int cpuId, PowerType type)
{
    data->after = extractBitField(msr_read(cpuId, power_regs[type]),32,0);
}

static uint32_t
power_read(int cpuId, uint64_t reg)
{
    return extractBitField(msr_read(cpuId, reg),32,0);
}

static uint32_t
power_tread(int socket_fd, int cpuId, uint64_t reg)
{
    return extractBitField(msr_tread(socket_fd, cpuId, reg),32,0);
}

#endif /*POWER_H*/
