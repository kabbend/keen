/*
* Keen compiler 
*
* list.h
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


#ifndef _LIST_H_
#define _LIST_H_

/*
 * List structure
 */
typedef struct _List *List;
typedef struct _ListElem *ListElem;

struct _ListElem
{
  void *object;
  struct _ListElem *next;
  struct _ListElem *prev;
  struct _List *l;		// holding list
};

struct _List
{
  ListElem l;
	/* first element of the list */

  ListElem last;
	/* last element of the list */

  int size;
	/* number of elements in the list */

  ListElem *t;
	/* array of pointers to elements, to ease get_ith() retrieval function */

  int maxt;
	/* maximum number of elements in previous array (0 at startup, is increased when necessary) */

};

/*
 * initialize a new list. Mandatory before any use of the list
 */
List newList ();

/* 
 * traversal functions
 * example:
 * #include "list.h"
 * // ... create list ...
 * ListElem e = get_first( l );
 * while( e ) { 
 *  void *o = object( e );
 *  ...
 *  e = get_next( e );
 * }
 */

#define FOR_EACH(o,l,e) ;ListElem e;for(e=get_first(l),o=(e)?object(e):NULL;e;e=get_next(e),o=(e)?object(e):NULL)
#define FOR_EACH_REWIND(o,l,e) ;ListElem e;for(e=get_last(l),o=(e)?object(e):NULL;e;e=get_prev(e),o=(e)?object(e):NULL)

/* all traversal functions below may return NULL */
#define get_last(li)	((li)->last)
#define get_first(li)	((li)->l)
#define get_next(e)	((e)->next)
#define get_prev(e)	((e)->prev)

/* return i-th element of l. i must be within l boundaries, otherwise get_ith aborts */
ListElem get_ith (List l, int i);

/* return the void* object hold by e */
#define object(e)	((void *)(e)->object)

void **get_ref(ListElem e);

/* copy list l into a new list
 */

List copy (List l);

/*
 * append object o to the List l. 
 * return the updated list 
 */
List add (List l, void *o);
ListElem _add (List l, void *o);
/*
 * insert object o respectively before and after object target in the list
 * return newly inserted ListElem
 */
ListElem add_before (List l, ListElem target, void *o);
ListElem add_after  (List l, ListElem target, void *o);

/*
 * remove the object o from the list, then return o
 * Return NULL if o is not found
 * The object o is not modified nor freed
 */
void *find_n_drop (List l, void *o);

/*
 * remove the element e within l, and return e
 * if e is NULL, do nothing and return NULL
 */
ListElem drop (List l, ListElem e);

/*
 * search object o in the list, and return it if found, otherwise return NULL
 */
void *find (List l, void *o);
ListElem finde (List l, void *o);

/*
 * replace first occurence of object 'before' by object 'after', if 'before' is found, then return 'after'
 * if object 'before' is not found, do nothing and return NULL
 */
void *alter (List l, void *before, void *after);

/*
 * replace the object hold by ListElem e by the 'new' object
 */
void *change (ListElem e, void *new);

/*
 * copy and join a and b: create a new List made of a and b in this order. a and b are not modified
 */
List join (List a, List b);

/*
 * return number of objects in list l
 * List l may be NULL
 */
int size (List l);

/*
 * remove all objects from the List l, and reinitialize it
 * the removed objects are _not_ freed
 * List l may be NULL
 */
void emptyl (List l);

/* 
 * return 1 if list is empty, 0 otherwise 
 * List l may be NULL
 */
int is_empty (List l);

/*
 * free list
 * remove all objects from the List l, and free the List itself
 * List l may be NULL
 */
void freel (List l);

/*
 * free list and objects
 * remove all objects from the List l, free all objects by calling
 * destr function on each one, then free the List itself
 * List l may be NULL
 */
void freelo (List l, void (*destr) (void *));

/* 
 * produce new list with union & intersection of two lists
 */
List list_union (List a, List b);
List list_intersection (List a, List b);
List list_minus (List a, List b);

#endif
