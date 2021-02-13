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

#include <Y/util/rectangle.h>
#include <Y/util/yutil.h>

struct Rectangle *
rectangleCreate (uint32_t x, uint32_t y, uint32_t w, uint32_t h)
{
  struct Rectangle *rect = ymalloc (sizeof (struct Rectangle));
  rect -> x = x;
  rect -> y = y;
  rect -> w = w;
  rect -> h = h;
  return rect;
}

struct Rectangle *
rectangleDuplicate (const struct Rectangle *r)
{
  return rectangleCreate (r->x, r->y, r->w, r->h);
}

void
rectangleDestroy (struct Rectangle *self)
{
  yfree (self);
}

bool
rectangleUnion (struct Rectangle *dest,
                const struct Rectangle *src1, const struct Rectangle *src2)
{
  uint32_t x0, y0, x1, y1, w, h, area0, area1;
  x0 = MIN (src1 -> x, src2 -> x);
  y0 = MIN (src1 -> y, src2 -> y);
  x1 = MAX (src1 -> x + src1 -> w, src2 -> x + src2 -> w);   
  y1 = MAX (src1 -> y + src1 -> h, src2 -> y + src2 -> h);
  w = x1 - x0;
  h = y1 - y0;
  area0 = src1 -> w * src1 -> h + src2 -> w * src2 -> h;
  area1 = w * h;
  if (dest)
    {
      dest -> x = x0;
      dest -> y = y0;
      dest -> w = w;
      dest -> h = h;
    }
  return area1 <= area0;
}

bool
rectangleIntersect (struct Rectangle *dest,
                    const struct Rectangle *src1, const struct Rectangle *src2)
{
  uint32_t x = MAX (src1 -> x, src2 -> x);
  uint32_t y = MAX (src1 -> y, src2 -> y);
  uint32_t mw = MIN (src1 -> x + src1 -> w, src2 -> x + src2 -> w);
  uint32_t mh = MIN (src1 -> y + src1 -> h, src2 -> y + src2 -> h);
  if (dest)
    {
      dest -> x = x;
      dest -> y = y;
      dest -> w = (mw > x) ? (mw - x) : (x - mw);
      dest -> h = (mh > y) ? (mh - y) : (y - mh);
    }
  return (mw >= x && mh >= y);
}

void
rectanglelistUnionOverlaps (struct llist *self)
{
  for (struct llist_node *node = llist_head (self);
       node != NULL;
       node = llist_node_next (node))
    {
      struct Rectangle *rect0 = llist_node_data (node);
      struct llist_node *other = llist_node_next (node);
      while (other != NULL)
        {
          struct Rectangle *rect1 = llist_node_data (other);
          if (rectangleIntersect (NULL, rect0, rect1))
            {
              struct llist_node *other_next = llist_node_next (other);
              rectangleUnion (rect0, rect0, rect1);
              llist_delete_node (other);
              yfree (rect1);
              other = other_next; 
            }
          else
            other = llist_node_next (other);
        }
    }
}

struct llist *
rectanglelistIntersectWith (struct llist *src1, struct llist *src2)
{
  struct llist *rc = new_llist ();
  for (struct llist_node *node1 = llist_head (src1);
       node1 != NULL;
       node1 = llist_node_next (node1))
    {
      for (struct llist_node *node2 = llist_head (src2);
           node2 != NULL;
           node2 = llist_node_next (node2))
        {
          struct Rectangle *intersection = ymalloc (sizeof (struct Rectangle));
          struct Rectangle *rect1 = llist_node_data (node1);
          struct Rectangle *rect2 = llist_node_data (node2);
          if (rectangleIntersect (intersection, rect1, rect2))
            {
              llist_add_tail (rc, intersection);
            }
          else
            {
              yfree (intersection);
            }
        }
    }
  return rc;
}


/* arch-tag: 31e15c03-683d-4a2b-b028-ab7010389777
 */
