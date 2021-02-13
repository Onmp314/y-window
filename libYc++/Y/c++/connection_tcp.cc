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
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>

void
Y::Connection::tcpInitialise (const char *display)
{
  int r;
  struct sockaddr_in sockaddr;
  struct hostent *host;
  char buffer[strlen (display)];
  char *portspec;

  sockaddr.sin_family = AF_INET;

  strcpy (buffer, display);
  portspec = strchr (buffer, ':');
  if (portspec == NULL)
    {
      sockaddr.sin_port = htons (8900);
    }
  else
    {
      *(portspec++) = '\0';
      sockaddr.sin_port = htons (strtoul (portspec, NULL, 10));
    }

  host = gethostbyname (buffer);
  if (host == NULL)
    {
      switch (h_errno)
        {
        case HOST_NOT_FOUND:
          std::cerr << "Host not found" << std::endl;
          abort();
        case NO_ADDRESS:
          std::cerr << "Couldn't find address for host" << std::endl;
          abort();
        default:
          std::cerr << "Error resolving host name" << std::endl;
          abort();
        }
    }
  sockaddr.sin_addr.s_addr = *((in_addr_t *) host -> h_addr_list[0]);

  int fd = socket (PF_INET, SOCK_STREAM, 0);
  r = connect (fd, (struct sockaddr *)&sockaddr, sizeof (sockaddr));

  if (r != 0)
    {
      std::cerr << "Failed to connect to Y Server via TCP: " << std::strerror(errno) << std::endl;
      abort();
    }

  server_fd = fd;
}

/* arch-tag: 22faa8e2-d988-4f1b-980b-42a0f84c6d9a
 */
