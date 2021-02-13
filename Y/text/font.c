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

#include <Y/text/font.h>

#include <Y/buffer/painterclass.h>
#include <Y/util/index.h>
#include <Y/util/yutil.h>
#include <Y/main/config.h>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_IMAGE_H
#include FT_GLYPH_H
#include FT_SIZES_H

#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <assert.h>

#include <wchar.h>

#include <iconv.h>

static FT_Library ft_library;
static struct Index *faces;


struct Face
{
  char *familyName;
  char *styleName;
  char *fileName;
  int id;
  int useCount;
  FT_Face face;
};

static void
faceCreate (FT_Face ft_face, const char *filename, int id)
{
  struct Face *face = ymalloc (sizeof (struct Face));
  face -> familyName = ystrdup (ft_face->family_name);
  face -> styleName = ystrdup (ft_face->style_name);
  face -> fileName = ystrdup (filename);
  face -> id = id;
  face -> useCount = 0;
  indexAdd (faces, face);
}

static FT_Face
faceObtain (struct Face *face)
{
  if (face -> useCount == 0)
    {
      int err = FT_New_Face (ft_library, face -> fileName, face -> id, &(face -> face));
      if (err)
        return NULL;
    }
  face -> useCount++;
  return face -> face;
}

static void
faceRelease (struct Face *face)
{
  face -> useCount--;
  if (face -> useCount == 0)
    {
      FT_Done_Face (face -> face);
    }
}

static void
faceDestroy (struct Face *face)
{
  if (face -> useCount != 0)
    {
      FT_Done_Face (face -> face);
    }
  yfree (face -> fileName);
  yfree (face -> styleName);
  yfree (face -> familyName);
  yfree (face);
}

static int
fontFaceKeyFunction (const void *key_v, const void *face_v)
{
  const char * const*key = key_v;
  const struct Face *face = face_v;
  int r;
  r = strcmp (key[0], face->familyName);
  if (r == 0)
    r = strcmp (key[1], face->styleName);
  return r;
}

static int
fontFaceComparisonFunction (const void *face1_v, const void *face2_v)
{
  const struct Face *face1 = face1_v;
  const struct Face *face2 = face2_v;
  int r;
  r = strcmp (face1->familyName, face2->familyName);
  if (r == 0)
    r = strcmp (face1->styleName, face2->styleName);
  return r;
}

static void
loadFontFile (const char *filename)
{
  FT_Face face;
  int i = 0;

  for (;;)
    {
      const char *facesKey[2];

      FT_Error err;
      err = FT_New_Face (ft_library, filename, i, &face );
      if (err)
        return;

      facesKey[0] = face->family_name;
      facesKey[1] = face->style_name;

      if (facesKey[0] == NULL)
        facesKey[0] = "";
      if (facesKey[1] == NULL)
        facesKey[1] = "";

      if (indexFind (faces, facesKey) == NULL)
        faceCreate (face, filename, i);

      if (i >= face -> num_faces)
        {
          FT_Done_Face (face);
          return;
        }

      FT_Done_Face (face);

      ++i;
    }
}

void
fontScanDirectory (const char *path, int recursiveLevels)
{
  DIR *dir;
  struct dirent *entry;
  struct stat entry_stat;
  int pathlen = strlen (path);
  char *name;

  if (pathlen == 0)
    return;

  dir = opendir (path);
  if (dir == NULL)
    {
      Y_ERROR ("Could not open directory '%s': %s", path, strerror (errno));
      return;
    }

  name = ymalloc (pathlen + NAME_MAX + 2);
  memcpy (name, path, pathlen);

  name[pathlen + NAME_MAX + 1] = '\0';
  
  if (name[pathlen-1] != '/')
    {
      pathlen++;
      name[pathlen-1] = '/';
    }

  while (( entry = readdir (dir) ))
    {
      if (entry -> d_name[0] == '.')
        continue;
    
      strncpy (name + pathlen, entry->d_name, NAME_MAX);      
 
      stat (name, &entry_stat);

      if (S_ISDIR(entry_stat.st_mode) && recursiveLevels > 0)
        {
          fontScanDirectory (name, recursiveLevels - 1);
        }
      else if (S_ISREG (entry_stat.st_mode))
        {
          loadFontFile (name);
        } 
    }
  yfree (name);
  closedir (dir);
}

