/*
 * =======================================================================================
 *
 *      Filename:  perfmon_ivybridge.h
 *
 *      Description:  Header File of perfmon module for Ivy Bridge.
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

#include <perfmon_ivybridge_events.h>
#include <perfmon_ivybridge_groups.h>

#define NUM_COUNTERS_IVYBRIDGE 11

static int perfmon_numCountersIvybridge = NUM_COUNTERS_IVYBRIDGE;
static int perfmon_numGroupsIvybridge = NUM_GROUPS_IVYBRIDGE;
static int perfmon_numArchEventsIvybridge = NUM_ARCH_EVENTS_IVYBRIDGE;

static PerfmonCounterMap ivybridge_counter_map[NUM_COUNTERS_IVYBRIDGE] = {
    {"FIXC0",PMC0},
    {"FIXC1",PMC1},
    {"FIXC2",PMC2},
    {"PMC0",PMC3},
    {"PMC1",PMC4},
    {"PMC2",PMC5},
    {"PMC3",PMC6},
    {"PWR0",PMC7},
    {"PWR1",PMC8},
    {"PWR2",PMC9},
    {"PWR3",PMC10},
};



