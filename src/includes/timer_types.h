/*
 * =======================================================================================
 *
 *      Filename:  timer_types.h
 *
 *      Description:  Types file for timer module.
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

#ifndef TIMER_TYPES_H
#define TIMER_TYPES_H

#include <stdint.h>
#include <sys/time.h>


/**
 * @brief Union for cycles measurements
 * 
 * Union with either one 64 bit unsigned integer or a struct
 * of two 32 bit unsigned integers used as register eax and ebx
 * in call to rdtsc.
 */
typedef union
{  
    uint64_t int64;                   /** 64 bit unsigned integer fo cycles */
    struct {uint32_t lo, hi;} int32;  /** two 32 bit unsigned integers used
                                        for register values */
} TscCounter;

/**
 * @brief Struct holding the start and stop timestamp
 * 
 * A reference to this struct is passed to the timer functions and hold the
 * timestamps.
 */
typedef struct {
    struct timeval before;
    struct timeval after;
} TimerData;

/**
 * @brief Struct holding the start, stop and base timestamps
 * 
 * A reference to this struct is passed to the cycles functions and holds
 * the start and stop timestamp. Additionally base holds a possible overhead
 * for empty measurement or cpuid serialization.
 */

typedef struct {
    TscCounter start;
    TscCounter stop;
    uint64_t base;
} CyclesData;


#endif /*TIMER_TYPES_H*/
