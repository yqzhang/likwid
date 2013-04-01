/*
 * =======================================================================================
 *
 *      Filename:  rdtsc.h
 *
 *      Description:  rdtsc module header
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

#ifndef RDTSC_H
#define RDTSC_H

#include <types.h>

#define RDTSC(cpu_c) \
__asm__ volatile( "rdtsc\n\t"           \
"movl %%eax, %0\n\t"  \
"movl %%edx, %1\n\t"  \
: "=r" ((cpu_c).int32.lo), "=r" ((cpu_c).int32.hi) \
: : "%eax", "%edx")

#define RDTSC2(cpu_c) \
__asm__ volatile("xor %%eax,%%eax\n\t"           \
"cpuid\n\t"           \
"rdtsc\n\t"           \
"movl %%eax, %0\n\t"  \
"movl %%edx, %1\n\t"  \
: "=r" ((cpu_c).int32.lo), "=r" ((cpu_c).int32.hi) \
: : "%eax","%ebx","%ecx","%edx")


#endif /*RDTSC_H*/
