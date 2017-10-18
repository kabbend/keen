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


#ifndef _HASH_H_
#define _HASH_H_

typedef struct lentry_
{
  void *value;
  struct lentry_ *next; 
} lentry;

typedef struct
{
  lentry **table;
} generic_hashtable;

generic_hashtable *create_generic_hashtable (void);
void insert_gh (generic_hashtable * t, void *e);
void *lookup_gh (generic_hashtable * t, void *e);

#endif
