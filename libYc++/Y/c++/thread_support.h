/************************************************************************
 *   Copyright (C) Andrew Suffield <asuffield@debian.org>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License as published by the Free Software Foundation; either
 *   version 2.1 of the License, or (at your option) any later version.
 *
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *   Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with this library; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef THREAD_SUPPORT_H
#define THREAD_SUPPORT_H

#include <iostream>
#include <pthread.h>

extern void cancel_unlock_helper(void *mutex_v);

#undef MUTEX_DEBUG

#ifdef MUTEX_DEBUG
#define note_locking(N, A) std::cerr << "Locking " << N << " (at " << A << ")" << std::endl
#define note_locked(N, A) std::cerr << "Locked " << N << " (at " << A << ")" << std::endl
#define note_unlocking(N, A) std::cerr << "Unlocking " << N << " (at " << A << ")" << std::endl
#else
#define note_locking(N, A)
#define note_locked(N, A)
#define note_unlocking(N, A)
#endif

/* These don't really behave like statements. They introduce a { and
 * }. pthreads sucks. Deal with it.
 */

#define lock_mutex(M, O)                                  \
  note_locking(#M, &(M));                                 \
  pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, &(O));   \
  pthread_cleanup_push(&cancel_unlock_helper, &(M));      \
  pthread_mutex_lock(&(M));                               \
  note_locked(#M, &(M))

#define unlock_mutex(O)                         \
  note_unlocking("?", 0);                       \
  pthread_cleanup_pop(1);                       \
  pthread_setcanceltype((O), NULL)

#endif

/* arch-tag: beacdae1-f4c4-4682-8984-dfaf63f0dd5d
 */