void
fontInitialise (struct Config *serverConfig)
{
  FT_Error err;
  err = FT_Init_FreeType (&ft_library);
  if (err)
    {
      Y_FATAL ("Failed to Initialise FreeType Library: Error code %d", err);
    }

  faces = indexCreate (fontFaceKeyFunction, fontFaceComparisonFunction);

  struct ConfigKeyIterator *i = configGetKeyIterator(serverConfig, "fontpath");
  if (!i)
    /* There is no fontpath group in the config file */
    return;

  /* Variable argument list, so we'll check by hand */
  struct TupleType argsType = {.count = 1, .list = (enum Type[]) {t_list}};
  for (; configKeyIteratorHasValue(i); configKeyIteratorNext(i))
    {
      const char *name = configKeyIteratorName(i);
      struct Tuple *args = configKeyIteratorValue(i, &argsType);
      assert(args != NULL);
      assert(!args->error);

      if (args->count == 0)
        fontScanDirectory(name, 0);
      else if (args->count == 1)
        {
          if (args->list[0].type != t_string)
            Y_WARN("Unrecognised argument for fontpath '%s'; ignoring this entry", name);
          else if (strcmp(args->list[0].string.data, "recursive") == 0)
            fontScanDirectory(name, 255);
          else
            Y_WARN("Unrecognised argument for fontpath '%s'; ignoring this entry", name);
        }
      else
        Y_WARN("Too many arguments for fontpath '%s'; ignoring this entry", name);

      tupleDestroy(args);
    }

  configKeyIteratorDestroy(i);
}

void
fontFinalise ()
{
  indexDestroy (faces, (void (*)(void *))faceDestroy);
  FT_Done_FreeType (ft_library);
}

struct FontGlyph
{
  FT_UInt   index;
  FT_Vector pos;
  FT_Glyph  image;
};

struct FontString
{
  int               string_length;
  int               num_glyphs;
  int               offset;
  int               width; 
  int               advance; 
  wchar_t          *string;
  struct FontGlyph *glyphs;
};

struct Font
{
  struct Face *face;
  FT_Face    ft_face;
  FT_Size    size;
  int        ptSize;
  struct FontString *cachedString;
};

struct Font *
fontCreate (const char *family, const char *style, int ptSize)
{
  const char *key[] = { family, style };
  struct Face *face;
  FT_Face ft_face;
  FT_Size size;
  FT_Error err;
  struct Font *font;

  face = indexFind (faces, key);

  if (face == NULL)
    {
      struct IndexIterator *iter = indexGetStartIterator (faces);
      face = indexiteratorGet (iter);
      indexiteratorDestroy (iter);
      if (face == NULL)
        {
          Y_ERROR ("No fonts found on system!");
          return NULL;
        }
    }

  ft_face = faceObtain (face);

  err = FT_New_Size (ft_face, &size);
  if (err)
    return NULL;

  FT_Activate_Size (size);

  FT_Set_Char_Size (ft_face, 0, ptSize << 6, 0, 0);

  font = ymalloc (sizeof (struct Font));
  font -> face = face;
  font -> ft_face = ft_face;
  font -> size = size;
  font -> ptSize = ptSize;
  font -> cachedString = NULL;
  return font;
}

void
fontGetMetrics (struct Font *self, int *ascent_p, int *descent_p, int *linegap_p)
{
  FT_Activate_Size (self -> size);

  if (ascent_p != NULL)
    *ascent_p = (self -> size -> metrics.ascender + 63) >> 6;
  if (descent_p != NULL)
    *descent_p = ((-self -> size -> metrics.descender) + 63) >> 6;
  if (linegap_p != NULL)
    *linegap_p = (self -> size -> metrics.height + 63) >> 6;
  
}

