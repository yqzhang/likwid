/*
 * =======================================================================================
 *
 *      Filename:  cpuid_types.h
 *
 *      Description:  Types file for cpuid module.
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

#ifndef CPUID_TYPES_H
#define CPUID_TYPES_H

typedef enum {
    NOCACHE=0,
    DATACACHE,
    INSTRUCTIONCACHE,
    UNIFIEDCACHE,
    ITLB,
    DTLB} CacheType;

typedef enum {
    NODE=0,
    SOCKET,
    CORE,
    THREAD} NodeLevel;

typedef struct {
    uint32_t family;
    uint32_t model;
    uint32_t stepping;
    uint64_t clock;
    int      turbo;
    char*  name;
    char*  features;
    uint32_t perf_version;
    uint32_t perf_num_ctr;
    uint32_t perf_width_ctr;
    uint32_t perf_num_fixed_ctr;
} CpuInfo;

typedef struct {
    uint32_t threadId;
    uint32_t coreId;
    uint32_t packageId;
    uint32_t apicId;
} HWThread;

typedef struct {
    int level;
    CacheType type;
    int associativity;
    int sets;
    int lineSize;
    int size;
    int threads;
    int inclusive;
} CacheLevel;

typedef struct {
    uint32_t numHWThreads;
    uint32_t numSockets;
    uint32_t numCoresPerSocket;
    uint32_t numThreadsPerCore;
    uint32_t numCacheLevels;
    HWThread* threadPool;
    CacheLevel*  cacheLevels;
    TreeNode* topologyTree;
} CpuTopology;


#endif /*CPUID_TYPES_H*/
