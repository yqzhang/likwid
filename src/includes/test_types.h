/*
 * =======================================================================================
 *
 *      Filename:  test_types.h
 *
 *      Description:  Type definitions for benchmarking framework
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

#ifndef TEST_TYPES_H
#define TEST_TYPES_H

#include <stdint.h>
#include <bstrlib.h>

typedef void (*FuncPrototype)();

typedef enum {
    SINGLE = 0,
    DOUBLE} DataType;

typedef enum {
    STREAM_1 = 1,
    STREAM_2,
    STREAM_3,
    STREAM_4,
    STREAM_5,
    STREAM_6,
    STREAM_7,
    STREAM_8,
    STREAM_9,
    STREAM_10,
    STREAM_11,
    STREAM_12,
    STREAM_13,
    STREAM_14,
    STREAM_15,
    STREAM_16,
    STREAM_17,
    STREAM_18,
    STREAM_19,
    STREAM_20,
    STREAM_21,
    STREAM_22,
    STREAM_23,
    STREAM_24,
    STREAM_25,
    STREAM_26,
    STREAM_27,
    STREAM_28,
    STREAM_29,
    STREAM_30,
    STREAM_31,
    STREAM_32,
    STREAM_33,
    STREAM_34,
    STREAM_35,
    STREAM_36,
    STREAM_37,
    STREAM_38,
    MAX_STREAMS} Pattern;

typedef struct {
    char* name;
    Pattern streams;
    DataType type ;
    int stride;
    FuncPrototype kernel;
    int  flops;
    int  bytes;
} TestCase;

typedef struct {
    uint64_t   size;
    uint32_t   iter;
    const TestCase* test;
    uint64_t   cycles;
    uint32_t numberOfThreads;
    int* processors;
    void** streams;
} ThreadUserData;

#endif /*TEST_TYPES_H*/
