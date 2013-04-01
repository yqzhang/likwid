/*
 * =======================================================================================
 *
 *      Filename:  strUtil.h
 *
 *      Description:  Header File strUtil Module. 
 *                    Helper routines for bstrlib and command line parsing
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

#ifndef STRUTIL_H
#define STRUTIL_H

#include <bstrlib.h>
#include <types.h>
#include <time.h>

#define CHECK_OPTION_STRING  \
if (! (argString = bSecureInput(200,optarg))) {  \
    ERROR_PLAIN_PRINT(Failed to read argument string!);  \
}

extern int str2int(const char* str);
extern uint32_t bstr_to_cpuset_physical(uint32_t* threads,  const_bstring q);
extern int bstr_to_cpuset(int* threads,  const_bstring str);
extern void bstr_to_eventset(StrUtilEventSet* set, const_bstring str);
extern bstring bSecureInput (int maxlen, char* vgcCtx);
extern int bJustifyCenter (bstring b, int width);
extern void bstr_to_workgroup(Workgroup* threads,  const_bstring str, DataType type, int numberOfStreams);
extern FILE* bstr_to_outstream(const_bstring argString, bstring filter);
extern uint64_t bstr_to_doubleSize(const_bstring str, DataType type);
extern void bstr_to_interval(const_bstring str, struct timespec* interval);

#endif /*STRUTIL_H*/
