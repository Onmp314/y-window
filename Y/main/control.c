/************************************************************************
 *   Copyright (C) Mark Thomas <markbt@efaref.net>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

#include <Y/main/control.h>
#include <Y/util/yutil.h>
#include <Y/util/index.h>
#include <Y/util/pqueue.h>
#include <stdio.h>
#include <signal.h>
#include <sys/time.h>
#include <assert.h>

static int controlRunning = 1;

struct ControlFileDescriptor
{
  int fd;
  int watchMask;
  void *userData;
  void (*callback)(int, int, void *);
};

static int
controlFileDescriptorsKeyFunction (const void *key_v, const void *obj_v)
{
  const int *key_p = key_v;
  const struct ControlFileDescriptor *obj = obj_v;
  return *key_p - obj -> fd;  
}

static int
controlFileDescriptorsComparisonFunction (const void *obj1_v, const void *obj2_v)
{
  const struct ControlFileDescriptor *obj1 = obj1_v;
  const struct ControlFileDescriptor *obj2 = obj2_v;
  return obj1->fd - obj2->fd;
}

static void
controlFileDescriptorsDestructorFunction (void *obj_v)
{
  yfree (obj_v);
}

static inline int
cmp_v (void *p1, void *p2)
{
  if (p1 == p2)
    return 0;
  else if (p1 < p2)
    return -1;
  else
    return 1;
}

struct ControlSignalHandler
{
  void *userData;
  void (*callback)(int, void *);
};

static int
controlSignalHandlerComparisonFunction (const void *obj1_v, const void *obj2_v)
{
  const struct ControlSignalHandler *obj1 = obj1_v, *obj2 = obj2_v;
  if (obj1->userData == obj2->userData)
    return cmp_v(obj1->callback, obj2->callback);
  else
    return cmp_v(obj1->userData, obj2->userData);
}

static void
controlSignalHandlerDestructorFunction (void *obj_v)
{
  yfree (obj_v);
}

struct ControlSignalHandlerSet
{
  int signo;
  struct Index *handlers;
};

static int
controlSignalHandlerSetKeyFunction (const void *key_v, const void *obj_v)
{
  const int *key_p = key_v;
  const struct ControlSignalHandlerSet *obj = obj_v;
  return *key_p - obj->signo;
}

static int
controlSignalHandlerSetComparisonFunction (const void *obj1_v, const void *obj2_v)
{
  const struct ControlSignalHandlerSet *obj1 = obj1_v;
  const struct ControlSignalHandlerSet *obj2 = obj2_v;
  return obj1->signo - obj2->signo;
}

static void
controlSignalHandlerSetDestructorFunction (void *obj_v)
{
  struct ControlSignalHandlerSet *obj = obj_v;
  indexDestroy (obj->handlers, controlSignalHandlerDestructorFunction);
  yfree (obj);
}

struct ControlTimedEvent
{
  struct timeval time;
  int id;
  void *userData;
  void (*callback)(void *);
};

static int nextTimedEventID = 1;

static int
controlTimedEventsComparisonFunction (const void *obj1_v, const void *obj2_v)
{
  const struct ControlTimedEvent *obj1 = obj1_v;
  const struct ControlTimedEvent *obj2 = obj2_v;
  if (obj1 -> time.tv_sec == obj2 -> time.tv_sec)
    {
       return obj1 -> time.tv_usec - obj2 -> time.tv_usec;
    }
  else
    {
       return obj1 -> time.tv_sec - obj2 -> time.tv_sec;
    }
}

static void
controlTimedEventDestructorFunction (void *obj_v)
{
  yfree (obj_v);
}

static struct Index *fileDescriptors, *signalHandlers;
static struct PQueue *timedEvents; 

static void
exitSignalHandler (int signo, void *userData)
{
  controlShutdownY ();
}

void
controlInitialise (void)
{
  int i;
  sigset_t block_mask;

  fileDescriptors = indexCreate (controlFileDescriptorsKeyFunction,
                                 controlFileDescriptorsComparisonFunction);
  signalHandlers = indexCreate (controlSignalHandlerSetKeyFunction,
                                controlSignalHandlerSetComparisonFunction);
  timedEvents = pqueueCreate (controlTimedEventsComparisonFunction);

  /* Block all the signals, except the ones which we should not handle (fatal errors) */
  sigfillset(&block_mask);
  sigdelset(&block_mask, SIGSTOP);
  sigdelset(&block_mask, SIGKILL);
  sigdelset(&block_mask, SIGFPE);
  sigdelset(&block_mask, SIGSEGV);
  sigdelset(&block_mask, SIGILL);
  sigdelset(&block_mask, SIGBUS);
  sigdelset(&block_mask, SIGABRT);
  /* Leave these alone for debuggers, if they exist */
