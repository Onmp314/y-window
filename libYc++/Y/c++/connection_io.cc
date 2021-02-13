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

#include <Y/c++/connection.h>

#include <cerrno>
#include <cstring>
#include <iostream>

#include <stdlib.h>
#include <sys/poll.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

#include "thread_support.h"

void
Y::Connection::setNonBlocking(int fd)
{
  /* Set the fd as non-blocking */
  int flags = 0;

  flags = fcntl(fd, F_GETFL, 0);
  if (flags == -1)
    {
      std::cerr << "fcntl() failed to get flags: " << std::strerror(errno) << std::endl;
      abort();
    }

  flags |= O_NONBLOCK;

  if (fcntl(fd, F_SETFL, flags) == -1)
    {
      std::cerr << "fcntl() failed to set flags: " << std::strerror(errno) << std::endl;
      abort();
    }
}

void
Y::Connection::doPoll (int poll_timeout)
{
  /* Now, it's quite possible that somebody might call
   * {register,unregister,change}FD while this is going on.
   *
   * When that happens, {working_,}pollfd_list might get
   * reallocated. To avoid that destroying working_pollfd_list while
   * we're using it, we claim it by making the version stashes in the
   * object NULL - then updateFDList will allocate a new one and we
   * can just throw this one away at the end
   *
   * Note that we don't want to hold the mutex while we're doing the
   * poll - that takes too long.
   */
  struct pollfd *ufds;
  int oldtype;
  lock_mutex(pollfd_list_mutex, oldtype);
  ufds = working_pollfd_list;
  working_pollfd_list = NULL;
  memcpy(ufds, pollfd_list, sizeof(struct pollfd) * pollfds);
  unlock_mutex(oldtype);

  /* Something else is polling right now, so do nothing */
  if (ufds == NULL)
    return;

  /* FIXME: If the next timer to expire will occur before
   * poll_timeout, reduce the timeout
   */

  /* We now own working_pollfd_list and are responsible for disposing of it */
  doPollIO(ufds, poll_timeout);

  lock_mutex(pollfd_list_mutex, oldtype);
  /* Put working_pollfd_list back if we can, otherwise just delete it */
  if (working_pollfd_list == NULL)
    working_pollfd_list = ufds;
  else
    delete[] ufds;
  unlock_mutex(oldtype);
}

void
Y::Connection::doPollIO (struct pollfd *ufds, int timeout)
{
  if (::poll(ufds, pollfds, timeout) == -1)
    {
      if (errno == EINTR)
        return;

      /* Throw an exception here */
      std::cerr << "poll() failed: " << std::strerror(errno) << std::endl;
      abort();
    }

  for (int i = 0; i < pollfds; i++)
    {
      int mask = 0;
      if (ufds[i].revents & POLLIN)
        {
          if (ufds[i].fd == server_fd)
            readServer();
          else
            mask |= Y_LISTEN_READ;
        }
      if (ufds[i].revents & POLLOUT)
        {
          if (ufds[i].fd == server_fd)
            writeServer();
          else
            mask |= Y_LISTEN_WRITE;
        }
      if (ufds[i].revents & POLLERR)
        {
          if (ufds[i].fd == server_fd)
            ;
          else
            mask |= Y_LISTEN_EXCEPT;
        }
      if (mask)
        {
          fdmap::iterator fi = fds.find(ufds[i].fd);
          if (fi == fds.end())
            abort();
          fi->second.callback (fi->second.fd, mask, fi->second.data);
        }
    }
}

void
Y::Connection::readServer ()
{
  char buf[8192];
  ssize_t ret = read(server_fd, buf, sizeof(buf));

  if (ret == -1)
    {
      if (errno == EINTR)
        return;

      /* Throw an exception here */
      std::cerr << "send() failed: " << std::strerror(errno) << std::endl;
      abort();
    }

  if (ret == 0)
    {
      /* Connection closed. Say what? */
      std::cerr << "Connection closed by server" << std::endl;
      abort();
    }

  /* Push all those bytes into the buffer */
  int oldtype;
  lock_mutex(inbound_mutex, oldtype);
  inbound_buffer.append(buf, ret);

  if (debug_io)
    {
      std::cerr << "Read " << ret << " bytes" << std::endl;
      //      std::cerr << "Buffer: '" << inbound_buffer << "'" << std::endl;
    }

  unlock_mutex(oldtype);
}