static struct FontString *
fontstringGenerate (struct Font *self, const wchar_t *string)
{
  struct FontString *fs = ymalloc (sizeof (struct FontString));

  FT_GlyphSlot      slot = self->ft_face->glyph;
  FT_BBox           glyph_bbox;
  FT_Bool           use_kerning;
  FT_UInt           previous;
  FT_Error          error;
  int               pen_x, pen_y, n;
  struct FontGlyph *glyph;

  fs -> string_length = wcslen (string);
  fs -> string = ymalloc (sizeof (wchar_t) * (fs -> string_length + 1));
  fs -> glyphs = ymalloc (sizeof (struct FontGlyph) * fs -> string_length);
  fs -> offset = 32000;
  fs -> width = -32000;
  fs -> advance = 0;

  memcpy (fs -> string, string, sizeof (wchar_t) * (fs -> string_length + 1));

  FT_Activate_Size (self -> size);

  pen_x = 0;   /* start at (0,0) !! */
  pen_y = 0;

  fs -> num_glyphs  = 0;
  use_kerning = FT_HAS_KERNING(self -> ft_face);
  previous    = 0;

  glyph = fs -> glyphs;
  for (n = 0; n < fs -> string_length; n++)
    {
      Y_SILENT ("string[%d]: 0x%08lx '%c'", n, string[n], (char)string[n]);
      glyph->index = FT_Get_Char_Index (self->ft_face, string[n]);

      if ( use_kerning && previous && glyph->index )
        {
          FT_Vector  delta;

          FT_Get_Kerning( self->ft_face, previous, glyph->index,
                          ft_kerning_default, &delta );

          pen_x += delta.x;
        }

      /* store current pen position */
      glyph->pos.x = pen_x;
      glyph->pos.y = pen_y;

      error = FT_Load_Glyph (self->ft_face, glyph->index, FT_LOAD_DEFAULT);
      if (error) continue;

      error = FT_Get_Glyph (self->ft_face->glyph, &glyph->image);
      if (error) continue;

      /* translate the glyph image now.. */
      FT_Glyph_Transform (glyph->image, 0, &glyph->pos);

      /* calculate its bounds */
      FT_Glyph_Get_CBox (fs->glyphs[n].image, ft_glyph_bbox_pixels,
                         &glyph_bbox);
      if (glyph_bbox.xMin < fs -> offset)
        fs -> offset = glyph_bbox.xMin;
      if (glyph_bbox.xMax > fs -> width)
        fs -> width = glyph_bbox.xMax;

      pen_x   += slot->advance.x;
      previous = glyph->index;

      /* move to next glyph */
      glyph++;
    }
  /* count number of glyphs loaded.. */
  fs -> num_glyphs = glyph - (fs->glyphs);

  /* ceiling the new pen position to get the advance */
  fs -> advance = (pen_x + 63) >> 6;

  return fs;
}

static void
fontstringDestroy (struct FontString *self)
{
  int n;
  if (self == NULL)
    return;
  yfree (self -> string);
  for (n=0; n<self->num_glyphs; ++n)
    FT_Done_Glyph (self->glyphs[n].image);
  yfree (self -> glyphs);
  yfree (self);
}

void
fontDestroy (struct Font *self)
{
  if (self == NULL)
    return;
  fontstringDestroy (self -> cachedString);
  FT_Done_Size (self -> size);
  faceRelease (self -> face);
  yfree (self);
}

void
fontMeasureWCString (struct Font *self, const wchar_t *string,
                     int *offset_p, int *width_p, int *advance_p)
{
  struct FontString *fs;

  if (self == NULL)
    return;

  FT_Activate_Size (self -> size);

  if (self -> cachedString == NULL
      || wcscmp (self -> cachedString -> string, string) != 0)
    {
       fontstringDestroy (self -> cachedString);
       self -> cachedString = fontstringGenerate (self, string);
    }

  fs = self -> cachedString;

  if (offset_p != NULL)
    *offset_p = fs -> offset;
  if (width_p != NULL)
    *width_p = fs -> width;
  if (advance_p != NULL)
    *advance_p = fs -> advance;
}

