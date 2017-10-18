/*
 * Keen compiler 
 *
 * list.c
 *
 * implement some low level functions to handle lists of objects
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


#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "list.h"
#include "hash.h"

void **
get_ref( ListElem e)
{
 return (void **) &(e->object);
}

List
newList ()
{
  return (List) xcalloc (1, sizeof (struct _List));
}

ListElem
get_ith (List l, int i)
{
  assert (i >= 0 && i < size (l));
  return l->t[i];
}

List
copy (List l)
{
  return join (l, newList ());
}

ListElem
_add (List l, void *o)
{
  assert (l);
  ListElem e = (ListElem) xcalloc (1, sizeof (struct _ListElem));
  e->object = o;
  e->next = NULL;
  e->l = l;
  if (size (l) == 0)
    {
      l->l = e;
      e->prev = NULL;
    }
  else
    {
      e->prev = l->last;
      l->last->next = e;
    }
  l->last = e;
  l->size++;
  /* create or enlarge table if needed */
  if ( l->maxt < l->size )
    {
      l->maxt += 20;
      l->t = (ListElem *) xrealloc( l->t, l->maxt * sizeof(ListElem)); 
    }
  /* insert sequentially into table */
  l->t[l->size-1] = e; 
  return e;
}

List
add (List l, void *o)
{
  assert (l);
  ListElem e = (ListElem) xcalloc (1, sizeof (struct _ListElem));
  e->object = o;
  e->next = NULL;
  e->l = l;
  if (size (l) == 0)
    {
      l->l = e;
      e->prev = NULL;
    }
  else
    {
      e->prev = l->last;
      l->last->next = e;
    }
  l->last = e;
  l->size++;
  /* create or enlarge table if needed */
  if ( l->maxt < l->size )
    {
      l->maxt += 20;
      l->t = (ListElem *) xrealloc( l->t, l->maxt * sizeof(ListElem)); 
    }
  /* insert sequentially into table */
  l->t[l->size-1] = e; 
  return l;
}

ListElem
add_before (List x, ListElem l, void *o)
{

 // if no position specified, add to list
 if (!l) 
   {
     return get_last( add(x,o) ); 
   }

 ListElem e = (ListElem) xcalloc (1, sizeof (struct _ListElem));
 e->object = o;
 e->l = x;
 if (l->prev) 
   {
     e->next = l->prev->next;
     e->prev = l->prev;
     l->prev->next = e;
     e->next->prev = e;  
   }
 else
   {
     e->next = x->l; 
     e->prev = NULL;
     if (x->l) x->l->prev = e;
     x->l = e;
   }
 x->size++;
 return e;
}

ListElem
add_after (List x, ListElem l, void *o)
{

 // if no position specified, add to list
 if (!l) 
   {
     return get_last( add(x,o) ); 
   }

 ListElem e = (ListElem) xcalloc (1, sizeof (struct _ListElem));
 e->object = o;
 e->l = x;
 if (l->next)
   {
     l->next->prev = e;
     e->next = l->next;
     e->prev = l;
     l->next = e;
   }
 else 
   {
     e->next = NULL;
     e->prev = x->last;
     if (x->last) x->last->next = e;
     x->last = e;
   } 
 x->size++;
 return e;
}

void *
find_n_drop (List l, void *o)
{
  ListElem current;
  assert (l);
  if (size (l) == 0)
    return NULL;
  for (current = l->l; current != NULL; current = current->next)
    {
      if (current->object == o)
	{
	  // if last object, change List last field
	  if (current == l->last)
	    {
	      l->last = current->prev;
	    }
	  else
	    {
	      current->next->prev = current->prev;
	    }
	  // if first object, change l field
	  if (current == l->l)
	    {
	      l->l = current->next;
	    }
	  else
	    {
	      current->prev->next = current->next;
	    }
	  // remove the listElem from list
	  void *obj = current->object;
	  free (current);
	  l->size--;
	  return obj;
	}
    }
  return NULL;
}

ListElem
drop (List l, ListElem e)
{
  if (is_empty (l))
    return NULL;
  if (e->prev)
    {
      e->prev->next = e->next;
    }
  else
    {
      l->l = e->next;
    }
  if (e->next)
    {
      e->next->prev = e->prev;
    }
  else
    {
      l->last = e->prev;
    }
  l->size--;
  return e;
}

void
emptyl (List l)
{
  ListElem p, next;
  if (!l)
    return;
  p = l->l;
  while (p != NULL)
    {
      next = p->next;
      free (p);
      p = next;
    }
  l->l = NULL;
  l->last = NULL;
  l->size = 0;
}

int
size (List l)
{
  if (!l)
    return 0;
  return l->size;
}

int
is_empty (List l)
{
  return (size (l) == 0);
}

void
freel (List l)
{
  emptyl (l);
  free (l);
}

/*
void
freelo (List l, void (*destr) (void *))
{
  ListElem p, next;
  if (!l)
    return;
  p = l->l;
  while (p != NULL)
    {
      next = p->next;
      destr (p->object);
      free (p);
      p = next;
    }
  free (l);
}
*/

void *
find (List l, void *o)
{
  ListElem e;
  for (e = l->l; e != NULL; e = e->next)
    {
      if (e->object == o)
	return o;
    }
  return NULL;
}

ListElem
finde (List l, void *o)
{
  ListElem e;
  assert (l);
  for (e = l->l; e != NULL; e = e->next)
    {
      if (e->object == o)
	return e;
    }
  return NULL;
}

List
join (List a, List b)
{
  assert (a);
  assert (b);
  List r = newList ();
  ListElem e;
  for (e = a->l; e != NULL; e = e->next)
    add (r, e->object);
  for (e = b->l; e != NULL; e = e->next)
    add (r, e->object);
  return r;
}

void *
alter (List l, void *o1, void *o2)
{
  ListElem e;
  assert (l);
  for (e = l->l; e != NULL; e = e->next)
    {
      if (e->object == o1)
	{
	  e->object = o2;
	  return o2;
	}
    }
  return NULL;
}

void *
change (ListElem e, void *value)
{
  assert (e);
  e->object = value;
}

List
list_union (List a, List b)
{
  List result = newList ();
  ListElem e = get_first (a);
  while (e)
    {
      if (!find (b, e->object)) add (result, e->object);
      e = get_next (e);
    }
  e = get_first (b);
  while (e)
    {
      add (result, e->object);
      e = get_next (e);
    }
  return result;
}

List
list_intersection (List a, List b)
{
  List result = newList ();
  ListElem e = get_first (a);
  while (e)
    {
      if (find (b, e->object) && !find(result,e->object))
	add (result, e->object);
      e = get_next (e);
    }
  return result;
}

List
list_minus (List a, List b)
{
  List result = newList ();
  ListElem e = get_first (a);
  while (e)
    {
      if (!find (b, e->object))
	add (result, e->object);
      e = get_next (e);
    }
  return result;
}
