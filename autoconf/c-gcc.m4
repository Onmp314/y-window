dnl This file is copyright (C) Andrew Suffield
dnl                                  <asuffield@freenode.net>
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

dnl Stolen from dpkg

dnl C_GCC_TRY_FLAGS(<warnings>, [gcc])

dnl If the second argument is present and is "gcc", then the warnings
dnl will only be used for gcc, not g++

AC_DEFUN([C_GCC_TRY_FLAGS],[
 AC_MSG_CHECKING([gcc warning flag(s) $1])
 if test "${GCC-no}" = yes
 then

   AC_LANG_PUSH([C])
   oldcflags="${CFLAGS-}"
   CFLAGS="${CFLAGS-} ${CWARNS} $1 -Werror"
   AC_TRY_COMPILE([
#include <string.h>
#include <stdio.h>

const char *a = "a";

const char *foo(void);
inline const char *foo(void) {return a;}

int main(void);
],[
    strcmp(foo(),"b"); fprintf(stdout,"test ok\n");
],[
   CWARNS="${CWARNS}$1 "
   gcc_ok=yes
   if test x$2 = xgcc; then
     AC_MSG_RESULT(ok)
   fi
],[
   gcc_ok=no
   if test x$2 = xgcc; then
     AC_MSG_RESULT(no)
   fi
])
   CFLAGS="${oldcflags}"
   AC_LANG_POP([C])

dnl ***************************************************
dnl Now the same thing for g++

   if test x$2 != xgcc; then

   AC_LANG_PUSH([C++])
   oldcxxflags="${CXXFLAGS-}"
   CXXFLAGS="${CXXFLAGS-} ${CXXWARNS} $1 -Werror"
   AC_TRY_COMPILE([
#include <string.h>
#include <stdio.h>

const char *a = "a";

const char *foo(void);
inline const char *foo(void) {return a;}

int main(void);
],[
    strcmp(foo(),"b"); fprintf(stdout,"test ok\n");
],[
   CXXWARNS="${CXXWARNS}$1 "
   if test x$gcc_ok = xyes; then
     AC_MSG_RESULT(ok)
   else
     AC_MSG_RESULT(g++ only)
   fi
],[
   if test x$gcc_ok = xyes; then
     AC_MSG_RESULT(gcc only)
   else
     AC_MSG_RESULT(no)
   fi
])
   CXXFLAGS="${oldcxxflags}"
   AC_LANG_POP([C++])
   fi

 else
  AC_MSG_RESULT(not using GCC, skipping)
 fi
])

AC_DEFUN([C_GCC_LD_TRY_FLAGS],[
 AC_MSG_CHECKING([gcc linker warning flag(s) $1])
 if test "${GCC-no}" = yes
 then
   oldldflags="${LDFLAGS-}"
   LDFLAGS="${LDFLAGS-} ${LDWARNS} -Wl,$1 -Wl,--fatal-warnings"
   AC_TRY_LINK([
#include <string.h>
#include <stdio.h>

const char *a = "a";

const char *foo(void);
inline const char *foo(void) {return a;}

int main(void);
],[
    strcmp(foo(),"b"); fprintf(stdout,"test ok\n");
],[
   LDWARNS="${LDWARNS}-Wl,$1 "
   AC_MSG_RESULT(ok)
],[
   AC_MSG_RESULT(no)
])
   LDFLAGS="${oldldflags}"
 else
  AC_MSG_RESULT(not using GCC, skipping)
 fi
])
