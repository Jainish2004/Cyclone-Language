/* This file is part of the Cyclone Library.
   Copyright (C) 2000-2001 Greg Morrisett, AT&T

   This library is free software; you can redistribute it and/or it
   under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of
   the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place, Suite
   330, Boston, MA 02111-1307 USA. */

// Originally ported from Objective Caml:

/***********************************************************************)
(*                                                                     *)
(*                           Objective Caml                            *)
(*                                                                     *)
(*   Pierre Weis and Xavier Leroy, projet Cristal, INRIA Rocquencourt  *)
(*                                                                     *)
(*  Copyright 1999 Institut National de Recherche en Informatique et   *)
(*  en Automatique.  All rights reserved.  This file is distributed    *)
(*  under the terms of the GNU Library General Public License.         *)
(*                                                                     *)
(***********************************************************************/


#ifndef _BUFFER_H_
#define _BUFFER_H_
#include <core.h>

namespace Buffer {

  /*** \subsection{\texttt{<buffer.h>}} */

  /*** Defines namespace Buffer, which implements extensible
       character arrays.  It was ported from Objective Caml. */

extern struct t;
typedef struct t @T;
  /** [T] is the type of buffers. */

extern T create(unsigned int n);
  /** [create(n)] returns a fresh buffer, initially empty.  [n] is the
      initial size of an internal character array that holds the
      buffer's contents.  The internal array grows when more than [n]
      character have been stored in the buffer; it shrinks back to the
      initial size when [reset] is called. */
extern mstring_t contents(T);
  /** [contents(b)] heap allocates and returns a string whose contents
      are the contents of buffer [b]. */
extern size_t length(T);
  /** [length(b)] returns the number of characters in buffer [b]. */
extern void clear(T);
  /** [clear(b)] makes [b] have zero characters.  Internal storage is
      not released. */
extern void reset(T);
  /** [reset(b)] sets the number of characters in [b] to zero, and
      sets the internal storage to the initial string.  This means
      that any storage used to grow the buffer since the last create
      or reset can be reclaimed by the garbage collector. */
extern void add_char(T,char);
  /** [add_char(b,c)] appends character [c] to the end of [b]. */
extern void add_substring(T,string_t,int offset,int len);
  /** [add_substring(b,s,ofs,len)] takes [len] characters starting at
      offset [ofs] in string [s] and appends them to the end of [b]. */
extern void add_string(T,string_t);
  /** [add_string(b,s)] appends the string [s] to the end of [b]. */
extern void add_buffer(T buf_dest, T buf_source);
  /** [add_buffer(b1,b2)] appends the current contents of [b2] to the
      end of [b1].  [b2] is not modified. */

}
#endif
