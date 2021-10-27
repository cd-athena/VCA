/*****************************************************************************
 * Copyright (C) 2021 Christian Doppler Laboratory ATHENA
 *
 * Authors: Steve Borho <steve@borho.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.
 *****************************************************************************/

#include "common.h"
#include "threading.h"

namespace VCA_NS {
// vca private namespace

#define STACK_ALIGN(func, ...) func(__VA_ARGS__)

#if NO_ATOMICS
pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;

int no_atomic_or(int* ptr, int mask)
{ 
    pthread_mutex_lock(&g_mutex);
    int ret = *ptr;
    *ptr |= mask;
    pthread_mutex_unlock(&g_mutex);
    return ret;
}

int no_atomic_and(int* ptr, int mask)
{
    pthread_mutex_lock(&g_mutex);
    int ret = *ptr;
    *ptr &= mask;
    pthread_mutex_unlock(&g_mutex);
    return ret;
}

int no_atomic_inc(int* ptr)
{
    pthread_mutex_lock(&g_mutex);
    *ptr += 1;
    int ret = *ptr;
    pthread_mutex_unlock(&g_mutex);
    return ret;
}

int no_atomic_dec(int* ptr)
{
    pthread_mutex_lock(&g_mutex);
    *ptr -= 1;
    int ret = *ptr;
    pthread_mutex_unlock(&g_mutex);
    return ret;
}

int no_atomic_add(int* ptr, int val)
{
    pthread_mutex_lock(&g_mutex);
    *ptr += val;
    int ret = *ptr;
    pthread_mutex_unlock(&g_mutex);
    return ret;
}
#endif

/* C shim for forced stack alignment */
static void stackAlignMain(Thread *instance)
{
    // defer processing to the virtual function implemented in the derived class
    instance->threadMain();
}

#if _WIN32

static DWORD WINAPI ThreadShim(Thread *instance)
{
    STACK_ALIGN(stackAlignMain, instance);

    return 0;
}

bool Thread::start()
{
    DWORD threadId;

    thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadShim, this, 0, &threadId);

    return threadId > 0;
}

void Thread::stop()
{
    if (thread)
        WaitForSingleObject(thread, INFINITE);
}

Thread::~Thread()
{
    if (thread)
        CloseHandle(thread);
}

#else /* POSIX / pthreads */

static void *ThreadShim(void *opaque)
{
    // defer processing to the virtual function implemented in the derived class
    Thread *instance = reinterpret_cast<Thread *>(opaque);

    STACK_ALIGN(stackAlignMain, instance);

    return NULL;
}

bool Thread::start()
{
    if (pthread_create(&thread, NULL, ThreadShim, this))
    {
        thread = 0;
        return false;
    }

    return true;
}

void Thread::stop()
{
    if (thread)
        pthread_join(thread, NULL);
}

Thread::~Thread() {}

#endif // if _WIN32

Thread::Thread()
{
    thread = 0;
}

}