#ifdef SIGPROF
  sigdelset(&block_mask, SIGPROF);
#endif
#ifdef SIGTRAP
  sigdelset(&block_mask, SIGTRAP);
#endif
  sigprocmask(SIG_BLOCK, &block_mask, NULL);

  /* And register these ones to make sure we exit when we should */
  controlRegisterSignalHandler(SIGTERM, NULL, &exitSignalHandler);
  controlRegisterSignalHandler(SIGINT, NULL, &exitSignalHandler);
}

void
controlRegisterFileDescriptor (int fd, int watchMask, void *userData,
              void (*callback)(int fd, int causeMask, void *userData))
{
  assert(callback != NULL);
  struct ControlFileDescriptor *obj = ymalloc (sizeof (*obj));
  obj -> fd = fd;
  obj -> watchMask = watchMask;
  obj -> userData = userData;
  obj -> callback = callback;
  indexAdd (fileDescriptors, obj);
}

void
controlChangeFileDescriptorMask (int fd, int watchMask)
{
  struct ControlFileDescriptor *obj = indexFind (fileDescriptors, &fd);
  if (obj != NULL)
    {
      obj -> watchMask = watchMask;
    }
}

void
controlUnregisterFileDescriptor (int fd)
{
  void *obj = indexRemove (fileDescriptors, &fd);
  yfree (obj);
}

void
controlRegisterSignalHandler (int signo, void *userData, void (*callback)(int signo, void *userData))
{
  assert(callback != NULL);
  struct ControlSignalHandlerSet *set = indexFind (signalHandlers, &signo);
  if (!set)
    {
      set = ymalloc (sizeof (*set));
      set->signo = signo;
      set->handlers = indexCreate(controlSignalHandlerComparisonFunction,
                                  controlSignalHandlerComparisonFunction);
      indexAdd (signalHandlers, set);
    }

  struct ControlSignalHandler *obj = ymalloc (sizeof (*obj));
  obj -> userData = userData;
  obj -> callback = callback;
  indexAdd (set->handlers, obj);
}

void
controlUnregisterSignalHandler (int signo, void *userData, void (*callback)(int signo, void *userData))
{
  struct ControlSignalHandlerSet *set = indexFind (signalHandlers, &signo);
  if (set)
    {
      struct ControlSignalHandler obj = {.userData = userData, .callback = callback};
      yfree (indexRemove (set->handlers, &obj));
    }
}

static void
signalHandlerIterator(const void *obj_v, void *set_v)
{
  struct ControlSignalHandlerSet *set = set_v;
  const struct ControlSignalHandler *obj = obj_v;
  obj->callback(set->signo, obj->userData);
}

static void
signalHandler(int signo)
{
  struct ControlSignalHandlerSet *set = indexFind (signalHandlers, &signo);
  indexIterate (set->handlers, set, signalHandlerIterator);
}

static void
controlPollSignals (void)
{
  sigset_t pending_signals;
  int i;
  sigpending(&pending_signals);
  for (i = 1; i < NSIG; i++)
    if (sigismember(&pending_signals, i))
      {
        struct sigaction sa;
        sigset_t block_mask;

        sa.sa_handler = &signalHandler;
        sigfillset(&sa.sa_mask);
        sa.sa_flags = SA_ONESHOT;
        sigaction(i, &sa, NULL);

        sigfillset(&block_mask);
        sigdelset(&block_mask, i);
        sigsuspend(&block_mask);
      }
}

int
controlTimerDelay (int minIntervalSeconds, int minIntervalMilliseconds,
                   void *userData, void (*callback)(void *userData))
{
  struct timeval now;
  struct ControlTimedEvent *event;

  event = ymalloc (sizeof (struct ControlTimedEvent));
  event -> userData = userData;
  event -> callback = callback;
  event -> id = nextTimedEventID ++;

  gettimeofday (&now, NULL);

  event -> time.tv_usec = (minIntervalMilliseconds * 1000 + now.tv_usec);
  event -> time.tv_sec = minIntervalSeconds + now.tv_sec;
  if (event -> time.tv_usec >= 1000000)
    {
      event -> time.tv_sec++;
      event -> time.tv_usec -= 1000000;
    }

  pqueueInsert (timedEvents, event); 

  return event -> id;
}

