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

#include <Y/y.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <getopt.h>
#include <locale.h>

#include <Y/setup.h>

#include <Y/screen/screen.h>
#include <Y/main/control.h>
#include <Y/main/config.h>
#include <Y/input/ykb.h>
#include <Y/input/keymap.h>
#include <Y/object/class.h>
#include <Y/object/object.h>
#include <Y/text/font.h>
#include <Y/util/yutil.h>
#include <Y/util/dbuffer.h>
#include <Y/message/client.h>

#include <Y/widget/window.h>
#include <Y/widget/menu.h>

static char *configFile = NULL;
static struct Config *serverConfig = NULL;

static void
final (void)
{
  clientFinalise ();
  moduleFinalise ();
  ykbFinalise ();
  keymapFinalise ();
  classFinalise ();
  fontFinalise ();
  screenFinalise ();
  controlFinalise ();
  configDestroy(serverConfig);
  yfree(configFile);
  dbuffer_cleanup();
  ylogClose ();
}

enum long_option_enum
  {
    lo_config,
    lo_no_detach,
    lo_emit_pid,
    lo_version,
    lo_license,
    lo_help,
    lo_last
  };

static struct option const longopts[] =
  {
    [lo_config] = {"config", required_argument, NULL, 0},
    [lo_no_detach] = {"no-detach", no_argument, NULL, 0},
    [lo_emit_pid] = {"emit-pid", no_argument, NULL, 0},
    [lo_version] = {"version", no_argument, NULL, 0},
    [lo_help] = {"help", no_argument, NULL, 0},
    [lo_last] = {NULL, no_argument, NULL, 0}
  };

static inline void
show_usage(void)
{
  fprintf(stderr, "Usage: Y [OPTION ...]\n");
  fprintf(stderr, "  --config file     use a different config file\n");
  fprintf(stderr, "  --no-detach       do not detach from teh controlling terminal\n");
  fprintf(stderr, "  --emit-pid        when detaching, emit the pid of the server on stdout\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "  --version         display version information and exit\n");
  fprintf(stderr, "  --help            display this message and exit\n");
  exit(0);
}

static inline void
show_version(void)
{
  fprintf(stderr, "Y %s\n", VERSION);
  exit(0);
}

int
main (int argc, char **argv)
{
  int i;
  bool detaching = true;
  bool emit_pid = false;

  pid_t pid;
  int pipedes[2];
  int status[2];

  /* We must run all of these in POSIX mode */
  setlocale(LC_CTYPE, "POSIX");
  setlocale(LC_COLLATE, "POSIX");
  setlocale(LC_MONETARY, "POSIX");
  setlocale(LC_NUMERIC, "POSIX");

  /* These can be localised */
  setlocale(LC_MESSAGES, "");
  setlocale(LC_TIME, "");

  configFile = ystrdup(getenv("YCONFIGFILE"));
  if (!configFile)
    {
      static char configSuffix[] = "/server.conf";
      configFile = ymalloc (strlen (yConfigDir) + strlen(configSuffix) + 1);
      strcpy (configFile, yConfigDir);
      strcat (configFile, configSuffix);
    }

  ylogInit ();

  int optc, longopt_index;

  while ((optc = getopt_long(argc, argv, "?", longopts, &longopt_index)) != -1)
    {
      /* Note that show_usage() does not return */
      if (optc == '?')
        show_usage();
      if (optc != 0)
        abort();
      switch(longopt_index)
        {
        case lo_config:
          if (optarg)
            configFile = optarg;
          break;
        case lo_no_detach:
          detaching = false;
          break;
        case lo_emit_pid:
          emit_pid = true;
          break;
        case lo_version:
          show_version();
          break;
        case lo_help:
          show_usage();
          break;
        default:
          abort();
        }
    }

  if (detaching)
    {
      if (pipe (pipedes) < 0)
        {
          fprintf (stderr, "Could not open pipe: %s", strerror (errno));
          return EXIT_FAILURE;
        }
    
      /* Detach from the console */
      pid = fork ();
      if (pid == -1)
        {
          Y_FATAL ("Fork failed: %s", strerror (errno));
          return EXIT_FAILURE;
        }
      else if (pid != 0)
        {
          /* Close the other end of the pipe, so that we hear about it
           * if the child dies
           */
          close (pipedes[1]);

          /* wait for child to finish initialising... */
          int returns[2];

          ssize_t ret = read (pipedes[0], returns, sizeof (int) * 2);
          if (ret > 0)
            {
              if (returns[1] > 0)
                {
                  char buffer[returns[1]+1];
                  read (pipedes[0], buffer, returns[1]);
                  buffer[returns[1]] = '\0';
                  fprintf (stderr, "Y: %s\n", buffer);
                }
              else
                {
                  if (emit_pid)
                    fprintf(stdout, "%lu\n", (long unsigned int)pid);
                }
              return (returns[0]);
            }
          else
            {
              /* lost contact with the child? */
              if (ret == 0 || errno == EPIPE)
                {
                  fprintf (stderr, "Y: server exited before starting up\n");
                }
              else
                {
                  perror ("Y: Error whilst detaching");
                }

              return EXIT_FAILURE;  
            }
        }

      close (pipedes[0]);
      fclose (stdin);
      setsid ();
    }

  serverConfig = configRead (configFile);;

  controlInitialise ();
  screenInitialise ();
  fontInitialise (serverConfig);
  classInitialise ();
  clientInitialise ();
  keymapInitialise (serverConfig);
  ykbInitialise (serverConfig);
  moduleInitialise (serverConfig);

  atexit (final);

  srandom (time (NULL) ^ getpid ());

  if (detaching)
    {
      /* signal to the parent process that all is well */
      status[0] = EXIT_SUCCESS;
      status[1] = 0;
      write (pipedes[1], status, sizeof (int) * 2);

      close (pipedes[1]);

      fclose (stdout);
      fclose (stderr);

      /* we are now fully detached... */
    }

  controlRun ();

  return EXIT_SUCCESS;
}


/* arch-tag: cfcca386-0d95-4369-8d50-a2c785bf1231
 */
