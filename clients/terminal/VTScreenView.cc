
#include "VTScreenView.h"
#include "terminal.h"

extern "C" {
#include <iterm/screen.h>
}

#include <cstdlib>

#include <iostream>
#include <string>

using std::string;
using std::cout;
using std::cerr;
using std::endl;

static void
VTScreenView_draw_text (VTScreenView *view, int col, int row,
                        char *mbstring, int length, int width)
{
//  cerr << "VTScreenView_draw_text " << col << " " << row << " `" <<
//          string (mbstring, length)
//       << "' [" << width << "]" << endl; 
  Y::Console *console = ((Terminal *) view -> object) -> getConsole ();
  console -> drawText (col, row, string (mbstring, length), width);
}

static void
VTScreenView_clear_rect(VTScreenView *view,int s_col,int s_row,
                        int e_col, int e_row)
{
//  cerr << "VTScreenView_clear_rect " << s_col << " " << s_row << " "
//                                     << e_col << " " << e_row << endl; 
  Y::Console *console = ((Terminal *) view -> object) -> getConsole ();
  console -> clearRect (s_col, s_row, e_col, e_row);
}

static void
VTScreenView_set_rendition(VTScreenView *view, int bold, int blink,
                           int inverse, int underline, int foreground,
                           int background, char charset)
{
//  cerr << "VTScreenView_set_rendition " << bold << " " << blink << " "
//       << inverse << " " << underline << " " << foreground << " "
//       << background <<  endl; 
  Y::Console *console = ((Terminal *) view -> object) -> getConsole ();
  console -> setRendition (bold, blink, inverse, underline, foreground,
                           background, charset);
}

static void
VTScreenView_swap_video(VTScreenView *view)
{
  cerr << "VTScreenView_swap_video" << endl; 
  Y::Console *console = ((Terminal *) view -> object) -> getConsole ();
  console -> swapVideo ();
}

static void
VTScreenView_ring(VTScreenView *view)
{
//  cerr << "VTScreenView_ring" << endl; 
  Y::Console *console = ((Terminal *) view -> object) -> getConsole ();
  console -> ring ();
}

static void
VTScreenView_resize_request(VTScreenView *view, int cols, int rows)
{
  cerr << "VTScreenView_resize_request " << cols << " " << rows << endl; 
}

static void
VTScreenView_update_cursor_position(VTScreenView *view, int cols, int rows)
{
//  cerr << "VTScreenView_update_cursor_position" << endl; 
  Y::Console *console = ((Terminal *) view -> object) -> getConsole ();
  console -> updateCursorPos (cols, rows);
}

static void
VTScreenView_notify_osc(VTScreenView *view, int type, char *string, int length)
{
//  cerr << "VTScreenView_notify_osc" << endl; 
  Terminal *term = (Terminal *) view -> object;
  term -> notifyOSC (type, string, length);
}

static void
VTScreenView_update_scrollbar(VTScreenView *view, int max, int top, int shown)
{
//  cerr << "VTScreenView_update_scrollbar" << endl; 
}

static void
VTScreenView_scroll_view(VTScreenView *view, int dest_row, int src_row,
                         int num_line)
{
//  cerr << "VTScreenView_scroll_view   dest:" << dest_row << " src:" << src_row <<
//                     " num:" << num_line << endl; 
  Y::Console *console = ((Terminal *) view -> object) -> getConsole ();
  console -> scrollView (dest_row, src_row, num_line);
}


VTScreenView *
VTScreenView_new (Terminal *term)
{
  VTScreenView *view = new VTScreenView;
  VTScreenView_init (view);
  view -> object               = term;
  view -> draw_text            = VTScreenView_draw_text;
  view -> clear_rect           = VTScreenView_clear_rect;
  view -> set_rendition        = VTScreenView_set_rendition;
  view -> notify_osc           = VTScreenView_notify_osc;
  view -> ring                 = VTScreenView_ring;
  view -> swap_video           = VTScreenView_swap_video;
  view -> resize_request         = VTScreenView_resize_request; 
  view -> update_cursor_position = VTScreenView_update_cursor_position;
  /* view -> update_scrollbar       = VTScreenView_update_scrollbar; */
  view -> scroll_view            = VTScreenView_scroll_view; 
  return view;
}

void
VTScreenView_exit (VTScreenView *view)
{
  exit (EXIT_SUCCESS);
}
void
VTScreenView_destroy (VTScreenView *view)
{
  if(view != NULL)
    {
      delete view;
    }
}

/* arch-tag: 8b8c37a8-5719-40d4-96db-bf05ad6e906e
 */
