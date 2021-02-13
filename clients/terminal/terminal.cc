
#include "terminal.h"

#include "VTScreenView.h"

extern "C" {
#include <iterm/unix/ttyio.h>
}

#include <iostream>

using std::cout;
using std::cerr;
using std::endl;
using SigC::bind;
using SigC::slot;

Terminal::Terminal (Y::Connection *y_) : cols(80), rows(24), y(y_)
{
  window = new Y::Window (y, "Terminal");
  window->background.set(0);

  console = new Y::Console (y);
  window -> setChild (console);
  window -> setFocussed (console);

  window -> requestClose.connect (bind (slot (&exit), EXIT_SUCCESS));

  io = ioInit (cols, rows, 1);
  vtcore = VTCore_new (io, cols, rows, 500);

  iofd = TtyTerminalIO_get_associated_fd (io);
  y->registerFD (iofd, Y_LISTEN_READ, this, &Terminal::ioDispatch);

  view = VTScreenView_new (this);
  VTCore_set_screen_view (vtcore, view);
  VTCore_set_exit_callback (vtcore, &VTScreenView_exit);
  console -> keyPress.connect (slot (*this, &Terminal::keyPress));
  console -> resize.connect (slot (*this, &Terminal::resize));

  window -> show ();
}

Terminal::~Terminal ()
{
  VTScreenView_destroy (view);
}

TerminalIO *
Terminal::ioInit (int cols, int rows, bool login)
{
  char defaultShell[] = "/bin/sh";
  char *shell;
  char *loginShell;
  char *program = defaultShell;
  char *argv[] = { program, NULL };
  char env[] = "TERM=xterm";
  putenv (env);
  shell = getenv ("SHELL");
  if (shell != NULL && shell[0] != '\0')
    {
      program = shell;
      argv[0] = shell;
    }

  if (login)
    {
      int length = strlen (program) + 1;
      loginShell = static_cast<char *> (malloc (length + 1));
      loginShell[0] = '-';
      memcpy (loginShell + 1, program, length);
      argv[0] = loginShell;
    }

  return TtyTerminalIO_new (cols, rows, program, argv);
}

void
Terminal::ioDispatch (int fd, int mask, void *data)
{
  Terminal *self = (Terminal *) data;
  self->y->changeFD (fd, 0);
  VTCore_dispatch (self -> vtcore);
  self->y->changeFD (fd, Y_LISTEN_READ);
}

void
Terminal::keyPress (enum YKeyCode key, uint32_t modifiers)
{
  if (key > 0 && key < 256 && key != YK_DELETE)
    {
      char k[] = { (char) key , '\0' };
      VTCore_write (vtcore, k, 1);
    }
  else if (key >= YK_F1 && key <= YK_F15)
    {
      VTCore_send_key (vtcore, VTK_F1 + key - YK_F1);
    }
  else
    {
      switch (key)
        {
          case YK_KP0:
            VTCore_send_key (vtcore, VTK_KP_0); break;
          case YK_KP1: 
            VTCore_send_key (vtcore, VTK_KP_1); break;
          case YK_KP2: 
            VTCore_send_key (vtcore, VTK_KP_2); break;
          case YK_KP3:
            VTCore_send_key (vtcore, VTK_KP_3); break;
          case YK_KP4:
            VTCore_send_key (vtcore, VTK_KP_4); break;
          case YK_KP5:
            VTCore_send_key (vtcore, VTK_KP_5); break;
          case YK_KP6:
            VTCore_send_key (vtcore, VTK_KP_6); break;
          case YK_KP7:
            VTCore_send_key (vtcore, VTK_KP_7); break;
          case YK_KP8:
            VTCore_send_key (vtcore, VTK_KP_8); break;
          case YK_KP9:
            VTCore_send_key (vtcore, VTK_KP_9); break;
          case YK_KP_PERIOD:
            VTCore_send_key (vtcore, VTK_KP_PERIOD); break;
          case YK_KP_MINUS:
            VTCore_send_key (vtcore, VTK_KP_DASH); break;
          case YK_KP_ENTER:
            VTCore_send_key (vtcore, VTK_KP_ENTER); break;
          case YK_UP:
            VTCore_send_key (vtcore, VTK_UP);   break;
          case YK_DOWN:
            VTCore_send_key (vtcore, VTK_DOWN);   break;
          case YK_LEFT:
            VTCore_send_key (vtcore, VTK_LEFT);   break;
          case YK_RIGHT:
            VTCore_send_key (vtcore, VTK_RIGHT);   break;
          case YK_HOME:
            VTCore_send_key (vtcore, VTK_HOME);   break;
          case YK_END:
            VTCore_send_key (vtcore, VTK_END);   break;
          case YK_PAGEUP:
            if (modifiers & YMOD_SHIFT)
              VTCore_scroll_up (vtcore, rows/2);
            else 
              VTCore_send_key (vtcore, VTK_PAGE_UP);
            break;
          case YK_PAGEDOWN:
            if (modifiers & YMOD_SHIFT)
              VTCore_scroll_down (vtcore, rows/2);
            else 
              VTCore_send_key (vtcore, VTK_PAGE_DOWN);
            break;
          case YK_DELETE:
            VTCore_send_key (vtcore, VTK_DELETE);   break;
          case YK_INSERT:
            VTCore_send_key (vtcore, VTK_INSERT);   break;
          default: ;
        }
    }
}

void
Terminal::resize (uint32_t cols, uint32_t rows)
{
  this -> cols = cols;
  this -> rows = rows;
  VTCore_set_screen_size (vtcore, cols, rows);
}

void
Terminal::notifyOSC (int type, char *data, int length)
{
}

int
main (int argc, char **argv)
{
  Y::Connection y;
  Terminal *t = new Terminal (&y);
  y.run();
  delete t;
}

/* arch-tag: 451c0b5d-331b-40b8-8d0b-a907e35e4dce
 */
