/*
 * Keen compiler 
 *
 * symbol.c
 *
 * implement symbol table routines
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

#include <string.h>
#include "xmalloc.h"
#include "hash.h"

#define HASHSIZE 		109

unsigned int
generic_hash (void *s)
{
  unsigned int h = 0;
  unsigned int p = (unsigned int) s;
  for (; p; p = p >> 1) h = (h * 65599) + p;
  return h;
}

generic_hashtable *
create_generic_hashtable (void)
{
  generic_hashtable *t = (generic_hashtable *) xmalloc (sizeof (generic_hashtable));
  t->table = (lentry **) xcalloc (HASHSIZE, sizeof (lentry *));
  return t;
}

void
insert_lh (generic_hashtable * t, void *e)
{
  int index = generic_hash (e) % HASHSIZE;
  lentry *entry = (lentry *) xmalloc(sizeof(lentry));
  entry->next = t->table[index];
  entry->value = e; 
  t->table[index] = entry;
}

void *
lookup_lh (generic_hashtable * t, void *e)
{
  lentry *s;
  int index = generic_hash (e) % HASHSIZE;
  for (s = t->table[index]; s != NULL; s = s->next)
    {
      if (s->value == e) return e;
    }
  return NULL;
}

