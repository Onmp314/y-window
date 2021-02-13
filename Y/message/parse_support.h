/************************************************************************
 *   Copyright (C) Andrew Suffield <asuffield@debian.org>
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

#ifndef Y_MESSAGE_PARSE_SUPPORT_H
#define Y_MESSAGE_PARSE_SUPPORT_H

/* These macros copy data into a buffer and increment the pointer */

#define ADD_SCALAR(P, S) do {memcpy((P), &(S), sizeof(S)); (P) += sizeof(S);} while(0)
#define ADD_DATA(P, S, L) do {size_t l = (L); memcpy((P), (S), l); (P) += l;} while(0)

/* These macros are used to safely parse a the buffer represented by
 * p/l; after extracting a value, p is incremented to point after it
 * and l is decremented by the amount of data removed, so l is always
 * the length remaning after p.
 *
 * Before:
 * "\1\0\0\0\0\0\0\0"
 *  ^p
 * l == 8
 *
 * uint32_t x;
 * GET_SCALAR(x)
 *
 * After:
 * "\1\0\0\0\0\0\0\0"
 *          ^p
 * l == 4
 * x == 1
 */

/* These return true on success and false if there wasn't enough data */

/* Extract a scalar from the buffer */
#define GET_SCALAR_(P, L, S) ({memcpy(&(S), (P), sizeof(S)); (P) += sizeof(S); (L) -= sizeof(S); true;})
#define GET_SCALAR(P, L, S) (((L) < sizeof(S)) ? false : GET_SCALAR_((P), (L), (S)))
/* Skip a chunk of data; presumably we already recorded a pointer to it */
#define SKIP_DATA_(P, L, S) ({(P) += (S); (L) -= (S); true;})
#define SKIP_DATA(P, L, S) (((L) < (S)) ? false : SKIP_DATA_((P), (L), (S)))

#endif

/* arch-tag: 1dea7994-03fb-471c-ae63-fe6d65ad03d3
 */