static int
controlCancelTimerDelayTestFunction (void *timedEvent_v, void *id_v)
{
  int *id = id_v;
  struct ControlTimedEvent *timedEvent = timedEvent_v;
  if (timedEvent -> id == *id)
    {
      yfree(timedEvent);
      return 1;
    }
  else
    return 0;
}

void
controlCancelTimerDelay (int id)
{
  pqueueRemove (timedEvents, &id, controlCancelTimerDelayTestFunction);  
}

static int
controlIteration (void)
{
  struct ControlTimedEvent *tev;
  struct IndexIterator *iter;
  struct timeval timeout;
  int maxFd = 0;
  int retval;
  fd_set fds[3];
  FD_ZERO (&fds[0]); 
  FD_ZERO (&fds[1]);  
  FD_ZERO (&fds[2]);

  iter = indexGetStartIterator (fileDescriptors);
  while (indexiteratorHasValue (iter))
    {
      struct ControlFileDescriptor *cfd = indexiteratorGet (iter);
      if (cfd -> watchMask & CONTROL_WATCH_READ)
        FD_SET (cfd -> fd, &fds[0]);
      if (cfd -> watchMask & CONTROL_WATCH_WRITE)
        FD_SET (cfd -> fd, &fds[1]);
      if (cfd -> watchMask & CONTROL_WATCH_EXCEPT)
        FD_SET (cfd -> fd, &fds[2]);
      indexiteratorNext (iter);
    }

  /* need to find maximum file descriptor */
  indexiteratorPrevious (iter); /* back up one place */
  {
    struct ControlFileDescriptor *cfd = indexiteratorGet (iter);
    if (cfd != NULL)
      maxFd = cfd -> fd;
  }
  indexiteratorDestroy (iter);

  /* work out time to next poll or timer event */
  tev = pqueuePeekNext (timedEvents);
  if (tev)
    {
      struct timeval now;
      gettimeofday (&now, NULL);
      timeout.tv_sec = tev -> time.tv_sec - now.tv_sec;
      timeout.tv_usec = tev -> time.tv_usec - now.tv_usec;
      if (timeout.tv_usec < 0)
        {
          timeout.tv_sec--;
          timeout.tv_usec += 1000000;
        }
    }
  else
    {
      timeout.tv_sec = 5;
      timeout.tv_usec = 0;
    }

  /* select () */
  if (timeout.tv_sec < 0 || (timeout.tv_sec == 0 && timeout.tv_usec == 0))
    {
      timeout.tv_sec = 0; timeout.tv_usec = 100; 
    }

  retval = select (maxFd + 1, &fds[0], &fds[1], &fds[2], &timeout);

  /* despatch whatever woke us up */
  if (retval > 0)
    {
      int i = 0;
      for (i = 0; i <= maxFd; ++i)
        {
          int mask = 0;
          if (FD_ISSET (i, &fds[0]))
            mask |= CONTROL_WATCH_READ;
          if (FD_ISSET (i, &fds[1]))
            mask |= CONTROL_WATCH_WRITE;
          if (FD_ISSET (i, &fds[2]))
            mask |= CONTROL_WATCH_EXCEPT;
          if (mask)
            {
              struct ControlFileDescriptor *cfd = indexFind (fileDescriptors, &i);
              if (cfd)
                cfd -> callback (i, mask, cfd -> userData);
            }
        }
    }

  /* call whatever timers have expired */
  if ((tev = pqueuePeekNext (timedEvents)) != NULL)
    {
      struct timeval now;
      gettimeofday (&now, NULL);
      while (now.tv_sec > tev -> time.tv_sec
             || (now.tv_sec == tev -> time.tv_sec
                 && now.tv_usec > tev -> time.tv_usec))
        {
          tev = pqueueGetNext (timedEvents);
          tev -> callback (tev -> userData);
          yfree (tev);
          tev = pqueuePeekNext (timedEvents);
          if (tev == NULL)
            break;
        }
    }

  controlPollSignals();

  return controlRunning;
}

void
controlRun (void)
{
  while (controlIteration ())
    ;
}

void
controlShutdownY (void)
{
  controlRunning = 0;
}

void
controlFinalise (void)
{
  indexDestroy (fileDescriptors, controlFileDescriptorsDestructorFunction);
  indexDestroy (signalHandlers, controlSignalHandlerSetDestructorFunction);
  pqueueDestroy (timedEvents, controlTimedEventDestructorFunction);
}

/* arch-tag: 902f698b-690f-44a7-a931-588a000282db
 */
