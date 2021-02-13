dnl This file is copyright (C) Andrew Suffield <asuffield@debian.org>
dnl
dnl This program is free software; you can redistribute it and/or modify
dnl it under the terms of the GNU General Public License as published by
dnl the Free Software Foundation; either version 2 of the License, or
dnl (at your option) any later version.
dnl
dnl This program is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl GNU General Public License for more details.
dnl
dnl You should have received a copy of the GNU General Public License
dnl along with this program; if not, write to the Free Software
dnl Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA

AC_DEFUN([BINUTILS_CXX_VERSION_SCRIPT],[
AC_MSG_CHECKING([binutils support for C++ in version scripts])

AC_LANG_PUSH([C++])

oldldflags="${LDFLAGS-}"
LDFLAGS="${LDFLAGS-} -Wl,--version-script=$srcdir/autoconf/test.Versions"


AC_LINK_IFELSE([
namespace Test
{
  int foo(void) {return 0;}
};

int main(void)
{
  return Test::foo();
}

],[
AC_MSG_RESULT([yes])
$1
],[
$2
AC_MSG_RESULT([no])
])

LDFLAGS="${oldldflags}"

AC_LANG_POP([C++])
])
