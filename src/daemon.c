/*
 * =======================================================================================
 *
 *      Filename:  daemon.c
 *
 *      Description:  C Module implementing a daemon time loop
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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>
#include <time.h>

#include <timer.h>
#include <perfmon.h>
#include <daemon.h>

static int daemon_run = 0;
static bstring eventString;


void
daemon_init(bstring str)
{
    eventString = bstrcpy(str);
    signal(SIGINT, daemon_stop);
    signal(SIGUSR1, daemon_interrupt);

}

void
daemon_start(struct timespec interval)
{
    daemon_run = 1;
    perfmon_startCounters();

    while (1)
    {
        if (daemon_run)
        {
            perfmon_readCounters();
            perfmon_logCounterResults((double) interval.tv_sec + (double) interval.tv_nsec * 1.0E-9 );
        }
        nanosleep( &interval, NULL);
    }
}

void
daemon_stop(int sig)
{
    printf("DAEMON:  EXIT on %d\n", sig);
    perfmon_stopCounters();
    signal(SIGINT, SIG_DFL);
    kill(getpid(), SIGINT);
}

void
daemon_interrupt(int sig)
{
    if (daemon_run)
    {
        perfmon_stopCounters();
        daemon_run = 0;
        printf("DAEMON:  STOP on %d\n",sig);
    }
    else
    {
        perfmon_setupEventSet(eventString);
        perfmon_startCounters();
        daemon_run = 1;
        printf("DAEMON:  START\n");
    }
}