void
Y::Connection::writeServer ()
{
  size_t remaining;
  ssize_t ret;

  int oldtype;
  lock_mutex(outbound_mutex, oldtype);
  ret = write(server_fd, outbound_buffer.data(), outbound_buffer.length());
  /* Remove any bytes that we wrote */
  if (ret > 0)
    outbound_buffer.erase(0, ret);
  remaining = outbound_buffer.length();
  unlock_mutex(oldtype);

  if (ret == -1)
    {
      if (errno == EINTR)
        return;

      /* Throw an exception here */
      std::cerr << "send() failed: " << std::strerror(errno) << std::endl;
      abort();
    }

  /* If we've got nothing left to write, don't poll for write any more */
  if (outbound_buffer.length() == 0)
    {
      lock_mutex(pollfd_list_mutex, oldtype);
      pollfd_list[pollfds - 1].events &= ~POLLOUT;
      unlock_mutex(oldtype);
    }

  if (debug_io && ret >= 0)
    {
      std::cerr << "Wrote " << ret << " bytes" << std::endl;
      lock_mutex(outbound_mutex, oldtype);
      if (outbound_buffer.length() > 0)
        {
          std::cerr << outbound_buffer.length() << " bytes left in buffer" << std::endl;
        }
      unlock_mutex(oldtype);
    }
}

void
Y::Connection::updateFDList ()
{
  int oldtype;
  lock_mutex(pollfd_list_mutex, oldtype);
  if (pollfd_list)
    delete[] pollfd_list;
  if (working_pollfd_list)
    delete[] working_pollfd_list;

  int oldtype2;
  lock_mutex(fds_mutex, oldtype2);
  pollfds = fds.size() + 1;
  pollfd_list = new struct pollfd[pollfds];
  working_pollfd_list = new struct pollfd[pollfds];

  int i;
  fdmap::iterator fi;
  for (i = 0, fi = fds.begin(); fi != fds.end(); i++, fi++)
    {
      pollfd_list[i].fd = fi->second.fd;
      pollfd_list[i].events = 0;
      pollfd_list[i].revents = 0;

      if (fi->second.mask & Y_LISTEN_READ)
        pollfd_list[i].events |= POLLIN;
      if (fi->second.mask & Y_LISTEN_WRITE)
        pollfd_list[i].events |= POLLOUT;
      if (fi->second.mask & Y_LISTEN_EXCEPT)
        pollfd_list[i].events |= POLLERR;
    }
  unlock_mutex(oldtype2);

  if (server_fd == -1)
    pollfds--;
  else
    {
      pollfd_list[pollfds - 1].fd = server_fd;
      pollfd_list[pollfds - 1].events = POLLIN;
      pollfd_list[pollfds - 1].revents = 0;

      if (outbound_buffer.length() > 0)
        pollfd_list[pollfds - 1].events |= POLLOUT;
    }
  unlock_mutex(oldtype);
}

void
Y::Connection::registerFD (int fd, int mask, void *data, void (*call)(int, int, void *))
{
  bool registered = false;
  int oldtype;
  lock_mutex(fds_mutex, oldtype);
  if (fds.find(fd) != fds.end())
    registered = true;
  else
    fds[fd] = FDHandler(fd, mask, data, call);
  unlock_mutex(oldtype);

  if (registered)
    abort();

  updateFDList();
}

void
Y::Connection::unregisterFD (int fd)
{
  bool erased = false;
  int oldtype;
  lock_mutex(fds_mutex, oldtype);
  fdmap::iterator i = fds.find(fd);
  if (i != fds.end())
    {
      fds.erase(i);
      erased = true;
    }
  unlock_mutex(oldtype);

  if (erased)
    updateFDList();
}

void
Y::Connection::changeFD (int fd, int mask)
{
  fdmap::iterator i;

  int oldtype;
  lock_mutex(fds_mutex, oldtype);
  i = fds.find(fd);
  if (i != fds.end())
    i->second.mask = mask;
  unlock_mutex(oldtype);

  if (i == fds.end())
    abort();

  updateFDList();
}

/* arch-tag: b895bf57-6e1e-487e-a5c1-3edf10fab6c7
 */
