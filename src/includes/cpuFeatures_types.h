/*
 * =======================================================================================
 *
 *      Filename:  cpuFeatures_types.h
 *
 *      Description:  Types file for CpuFeature module.
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

#ifndef CPUFEATURES_TYPES_H
#define CPUFEATURES_TYPES_H

typedef enum {
    HW_PREFETCHER=0,
    CL_PREFETCHER,
    DCU_PREFETCHER,
    IP_PREFETCHER} CpuFeature;

typedef struct {
	unsigned int fastStrings:1;
	unsigned int thermalControl:1;
	unsigned int perfMonitoring:1;
	unsigned int hardwarePrefetcher:1;
	unsigned int ferrMultiplex:1;
	unsigned int branchTraceStorage:1;
	unsigned int pebs:1;
	unsigned int speedstep:1;
	unsigned int monitor:1;
	unsigned int clPrefetcher:1;
	unsigned int speedstepLock:1;
	unsigned int cpuidMaxVal:1;
	unsigned int xdBit:1;
	unsigned int dcuPrefetcher:1;
	unsigned int dynamicAcceleration:1;
	unsigned int turboMode:1;
	unsigned int ipPrefetcher:1;
    } CpuFeatureFlags;



#endif /*CPUFEATURES_TYPES_H*/
