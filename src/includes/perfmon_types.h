/*
 * =======================================================================================
 *
 *      Filename:  perfmon_types.h
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

#ifndef PERFMON_TYPES_H
#define PERFMON_TYPES_H

#include <bstrlib.h>
#include <perfmon_group_types.h>

/* #####   EXPORTED TYPE DEFINITIONS   #################################### */

typedef enum {
    PMC0 = 0,
    PMC1, PMC2, PMC3, PMC4, PMC5, PMC6,
    PMC7, PMC8, PMC9, PMC10, PMC11, PMC12,
    PMC13, PMC14, PMC15, PMC16, PMC17, PMC18,
    PMC19, PMC20, PMC21, PMC22, PMC23, PMC24,
    PMC25, PMC26, PMC27, PMC28, PMC29, PMC30,
    PMC31, PMC32, PMC33, PMC34, PMC35, PMC36,
    PMC37, PMC38, PMC39, PMC40, PMC41, PMC42,
    PMC43, PMC44, PMC45, PMC46, PMC47,
    NUM_PMC} PerfmonCounterIndex;

typedef enum {
    PMC = 0,
    FIXED,
    UNCORE,
    MBOX0,
    MBOX1,
    MBOX2,
    MBOX3,
    MBOXFIX,
    BBOX0,
    BBOX1,
    RBOX0,
    RBOX1,
    WBOX,
    SBOX0,
    SBOX1,
    SBOX2,
    CBOX,
    PBOX,
    POWER,
    NUM_UNITS} PerfmonType;

typedef struct {
    char* key;
    PerfmonCounterIndex index;
} PerfmonCounterMap;

typedef struct {
    char* key;
    PerfmonGroup index;
    char* info;
    char* config;
} PerfmonGroupMap;

typedef struct {
    char* key;
    char* msg;
} PerfmonGroupHelp;

/* only used in westmereEX at the moment */
typedef struct {
    uint32_t  ctrlRegister;
    uint32_t  statusRegister;
    uint32_t  ovflRegister;
} PerfmonUnit;

typedef struct {
    PerfmonType  type;
    int       init;
    int       id;
    uint64_t  configRegister;
    uint64_t  counterRegister;
    uint64_t  counterRegister2;
    uint64_t  counterData;
} PerfmonCounter;

typedef struct {
    int processorId;
    PerfmonCounter counters[NUM_PMC];
} PerfmonThread;

typedef struct {
    const char*    name;
    const char*    limit;
    uint16_t eventId;
    uint8_t umask;
    uint8_t cfgBits;
    uint8_t cmask;
} PerfmonEvent;

typedef struct {
    PerfmonEvent event;
    PerfmonCounterIndex index;
    double* result;
} PerfmonEventSetEntry;

typedef struct {
    int numberOfEvents;
    PerfmonEventSetEntry* events;
} PerfmonEventSet;


typedef struct {
    bstring label;
    double* value;
} PerfmonResult;

typedef struct {
    bstrList* header;
    int numRows;
    int numColumns;
    PerfmonResult* rows;
} PerfmonResultTable;


#endif /*PERFMON_TYPES_H*/
