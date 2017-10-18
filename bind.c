/*
* Keen compiler 
*
* bind.c
*
* implement bindings in symbol table
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

#include <stdlib.h>
#include "bind.h"

bind *
new_bind_const (int value)
{
  bind *b = (bind *) xmalloc (sizeof (bind));
  b->kind = b_const;

  type *t = type_dup( Int );
  t->qualifier = t_const;

  b->t = t;
  b->u.c.value = value;

  return b;
}

bind *
new_bind_goto (label *internal_label, int is_defined)
{
  bind *b = (bind *) xmalloc (sizeof (bind));
  b->kind = b_goto;
  b->u.l.internal_label = internal_label;
  b->u.l.is_defined = is_defined;
  return b;
}

bind *
new_bind_type (type * t)
{
  bind *b = (bind *) xmalloc (sizeof (bind));
  b->kind = b_type;
  b->t = t;
  return b;
}

bind *
new_bind_var (type * t, int parameter, accessor * access, int is_extern)
{
  bind *b = (bind *) xmalloc (sizeof (bind));
  b->kind = b_var;
  b->t = t;
  b->is_parameter = parameter;
  b->u.v.access = access;
  b->u.v.is_extern = is_extern;
  return b;
}

bind *
new_bind_func (type * t, parameterList l, int ellipsis, int is_defined,
	       int is_implicit, frame * f, position fdp)
{
  bind *b = (bind *) xmalloc (sizeof (bind));
  b->kind = b_func;
  b->t = type_func ();		/* dummy type */
  b->u.f.return_t = t;
  b->u.f.arg_list = l;
  b->u.f.ellipsis = ellipsis;
  b->u.f.is_defined = is_defined;
  b->u.f.is_implicit = is_implicit;
  b->u.f.f_frame = f;
  b->u.f.first_decl_position = fdp;
  return b;
}

void
free_bind (bind * b)
{
  if (b)
    free (b);
}
