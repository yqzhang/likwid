/*
 * =======================================================================================
 *
 *      Filename:  cpuid.h
 *
 *      Description:  Header File cpuid Module. 
 *                    Reads out cpuid information and initilaizes a global 
 *                    data structure cpuid_info.
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

#ifndef CPUID_H
#define CPUID_H

#include <types.h>

/* Intel P6 */
#define PENTIUM_M_BANIAS     0x09U
#define PENTIUM_M_DOTHAN     0x0DU
#define CORE_DUO             0x0EU
#define CORE2_65             0x0FU
#define CORE2_45             0x17U
#define ATOM                 0x1CU
#define NEHALEM              0x1AU
#define NEHALEM_BLOOMFIELD   0x1AU
#define NEHALEM_LYNNFIELD    0x1EU
#define NEHALEM_LYNNFIELD_M  0x1FU
#define NEHALEM_WESTMERE     0x2CU
#define NEHALEM_WESTMERE_M   0x25U
#define SANDYBRIDGE          0x2AU
#define SANDYBRIDGE_EP       0x2DU
#define IVYBRIDGE            0x3AU
#define NEHALEM_EX           0x2EU
#define WESTMERE_EX          0x2FU
#define XEON_MP              0x1DU

/* Intel MIC */
#define XEON_PHI           0x01U

/* AMD K10 */
#define BARCELONA      0x02U
#define SHANGHAI       0x04U
#define ISTANBUL       0x08U
#define MAGNYCOURS     0x09U

/* AMD K8 */
#define OPTERON_SC_1MB  0x05U
#define OPTERON_DC_E    0x21U
#define OPTERON_DC_F    0x41U
#define ATHLON64_X2     0x43U
#define ATHLON64_X2_F   0x4BU
#define ATHLON64_F1     0x4FU
#define ATHLON64_F2     0x5FU
#define ATHLON64_X2_G   0x6BU
#define ATHLON64_G1     0x6FU
#define ATHLON64_G2     0x7FU


#define  P6_FAMILY        0x6U
#define  MIC_FAMILY       0xBU
#define  NETBURST_FAMILY  0xFFU
#define  K15_FAMILY       0x15U
#define  K10_FAMILY       0x10U
#define  K8_FAMILY        0xFU

/** Structure holding cpuid information
 *
 */
extern CpuInfo cpuid_info;
extern CpuTopology cpuid_topology;

/** Init routine to intialize global structure.
 *
 *  Determines: 
 *  - cpu family
 *  - cpu model
 *  - cpu stepping
 *  - cpu clock
 *  - Instruction Set Extension Flags
 *  - Performance counter features (Intel P6 only)
 *
 */
extern void cpuid_init (void);
extern void cpuid_initTopology (void);
extern void cpuid_initCacheTopology (void);
extern int  cpuid_isInCpuset(void);

#endif /*CPUID_H*/