void
fontRenderWCString (struct Font *self, struct Painter *painter,
                    const wchar_t *text, int x, int y)
{
  FT_Vector  start;
  FT_Error   error;
  struct FontString *fs;
  int n;

  if (self == NULL)
    return;
  if (text == NULL)
    return;

  FT_Activate_Size (self -> size);

  if (self -> cachedString == NULL
      || wcscmp (self -> cachedString -> string, text) != 0)
    {
       fontstringDestroy (self -> cachedString);
       self -> cachedString = fontstringGenerate (self, text);
    }

  start.x = x * 64;
  start.y = -y * 64;

  fs = self -> cachedString;

  for ( n = 0; n < fs->num_glyphs; n++ )
    {
      FT_Glyph  image;
      FT_BBox   bbox;

      /* create a copy of the original glyph */
      error = FT_Glyph_Copy( fs->glyphs[n].image, &image );
      if (error) continue;

      /* transform copy (this will also translate it to the correct
       * position */
      FT_Glyph_Transform( image, NULL, &start );

      /* check bounding box, if the transformed glyph image
       * is not in our target surface, we can avoid rendering it */
      FT_Glyph_Get_CBox( image, ft_glyph_bbox_pixels, &bbox );
/*
      at some point check that the glyph needs to be rendered
      using the painter.
      if ( bbox.xMax <= 0 || bbox.xMin >= my_target_width  ||
           bbox.yMax <= 0 || bbox.yMin >= my_target_height )
        continue;
*/

      /* convert glyph image to bitmap (destroy the glyph copy !!) */
      error = FT_Glyph_To_Bitmap( &image,
                                  ft_render_mode_normal,
                                  0,      /* no additional translation */
                                  1 );    /* destroy copy in "image" */
      if (!error)
        {
          FT_BitmapGlyph bitmap = (FT_BitmapGlyph)image;

          painterDrawAlphamap (painter, bitmap->bitmap.buffer,
                               bitmap->left, -bitmap->top,
                               bitmap->bitmap.width,
                               bitmap->bitmap.rows,
                               bitmap->bitmap.pitch);

          FT_Done_Glyph (image);
        }
    }
}

void
fontMeasureString (struct Font *self, const char *text,
                   int *offset_p, int *width_p, int *advance_p)
{
  int l;
  if (text == NULL)
    return;
  l = strlen (text);
  {
    iconv_t cd;
#if __BYTE_ORDER == __LITTLE_ENDIAN
    cd = iconv_open ("UCS-4LE", "UTF-8");
#else
    cd = iconv_open ("UCS-4", "UTF-8");
#endif
    if (cd != (iconv_t)(-1))
      {
        wchar_t wtext [sizeof(wchar_t) * (l + 1)];
        char *wtext_p = (char *)wtext;
        char text_a [l + 1];
        memcpy(text_a, text, l + 1);
        char *text_p = text_a;
        size_t inbytes = (l + 1), outbytes = sizeof (wchar_t) * 2 * (l + 1);
        size_t e;
        e = iconv (cd, &text_p, &inbytes, &wtext_p, &outbytes);
        if (e != (size_t)(-1))
          {
            fontMeasureWCString (self, wtext, offset_p, width_p, advance_p);
          } 
        else
          {
            perror ("fontMeasureString [conv]");
          }
        iconv_close (cd);
      }
    else
      {
        perror ("fontMeasureString [open]");
      }
  }
}

void
fontRenderString (struct Font *self, struct Painter *painter,
                  const char *text, int x, int y)
{
  int l;
  if (text == NULL)
    return;
  l = strlen (text);
  {
    iconv_t cd;
#if __BYTE_ORDER == __LITTLE_ENDIAN
    cd = iconv_open ("UCS-4LE", "UTF-8");
#else
    cd = iconv_open ("UCS-4", "UTF-8");
#endif
    if (cd != (iconv_t)(-1))
      {
        wchar_t wtext [sizeof(wchar_t) * (l + 1)];
        char *wtext_p = (char *)wtext;
        char text_a [l + 1];
        memcpy(text_a, text, l + 1);
        char *text_p = text_a;
        size_t inbytes = (l + 1), outbytes = sizeof (wchar_t) * 2 * (l + 1);
        size_t e;
        e = iconv (cd, &text_p, &inbytes, &wtext_p, &outbytes);
        if (e != (size_t)(-1))
          {
            fontRenderWCString (self, painter, wtext, x, y);
          } 
        else
          {
            perror ("fontRenderString [conv]");
          }
        iconv_close (cd);
      }
    else
      {
        perror ("fontRenderString [open]");
      }
  } 
}

/* arch-tag: 061a278e-7ca5-4568-95b8-e771e90ea647
 */
