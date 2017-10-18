/*
* Keen compiler 
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

#ifndef _TRANSLATE_H_
#define _TRANSLATE_H_

#include "grammar.h"
#include "type.h"
#include "ir.h"
#include "temp.h"
#include "frame.h"
#include "list.h"

//
// FRAGMENT STRUCTURE
//
typedef struct xfragment
{
  enum
  {
    frag_func,			/* function */
    frag_var,			/* global variable */
    frag_data,			/* static data */
    frag_info                   /* miscellaneous */
  } kind;
  union
  {
    struct
    {
      frame *f;
      irnode *body;
    } func;
    struct
    {
      accessor *access;
      irnode *body;
    } var;
    struct
    {
      accessor *access;
      irnode *body;
    } data;
    struct
    {
      irnode *body;
    } info;
  } u;
} fragment;

typedef List fragmentList;

fragmentList translate (n_unit * u);

void print_fragmentList (fragmentList f);

void parse_declaration_for_typedefs (n_declaration * p);

#endif
