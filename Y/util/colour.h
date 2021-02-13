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

#ifndef Y_UTIL_COLOUR_H
#define Y_UTIL_COLOUR_H

#include <Y/y.h>
#include <stdint.h>
#include <endian.h>

/* this determines how the dest alpha should be calculated */
enum ColourBlendMode
{
  /* These are the Porter-Duff operators. */       
  COLOUR_BLEND_CLEAR,
  COLOUR_BLEND_SOURCE,      COLOUR_BLEND_DEST,
  COLOUR_BLEND_SOURCE_OVER, COLOUR_BLEND_SOURCE_IN,
  COLOUR_BLEND_SOURCE_OUT,  COLOUR_BLEND_SOURCE_ATOP,
  COLOUR_BLEND_DEST_OVER,   COLOUR_BLEND_DEST_IN,
  COLOUR_BLEND_DEST_OUT,    COLOUR_BLEND_DEST_ATOP,
  COLOUR_BLEND_XOR
};

#if __BYTE_ORDER == __LITTLE_ENDIAN
# define INDEX_A 3
# define INDEX_R 2
# define INDEX_G 1
# define INDEX_B 0
#elif __BYTE_ORDER == __PDP_ENDIAN
# define INDEX_A 2
# define INDEX_R 3
# define INDEX_G 0
# define INDEX_B 1
#else /* big endian */
# define INDEX_A 0
# define INDEX_R 1
# define INDEX_G 2
# define INDEX_B 3
#endif

extern int colourMode1base[];
extern int colourMode1mult[];
extern int colourMode2base[];
extern int colourMode2mult[];

extern void colourBlend (uint32_t *d, uint32_t c1, uint32_t c2,
                         uint32_t modulation, enum ColourBlendMode mode);
extern void colourBlendSourceOver (uint32_t *d, uint32_t c1, uint32_t c2,
                                   uint8_t modulation);

extern inline void
colourBlend (uint32_t *d, uint32_t c1, uint32_t c2,
             uint32_t modulation, enum ColourBlendMode mode)
{
  register uint8_t *d_c  = (uint8_t *)d;
  register uint8_t *c1_c = (uint8_t *)&c1;
  register uint8_t *c2_c = (uint8_t *)&c2;
  register uint32_t alpha1 =  ((uint32_t)c1_c[INDEX_A]);
  register uint32_t alpha2 = (((uint32_t)c2_c[INDEX_A]) * modulation) / 0xFF;
  register uint32_t f1 = alpha1 * (colourMode1base[mode] + colourMode1mult[mode] * alpha2);
  register uint32_t f2 = alpha2 * (colourMode2base[mode] + colourMode2mult[mode] * alpha1);
  register uint32_t newalpha;
  newalpha     = (f1 + f2) > 0xFE01 ? 0xFE01 : (f1 + f2);
  d_c[INDEX_A] = newalpha / 0xFF;
  if (newalpha == 0)
    return;
  d_c[INDEX_R] = (((uint32_t)c1_c[INDEX_R])*f1 + ((uint32_t)c2_c[INDEX_R])*f2) / newalpha;
  d_c[INDEX_G] = (((uint32_t)c1_c[INDEX_G])*f1 + ((uint32_t)c2_c[INDEX_G])*f2) / newalpha;
  d_c[INDEX_B] = (((uint32_t)c1_c[INDEX_B])*f1 + ((uint32_t)c2_c[INDEX_B])*f2) / newalpha;
}

#if 0
extern inline void
colourBlendSourceOver (uint32_t *d, uint32_t c1, uint32_t c2,
                       uint8_t modulation)
{
  *d = c2;
}
#else
extern inline void
colourBlendSourceOver (uint32_t *d, uint32_t c1, uint32_t c2,
                       uint8_t modulation)
{
  register uint8_t *d_c  = (uint8_t *)d;
  register uint8_t *c1_c = (uint8_t *)&c1;
  register uint8_t *c2_c = (uint8_t *)&c2;
  register uint32_t alpha1 =  ((uint32_t)c1_c[INDEX_A]);
  register uint32_t alpha2 = (((uint32_t)c2_c[INDEX_A]) * modulation) / 0xFF;
  register uint32_t f1 = alpha1 * (0xFF - alpha2);
  register uint32_t f2 = alpha2 * 0xFF;
  register uint32_t newalpha;
  newalpha     = (f1 + f2) > 0xFE01 ? 0xFE01 : (f1 + f2);
  d_c[INDEX_A] = newalpha / 0xFF;
  if (newalpha == 0)
    return;
  d_c[INDEX_R] = (  ((uint32_t)c1_c[INDEX_R]) * f1
                  + ((uint32_t)c2_c[INDEX_R]) * f2 ) / newalpha;
  d_c[INDEX_G] = (  ((uint32_t)c1_c[INDEX_G]) * f1
                  + ((uint32_t)c2_c[INDEX_G]) * f2 ) / newalpha;
  d_c[INDEX_B] = (  ((uint32_t)c1_c[INDEX_B]) * f1
                  + ((uint32_t)c2_c[INDEX_B]) * f2 ) / newalpha;
}
#endif

#endif

/* arch-tag: 2b3a1383-9fd4-4797-809a-2b2ae40ccec1
 */
