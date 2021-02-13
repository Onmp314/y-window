AC_DEFUN([LIBITERM_CHECK],
[
oldlibs="$LIBS"
LIBS="-literm $LIBS"

AC_LANG_PUSH([C++])
AC_MSG_CHECKING([for libiterm])
AC_LINK_IFELSE(
[
extern "C" {
#include <iterm/core.h>
#include <iterm/unix/ttyio.h>
}

int main (void)
{
  VTCore_destroy(VTCore_new (NULL, 80, 24, 500));
  return 0;
}
],
[
LIBITERM_LIBS="-literm"
AC_SUBST(LIBITERM_LIBS)
have_libiterm=yes
AC_MSG_RESULT(yes)
],
[
have_libiterm=no
AC_MSG_RESULT(no)
])
AC_LANG_POP([C++])

LIBS="$oldlibs"

])
