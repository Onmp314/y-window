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

#ifndef Y_BUFFERIO_H
#define Y_BUFFERIO_H

#include <Y/y.h>
#include <Y/buffer/buffer.h>
#include <stdio.h>

/** \brief load image data from a file
 *
 * \param filename the name of the file to load
 * \param w        the width to load the image at, or -1 for default
 * \param h        the height to load the image at, or -1 to aspect
 *                 lock with the width
 */
struct Buffer *bufferLoadFromFile (const char *filename, int w, int h);

typedef struct Buffer *(*BufferFileHandler)(FILE *, int w, int h);

/** \brief register a new file handler
 *
 * \param extensions a null terminated array of the extensions this
 *                   handler can handle, e.g. \code { "jpg", "jpeg", NULL };
 */ 
void bufferRegisterFileHandler (BufferFileHandler, const char *extensions[]);
void bufferUnregisterFileHandler (BufferFileHandler);

#endif

/* arch-tag: 3fc5eedd-b7d9-4271-8b70-88c11bd1cc54
 */
