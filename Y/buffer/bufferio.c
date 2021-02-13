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

#include <Y/buffer/rgbabuffer.h>
#include <Y/buffer/bufferio.h>
#include <Y/util/llist.h>
#include <Y/util/yutil.h>

#include <png.h>

#include <string.h>
#include <stdlib.h>

struct FileHandlerInfo
{
  BufferFileHandler callback;
  const char **extensions;
};

static int maximumExtensionLength = 0;
static struct llist *handlers = NULL;
static void bufferioInitialise (void);

struct Buffer *
bufferLoadFromFile (const char *filename, int w, int h)
{
  if (handlers == NULL)
    bufferioInitialise ();

  FILE *file;
  file = fopen (filename, "r");

  if (file == NULL)
    return NULL;

  struct Buffer *buffer = NULL;

  /* 1. Split filename and try the extension
   */

  int i = 0;
  int l = strlen (filename);
  const char *ext = filename + l - 1;

  while (*ext != '.' && i < l && i < maximumExtensionLength)
    {
      ext--;
      i++;
    }

  if (i != 0 && i < l && i < maximumExtensionLength)
    {
      /* ext currently points at the dot, we want the extension */
      ext++;

      for (struct llist_node *node = llist_head (handlers);
           node;
           node = llist_node_next (node)) 
        {
          struct FileHandlerInfo *info = llist_node_data (node);

          for (int e=0; info->extensions[e] != NULL; ++e)
            {
              if (strcmp (ext, info->extensions[e]) == 0)
                {
                  /* try this handler */
                  buffer = info->callback (file, w, h);

                  if (buffer != NULL)
                    return buffer;

                  break;
                }
            }
        }
    }

  /* 2. Try all the loaders in sequence
   */

  for (struct llist_node *node = llist_head (handlers);
       node;
       node = llist_node_next (node)) 
    {
      struct FileHandlerInfo *info = llist_node_data (node);

      /* try this handler */
      buffer = info->callback (file, w, h);

      if (buffer != NULL)
        return buffer;
    }

  /* 3. fail
   */

  return NULL;

}

void
bufferRegisterFileHandler (BufferFileHandler callback, const char *extensions[])
{
  if (handlers == NULL)
    bufferioInitialise ();

  struct FileHandlerInfo *info = ymalloc (sizeof (struct FileHandlerInfo));
  info->callback = callback;
  info->extensions = extensions;
  llist_add_tail (handlers, info);

  for (int i=0; extensions[i] != NULL; ++i)
    {
      int l = strlen (extensions [i]);
      if (l > maximumExtensionLength)
        maximumExtensionLength = l;
    }
}

void
bufferUnregisterFileHandler (BufferFileHandler callback)
{
  if (handlers == NULL)
    return;

  for (struct llist_node *node = llist_head (handlers);
       node;
       node = llist_node_next (node)) 
    {
      struct FileHandlerInfo *info = llist_node_data (node);
      if (info->callback == callback)
        {
          llist_delete_node (node);
          yfree (info);
          return;
        }
    }
}

#define PNG_HEADER_SIZE 8

static struct Buffer *
bufferioLoadPNG (FILE *file, int w, int h)
{
  fpos_t start;
  if (fgetpos (file, &start))
    return NULL;
  
  /* Check if the supplied file is in the correct format...
   */
  char header[PNG_HEADER_SIZE];

  if (fread (header, 1, PNG_HEADER_SIZE, file) != PNG_HEADER_SIZE)
    return NULL;

  fsetpos (file, &start);

  if (png_sig_cmp (header, (png_size_t) 0, PNG_HEADER_SIZE) != 0)
    {
      return NULL;
    }

  png_structp png_ptr = png_create_read_struct (PNG_LIBPNG_VER_STRING,
    (png_voidp) NULL, NULL, NULL);

  if (!png_ptr)
    return NULL;

  png_infop info_ptr = png_create_info_struct (png_ptr);
  if (!info_ptr)
    {
      png_destroy_read_struct (&png_ptr,
                               (png_infopp) NULL, (png_infopp) NULL);
      return NULL;
    }

  png_infop end_info = png_create_info_struct (png_ptr);
  if (!end_info)
    {
      png_destroy_read_struct (&png_ptr, &info_ptr, (png_infopp) NULL);
      return NULL;
    }

  if (setjmp (png_jmpbuf (png_ptr)))
    {
      png_destroy_read_struct (&png_ptr, &info_ptr, &end_info);
      fsetpos (file, &start);
      return NULL;
    }

  png_init_io (png_ptr, file);
  png_read_info (png_ptr, info_ptr);

  unsigned int width = png_get_image_width (png_ptr, info_ptr);
  unsigned int height = png_get_image_height (png_ptr, info_ptr);
  unsigned char colortype = png_get_color_type (png_ptr, info_ptr);

  struct Buffer *buffer = NULL;

  if (colortype == PNG_COLOR_TYPE_RGB)
    {
      /* build an RGBBuffer *TODO*
       */
      abort ();
    }
  else if (colortype == PNG_COLOR_TYPE_RGB_ALPHA)
    {
      /* build an RGBABuffer
       */
      
      /* we use machine endian ARGB not big endian RGBA
       */
#if __BYTE_ORDER == __LITTLE_ENDIAN
      png_set_bgr(png_ptr);
#else
      png_set_swap_alpha(png_ptr); 
#endif

      unsigned int dw = 16;
      unsigned int dh = 16;

      while (dw < width)
        dw *= 2;
      while (dh < height)
        dh *= 2;

      uint32_t *data = ymalloc (dw * dh * sizeof (uint32_t));
      uint32_t *row = data;

      for (unsigned int i=0; i < height; i++)
        {
          png_read_row (png_ptr, (png_bytep)row , NULL);
          row += dw;
        }

      buffer = rgbabufferToBuffer (rgbabufferCreateFromData (width, height,
        dw, dh, data));
    }

  png_read_end (png_ptr, end_info);
  png_destroy_read_struct (&png_ptr, &info_ptr, &end_info);

  return buffer;
}

static const char *bufferioLoadPNGExtensions[] = { "png", NULL };

void
bufferioInitialise (void)
{
  handlers = new_llist ();
  bufferRegisterFileHandler (bufferioLoadPNG, bufferioLoadPNGExtensions);
}

/* arch-tag: 05ffe1be-5b59-47a3-9c88-09890261e263
 */
