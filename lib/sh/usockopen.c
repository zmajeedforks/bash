/*
 * usockopen.c -- functions to make unix domain socket connections
 *
 * Zartaj Majeed
 */

/* Copyright (C) 2019-2025 Free Software Foundation, Inc.

   This file is part of GNU Bash, the Bourne Again SHell.

   Bash is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Bash is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Bash.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <config.h>

#if defined (HAVE_UNISTD_H)
#  include <unistd.h>
#endif

#include <stdio.h>
#include <sys/types.h>

#if defined (HAVE_SYS_SOCKET_H)
#  include <sys/socket.h>
#endif

#if defined (HAVE_SYS_UN_H)
#  include <sys/un.h>
#endif

#include <bashansi.h>
#include <bashintl.h>

#include <errno.h>

#ifndef errno
extern int errno;
#endif

static int _usockopen __P((char *, int));

/*
 * Open Unix domain socket connection
 * Returns the connected socket or -1 on error.
 */
static int
_usockopen (path, typ)
     char *path;
     int typ;
{
  int s, e;
  struct sockaddr_un addr;
  int socktyp = typ == 's'? SOCK_STREAM: SOCK_DGRAM;
  size_t len = strlen(path);

  if (len >= sizeof addr.sun_path)
    {
      internal_error ("path too long \"%s\"", path);
      errno = EINVAL;
      return -1;
    }

  if ((s = socket (AF_UNIX, socktyp, 0)) < 0)
    {
      sys_error ("unix socket");
      return -1;
    }

  memset (&addr, 0, sizeof addr);
  addr.sun_family = AF_UNIX;
  strncpy (addr.sun_path, path, sizeof(addr.sun_path) - 1);

  if (connect (s, &addr, sizeof addr) < 0)
    {
      e = errno;
      sys_error ("unix connect");
      close (s);
      errno = e;
      return -1;
    }
  return s;
}

/*
 * Open Unix domain socket connection to a path like /dev/unixstream/tmp/xyz.sock or /dev/unixdgram/tmp/xyz.sock
 * Returns the connected socket or -1 on error.
 */
int
usockopen (path)
     char *path;
{
  char *s;
  int fd;

  s = strchr (path+9, '/');
  /* relative path starts with "./" or "../" */
  if (s[1] == '.' && s[2] == '/' ||
      s[1] == '.' && s[2] == '.' && s[3] == '/')
    {
      ++s;
    }
  fd = _usockopen (s, path[9]);

  return fd;
}

