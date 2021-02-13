#ifndef YITERM_TERMINAL_H
#define YITERM_TERMINAL_H

#include <Y/c++.h>
#include <sigc++/sigc++.h>

extern "C" {
#include <iterm/core.h>
}

class Terminal : public SigC::Object
{
 private:
  Y::Window *window;
  Y::Console *console;

  int cols, rows;

  VTCore *vtcore;
  TerminalIO *io;
  VTScreenView *view;
  int iofd;

  static void ioDispatch (int, int, void *);
  TerminalIO *ioInit (int, int, bool);

  void keyPress (enum YKeyCode, uint32_t);
  void resize (uint32_t, uint32_t);

  Y::Connection *y;

 public:
  Y::Console *getConsole () { return console; }

  void notifyOSC (int type, char *text, int length);

  Terminal (Y::Connection *y_);
  virtual ~Terminal ();
};

#endif

/* arch-tag: bedd3539-7871-4284-8d36-a0a51d551d80
 */
