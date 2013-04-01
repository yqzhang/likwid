/*
 * =======================================================================================
 *      Filename:  timer.h
 *
 *      Description:  Measure runtime with getTimeOfday and rdtsc.
 *
 *      A C module to measure runtime. There are two methods: with gettimeofday
 *      for longer time periods (over 0.5 sec) and with rdtsc (read time stamp
 *      counter) for shorter periods. There is a variation for measurements
 *      with rdtsc of 100 cycles in the worst case. Therefore sensible
 *      measurements should be over 1000 cycles.
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

#ifndef TIMER_H
#define TIMER_H

#include <sys/time.h>
#include <timer_types.h>

#define RDTSC2(cpu_c) \
__asm__ volatile( "rdtsc\n\t"           \
"movl %%eax, %0\n\t"  \
"movl %%edx, %1\n\t"  \
: "=r" ((cpu_c).int32.lo), "=r" ((cpu_c).int32.hi) \
: : "%eax", "%edx")

#define RDTSC(cpu_c) \
__asm__ volatile("xor %%eax,%%eax\n\t"           \
"cpuid\n\t"           \
"rdtsc\n\t"           \
"movl %%eax, %0\n\t"  \
"movl %%edx, %1\n\t"  \
: "=r" ((cpu_c).int32.lo), "=r" ((cpu_c).int32.hi) \
: : "%eax","%ebx","%ecx","%edx")



/**
 * @brief  Initialize timer module
 *
 * Determines processor clock and cycles for cpuid.
 */
extern void timer_init(void );

/**
 * @brief  Start timer measurement with getTimeofDay
 * @param  time  Reference to struct holding the timestamps
 */
extern void timer_start(TimerData* time);

/**
 * @brief  Stop timer measurement with getTimeofDay
 * @param  time Reference to struct holding the timestamps
 */
extern void timer_stop(TimerData* time);

/**
 * @brief  Get timer measurement with getTimeofDay
 * @param  time Reference to struct holding the timestamps
 * @return Time duration between start and stop in seconds
 */
extern float timer_print(TimerData* timer);

/**
 * @brief  Start cycles measurement with rdtsc
 * @param cycles Reference to struct holding the timestamps 
 */
extern void timer_startCycles(CyclesData* cycles);

/**
 * @brief  Stop cycles measurement with rdtsc
 * @param cycles Reference to struct holding the timestamps 
 */
extern void timer_stopCycles(CyclesData* cycles);


/**
 * @brief  Get time of cycles measurement
 * @param cycles Reference to struct holding the timestamps 
 * @return Timer duration between start and stop in seconds
 */
extern float timer_printCyclesTime(CyclesData* cycles);

/**
 * @brief  Get cycles of cycles measurement
 * @param cycles Reference to struct holding the timestamps 
 * @return cycles between start and stop
 */
extern uint64_t timer_printCycles(CyclesData* cycles);

/**
 * @brief  Get Clock rate of cpu in Hertz 
 * @return clock rate of cpu
 */
extern uint64_t timer_getCpuClock(void);

/**
 * @brief  Get cycles for cpuid
 * @return cycles for cpuid
 */
extern uint64_t timer_getCpuidCycles(void);

#endif /* TIMER_H */
