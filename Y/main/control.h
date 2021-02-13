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

#ifndef Y_CONTROL_CONTROL_H
#define Y_CONTROL_CONTROL_H

#include <Y/y.h>

void controlInitialise (void);

#define CONTROL_WATCH_READ   (1<<1)
#define CONTROL_WATCH_WRITE  (1<<2)
#define CONTROL_WATCH_EXCEPT (1<<3)

void controlRegisterFileDescriptor (int fd, int watchMask, void *userData,
                   void (*callback)(int fd, int causeMask, void *userData));

void controlChangeFileDescriptorMask (int fd, int watchMask);

void controlUnregisterFileDescriptor (int fd);

int  controlTimerDelay (int minIntervalSeconds, int minIntervalMilliseconds,
                        void *userData, void (*callback)(void *userData));
void controlCancelTimerDelay (int id);

void controlRegisterSignalHandler (int signo, void *userData,
                                   void (*callback)(int signo, void *userData));
void controlUnregisterSignalHandler (int signo, void *userData,
                                     void (*callback)(int signo, void *userData));

/* Run the server ;-) */
void controlRun (void);

void controlShutdownY (void);

void controlFinalise (void);

#endif /* Y_CONTROL_CONTROL_H */

/* arch-tag: 56be15d9-2520-4b99-a82d-cbd922cf216a
 */
