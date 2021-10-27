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

#ifndef VCA_WINXP_H
#define VCA_WINXP_H

#if defined(_WIN32) && (_WIN32_WINNT < 0x0600) // _WIN32_WINNT_VISTA

#ifdef _MSC_VER
#include <intrin.h> // _InterlockedCompareExchange64
#endif

namespace VCA_NS {
/* non-native condition variable */
typedef struct
{
    CRITICAL_SECTION broadcastMutex;
    CRITICAL_SECTION waiterCountMutex;
    HANDLE semaphore;
    HANDLE waitersDone;
    volatile int waiterCount;
    volatile int bIsBroadcast;
} ConditionVariable;

int WINAPI cond_init(ConditionVariable *cond);
void WINAPI cond_broadcast(ConditionVariable *cond);
void WINAPI cond_signal(ConditionVariable *cond);
BOOL WINAPI cond_wait(ConditionVariable *cond, CRITICAL_SECTION *mutex, DWORD wait);
void cond_destroy(ConditionVariable *cond);

/* map missing API symbols to our structure and functions */
#define CONDITION_VARIABLE          VCA_NS::ConditionVariable
#define InitializeConditionVariable VCA_NS::cond_init
#define SleepConditionVariableCS    VCA_NS::cond_wait
#define WakeConditionVariable       VCA_NS::cond_signal
#define WakeAllConditionVariable    VCA_NS::cond_broadcast
#define XP_CONDITION_VAR_FREE       VCA_NS::cond_destroy

} // namespace VCA_NS

#else // if defined(_WIN32) && (_WIN32_WINNT < 0x0600)

#define XP_CONDITION_VAR_FREE(x)

#endif // _WIN32_WINNT <= _WIN32_WINNT_WINXP

#endif // ifndef VCA_WINXP_H
