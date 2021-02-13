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

#include <Y/util/colour.h>

int colourMode1base[] = { 0,   0,0xFF,0xFF,0,   0,0xFF,0xFF,0,0xFF,   0,0xFF };
int colourMode1mult[] = { 0,   0,   0,  -1,0,   0,  -1,   0,1,  -1,   1,  -1 };
int colourMode2base[] = { 0,0xFF,   0,0xFF,0,0xFF,   0,0xFF,0,   0,0xFF,0xFF };
int colourMode2mult[] = { 0,   0,   0,   0,1,  -1,   1,  -1,0,   0,  -1,  -1 };

void
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

void
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


/* arch-tag: 5ad5a73d-7fc7-4d57-8f89-2b6b30386cf4
 */
