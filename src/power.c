/*
 * =======================================================================================
 *
 *      Filename:  power.c
 *
 *      Description:  Module implementing Intel RAPL interface
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
#include <math.h>

#include <types.h>
#include <power.h>
#include <cpuid.h>

/* #####   EXPORTED VARIABLES   ########################################### */

PowerInfo power_info;
const uint32_t power_regs[4] = {MSR_PKG_ENERGY_STATUS,
                                MSR_PP0_ENERGY_STATUS,
                                MSR_PP1_ENERGY_STATUS,
                                MSR_DRAM_ENERGY_STATUS};

/* #####   FUNCTION DEFINITIONS  -  LOCAL TO THIS SOURCE FILE   ########### */



/* #####   FUNCTION DEFINITIONS  -  EXPORTED FUNCTIONS   ################## */

void
power_init(int cpuId)
{
    uint64_t flags;
    uint32_t tmp;
    int hasRAPL = 0;

    /* determine Turbo Mode features */
    double busSpeed;

    if ((cpuid_info.model == SANDYBRIDGE_EP) ||
            (cpuid_info.model == SANDYBRIDGE) ||
            (cpuid_info.model == IVYBRIDGE))
    {
        hasRAPL = 1;
    }

    if (cpuid_info.turbo)
    {
        flags = msr_read(cpuId, MSR_PLATFORM_INFO);

        if ( hasRAPL )
        {
            busSpeed = 100.0;
        }
        else 
        {
            busSpeed = 133.33;
        }

        power_info.baseFrequency = busSpeed * (double) extractBitField(flags,8,8);
        power_info.minFrequency  = busSpeed * (double) extractBitField((flags>>(32)),8,8);

        power_info.turbo.numSteps = cpuid_topology.numCoresPerSocket;
        power_info.turbo.steps = (double*) malloc(power_info.turbo.numSteps * sizeof(double));

        flags = msr_read(cpuId, MSR_TURBO_RATIO_LIMIT);

        for (int i=0; i < power_info.turbo.numSteps; i++)
        {
            if ( i < 4 )
            {
                power_info.turbo.steps[i] = busSpeed * (double) extractBitField(flags,8,i*8);
            }
            else
            {
                power_info.turbo.steps[i] = busSpeed * (double) extractBitField(flags>>(32),8,i*8);
            }
        }
    }
    else
    {
        power_info.turbo.numSteps = 0;
    }

    /* determine RAPL parameters */
    if ( hasRAPL )
    {
        flags = msr_read(cpuId, MSR_RAPL_POWER_UNIT);

        power_info.powerUnit = pow(0.5,(double) extractBitField(flags,4,0));
        power_info.energyUnit = pow(0.5,(double) extractBitField(flags,5,8));
        power_info.timeUnit = pow(0.5,(double) extractBitField(flags,4,16));

        flags = msr_read(cpuId, MSR_PKG_POWER_INFO);
        power_info.tdp = (double) extractBitField(flags,15,0) * power_info.powerUnit;
        power_info.minPower =  (double) extractBitField(flags,15,16) * power_info.powerUnit;
        power_info.maxPower = (double) extractBitField(flags,15,32) * power_info.powerUnit;
        power_info.maxTimeWindow = (double) extractBitField(flags,15,48) * power_info.timeUnit;
    }
    else
    {
        power_info.powerUnit = 0.0;
    }
}

