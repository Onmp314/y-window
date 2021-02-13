dnl
dnl LIBSDL CHECK script

AC_DEFUN([LIBSDL_CHECK],
[dnl
dnl Get the cflags and libraries from the sdl-config program
dnl
AC_ARG_WITH(libsdl-prefix,AC_HELP_STRING([--with-libsdl-prefix],[Prefix where libSDL is installed (optional)]),
            libsdl_prefix="$withval", libsdl_prefix="")
AC_ARG_WITH(libsdl-exec-prefix,AC_HELP_STRING([--with-libsdl-exec-prefix],[Exec prefix where libSDL is installed (optional)]),
            libsdl_exec_prefix="$withval", libsdl_exec_prefix="")
AC_ARG_ENABLE(libsdltest,AC_HELP_STRING([--disable-libsdltest],[Do not try to compile and run a test LIBSDL program]),
              , enable_libsdltest=yes)

  if test x$libsdl_exec_prefix != x ; then
    libsdl_args="$libsdl_args --exec-prefix=$libsdl_exec_prefix"
    if test x${SDL_CONFIG+set} != xset ; then
       SDL_CONFIG=$libsdl_exec_prefix/bin/sdl-config
    fi
  fi
  if test x$libsdl_prefix != x ; then
    libsdl_args="$libsdl_args --prefix=$libsdl_prefix"
    if test x${SDL_CONFIG+set} != xset ; then
       SDL_CONFIG=$libsdl_prefix/bin/sdl-config
    fi
  fi

  AC_PATH_PROG(SDL_CONFIG, sdl-config, no)
  min_libsdl_version=ifelse([$1], ,1.0.0,$1)
  AC_MSG_CHECKING(for libSDL - version >= $min_libsdl_version)
  no_libsdl=""
  if test "$SDL_CONFIG" = "no" ; then
    no_libsdl=yes
  else
    LIBSDL_CPPFLAGS=`$SDL_CONFIG $libsdlconf_args --cflags`
    LIBSDL_LIBS=`$SDL_CONFIG $libsdlconf_args --libs`
    libsdl_major_version=`$SDL_CONFIG $libsdlconf_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\1/'`
    libsdl_minor_version=`$SDL_CONFIG $libsdlconf_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\2/'`
    libsdl_micro_version=`$SDL_CONFIG $libsdlconf_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\3/'`

    if test "x$enable_libsdltest" = "xyes" ; then
      ac_save_CPPFLAGS="$CPPFLAGS"
      ac_save_LIBS="$LIBS"
      CPPFLAGS="$CPPFLAGS $LIBSDL_CPPFLAGS"
      LIBS="$LIBS $LIBSDL_LIBS"

dnl
dnl Now check if the installed LIBSDL is sufficiently new.
dnl
    rm -f conf.libsdltest
    AC_TRY_RUN([
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "SDL.h"

static char*
my_strdup (const char *str)
{
  char *new_str;
  
  if (str)
    {
      new_str = (char *)malloc ((strlen (str) + 1) * sizeof(char));
      strcpy (new_str, str);
    }
  else
    new_str = NULL;
  
  return new_str;
}

int main (void)
{
  int major, minor, micro;
  char *tmp_version;

  { FILE *fp = fopen("conf.libsdltest", "a"); if ( fp ) fclose(fp); }

  /* HP/UX 9 (%@#!) writes to sscanf strings */
  tmp_version = my_strdup("$min_libsdl_version");
  if (sscanf(tmp_version, "%d.%d.%d", &major, &minor, &micro) != 3)
    {
      printf("%s, bad version string\n", "$min_libsdl_version");
      exit(1);
    }

  if (($libsdl_major_version > major) ||
     (($libsdl_major_version == major) && ($libsdl_minor_version > minor)) ||
     (($libsdl_major_version == major) && ($libsdl_minor_version == minor) &&
      ($libsdl_micro_version >= micro)))
    {
      return 0;
    }
  else
    {
      printf ("\n*** 'sdl-config --version' returned %d.%d.%d, but the minimum version\n",
              $libsdl_major_version, $libsdl_minor_version, $libsdl_micro_version);
      printf ("*** of LIBSDL required is %d.%d.%d.  If sdl-config is correct, then is is\n",
              major, minor, micro);
      printf ("*** best to upgrade to the required version.\n");
      return 1;
    }
}

],, no_libsdl=yes,[echo $ac_n "cross compiling; assumed OK... $ac_c"])
      CPPFLAGS="$ac_save_CPPFLAGS"
      LIBS="$ac_save_LIBS"
    fi
  fi
  if test "x$no_libsdl" = x ; then
    AC_MSG_RESULT(yes)
    AC_SUBST(HAVE_LIBSDL, 1)
    ifelse([$2], ,:, [$2])
  else
    AC_MSG_RESULT(no)
    if test "$SDL_CONFIG" = "no" ; then
      echo "*** The sdl-config program installed by libSDL could not be found"
      echo "*** If LIBSDL was installed in PREFIX, make sure PREFIX/bin is in"
      echo "*** your path, or set the SDL_CONFIG environment variable to the"
      echo "*** full path to sdl-config."
    else
      if test -f conf.libsdltest ; then
        :
      else
        echo "*** Could not run LIBSDL test program, checking why..."
          CPPFLAGS="$CFLAGS $FREETYPE_CPPFLAGS"
          LIBS="$LIBS $FREETYPE_LIBS"
          AC_TRY_LINK([
#include <stdio.h>
#include "SDL.h"
],      [ return 0; ],
        [ echo "*** The test program compiled, but did not run. This usually means"
          echo "*** that the run-time linker is not finding libSDL or finding the wrong"
          echo "*** version of libSDL. If it is not finding libSDL, you'll need to set your"
          echo "*** LD_LIBRARY_PATH environment variable, or edit /etc/ld.so.conf to point"
          echo "*** to the installed location  Also, make sure you have run ldconfig if that"
          echo "*** is required on your system"
          echo "***"
          echo "*** If you have an old version installed, it is best to remove it, although"
          echo "*** you may also be able to get things to work by modifying LD_LIBRARY_PATH"],
        [ echo "*** The test program failed to compile or link. See the file config.log for the"
          echo "*** exact error that occured. This usually means libSDL was incorrectly installed"
          echo "*** or that you have moved libSDL since it was installed. In the latter case, you"
          echo "*** may want to edit the sdl-config script: $SDL_CONFIG" ])
          CPPFLAGS="$ac_save_CPPFLAGS"
          LIBS="$ac_save_LIBS"
       fi
     fi
     LIBSDL_CPPFLAGS=""
     LIBSDL_LIBS=""
     AC_SUBST(HAVE_LIBSDL, 0)
     ifelse([$3], , :, [$3])
  fi
  AC_SUBST(LIBSDL_CPPFLAGS)
  AC_SUBST(LIBSDL_LIBS)
  rm -f conf.libsdltest
])
