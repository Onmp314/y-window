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
#include <iostream>

#include <sys/socket.h>
#include <sys/un.h>

void
Y::Connection::unixInitialise (const char *display)
{
  int r;
  struct sockaddr_un sockaddr;
  sockaddr.sun_family = AF_UNIX;
  strncpy (sockaddr.sun_path, display, 100);
  int fd = socket (PF_UNIX, SOCK_STREAM, 0);
  r = connect (fd, (struct sockaddr *)&sockaddr, sizeof (sockaddr));

  if (r != 0)
    {
      std::cerr << "Failed to connect to Y Server via Unix domain socket: " << std::strerror(errno) << std::endl;
      abort();
    }

  server_fd = fd;
}

/* arch-tag: c5315195-82c1-4c90-ba8c-d3dd4ec0bcad
 */
