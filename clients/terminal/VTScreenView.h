#ifndef YITERM_VTSCREENVIEW_H
#define YITERM_VTSCREENVIEW_H

#include <iterm/screen.h>
#include "terminal.h"

VTScreenView *VTScreenView_new (Terminal *term);
void VTScreenView_exit (VTScreenView *view);
void VTScreenView_destroy (VTScreenView *view);

#endif

/* arch-tag: 9ffa1a00-b43a-47dd-92e1-3938c5e01081
 */
