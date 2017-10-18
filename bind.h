/*
* Keen compiler 
*
* bind.h
*
* define bindings in symbol table
*
* Copyright (C) 2003 Karim Ben Djedidia <kabend@free.fr> 
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  
* USA
*
*/

#ifndef _BIND_H_
#define _BIND_H_

#include "type.h"
#include "frame.h"
#include "grammar.h"		/* for position structure */
#include "list.h"
#include "temp.h"               /* for label */

/* BIND DEFINITIONS */

typedef struct xbind
{
  enum
  { b_type, b_var, b_func, b_goto, b_const } kind;
  int is_parameter;
  type *t;
  union
  {
    /* variable */
    struct
    {
      accessor *access;
      int is_extern;
    } v;
    /* function */
    struct
    {
      type *return_t;
      parameterList arg_list;
      int ellipsis;
      int is_defined;
      int is_implicit;
      position first_decl_position;
      frame *f_frame;
    } f;
    /* goto label */
    struct 
    {
      label *internal_label;
      int is_defined; 
      int first_use_line;
    } l;
    /* integral constant for enumeration */
    struct 
    {
      int value;
    } c;
  } u;
} bind;

bind *new_bind_type (type * t);
bind *new_bind_var (type * t, int parameter, accessor * a, int is_extern);
bind *new_bind_func (type * return_t, parameterList arg_list, int ellipsis,
		     int is_defined, int is_implicit, frame * f,
		     position fdp);
bind *new_bind_goto ( label *internal_label, int is_defined );
bind *new_bind_const (int value);
void free_bind (bind * b);

#endif
