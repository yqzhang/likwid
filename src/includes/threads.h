/*
 * =======================================================================================
 *
 *      Filename:  threads.h
 *
 *      Description:  Header file of pthread interface module
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

#ifndef THREADS_H
#define THREADS_H

#include <types.h>
#include <pthread.h>
#include <threads_types.h>

#define THREADS_BARRIER pthread_barrier_wait(&threads_barrier)

extern pthread_barrier_t threads_barrier;
extern ThreadData* threads_data;
extern ThreadGroup* threads_groups;


/**
 * @brief  Initialization of the thread module
 * @param  numberOfThreads  The total number of threads
 */
extern void threads_init(int numberOfThreads);

/**
 * @brief  Create all threads
 * @param  startRoutine thread entry function pointer
 */
extern void threads_create(void *(*startRoutine)(void*));

/**
 * @brief  Register User thread data for all threads
 * @param  data  Reference to the user data structo
 * @param  func  Optional function pointer to copy data
 */
extern void threads_registerDataAll(
        ThreadUserData* data,
        threads_copyDataFunc func);

/**
 * @brief  Register User thread data for one thread
 * @param  threadId thread Id 
 * @param  data  Reference to the user data structo
 * @param  func  Optional function pointer to copy data
 */
extern void threads_registerDataThread(
        int threadId,
        ThreadUserData* data,
        threads_copyDataFunc func);

/**
 * @brief  Register User thread data for a thread group
 * @param  groupId  group Id
 * @param  data  Reference to the user data structo
 * @param  func  Optional function pointer to copy data
 */
extern void threads_registerDataGroup(
        int groupId,
        ThreadUserData* data,
        threads_copyDataFunc func);

/**
 * @brief  Join the threads and free memory of data structures
 * @param
 */
extern void threads_destroy(void);

/**
 * @brief  Create Thread groups
 * @param  numberOfGroups The number of groups to create
 */
extern void threads_createGroups(int numberOfGroups);

#endif /* THREADS_H */
