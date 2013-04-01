/*
 * =======================================================================================
 *
 *      Filename:  error.h
 *
 *      Description:  Central error handling macros
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

#ifndef ERROR_H
#define ERROR_H

#include <errno.h>
#include <string.h>
#include <msr.h>

#define str(x) #x

#define FINALIZE  msr_finalize()

#define ERRNO_PRINT fprintf(stderr, "ERROR - [%s:%d] %s\n", __FILE__, __LINE__, strerror(errno))

#define ERROR  \
    ERRNO_PRINT; \
    FINALIZE; \
    exit(EXIT_FAILURE)

#define ERROR_PLAIN_PRINT(msg) \
   fprintf(stderr,  "ERROR - [%s:%d] " str(msg) "\n", __FILE__, __LINE__);  \
   FINALIZE; \
   exit(EXIT_FAILURE)


#define ERROR_PRINT(fmt, ...) \
   fprintf(stderr,  "ERROR - [%s:%d] " str(fmt) "\n", __FILE__, __LINE__, __VA_ARGS__);  \
   FINALIZE; \
   exit(EXIT_FAILURE)

#define CHECK_ERROR(func, msg)  \
    if ((func) < 0) { \
        fprintf(stderr, "ERROR - [%s:%d] " str(msg) " - %s \n", __FILE__, __LINE__, strerror(errno));  \
    }

#define EXIT_IF_ERROR(func, msg)  \
    if ((func) < 0) {  \
        fprintf(stderr,"ERROR - [%s:%d] " str(msg) " - %s \n", __FILE__, __LINE__, strerror(errno)); \
        FINALIZE; \
        exit(EXIT_FAILURE); \
    }

#ifndef DEBUGLEV
#define DEBUGLEV 0
#endif

#define VERBOSEPRINTREG(cpuid,reg,flags,msg) \
    if (perfmon_verbose) {  \
        printf("DEBUG - [%s:%d] "  str(msg) " [%d] Register 0x%llX , Flags: 0x%llX \n",  \
                __FILE__, __LINE__,  (cpuid), LLU_CAST (reg), LLU_CAST (flags)); \
        fflush(stdout);  \
    } 


#define DEBUG_PRINT(lev, fmt, ...) \
    if (DEBUGLEV > lev) { \
        printf(fmt, __VA_ARGS__); \
        fflush(stdout); \
    }

#endif /*ERROR_H*/
