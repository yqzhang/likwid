/*
 * =======================================================================================
 *
 *      Filename:  perfmon.h
 *
 *      Description:  Header File of perfmon module.
 *                    Configures and reads out performance counters
 *                    on x86 based architectures. Supports multi threading.
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

#ifndef PERFMON_H
#define PERFMON_H

#include <bstrlib.h>
#include <types.h>

extern int perfmon_verbose;

extern void (*perfmon_startCountersThread) (int thread_id);
extern void (*perfmon_stopCountersThread) (int thread_id);
extern int  (*perfmon_getIndex) (bstring reg, PerfmonCounterIndex* index);
extern void (*perfmon_setupCounterThread) (int thread_id,
       PerfmonEvent* event , PerfmonCounterIndex index);

extern void perfmon_initEventSet(StrUtilEventSet* eventSetConfig, PerfmonEventSet* set);
extern void perfmon_setCSVMode(int v);
extern void perfmon_printAvailableGroups(void);
extern void perfmon_printGroupHelp(bstring group);
extern void perfmon_init(int numThreads, int threads[],FILE* outstream);
extern void perfmon_finalize(void);
extern void perfmon_setupEventSet(bstring eventString);
extern double perfmon_getEventResult(int thread, int index);
extern int perfmon_setupEventSetC(char* eventCString, const char*** eventnames);
extern void perfmon_setupCounters(void);
extern void perfmon_startCounters(void);
extern void perfmon_stopCounters(void);
extern void perfmon_readCounters(void);
extern double perfmon_getResult(int threadId, char* counterString);
extern void perfmon_printMarkerResults(bstring filepath);
extern void perfmon_logCounterResults(double time);
extern void perfmon_printCounterResults(void);
extern void perfmon_printCounters(void);
extern void perfmon_printEvents(void);

#endif /*PERFMON_H*/
