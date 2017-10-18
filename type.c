/*
 * Keen compiler 
 *
 * type.c
 *
 * implement type handling routines
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
#include <string.h>
#include <stdlib.h>
#include "type.h"
#include "list.h"
#include "symbol.h"
#include "bind.h"
#include "error.h"
#include "ir.h"

/* Types
   We define here all C types in a machine-independant way. All machine-dependant
   details (size, alignment...) will be defined in each architecture separately. 
   Types are used in keen in 2 ways typically:
   - as symbol attribute (among others) when abstract syntax is parsed,
   - as IR irnode (optional) attribute when IR fragments are generated in translation 
     phase. */

/* basic types
   These C types are stable and can be defined once for all. They can be compared
   directly with == operator */
type _void   = { t_void,  0, 0, undefined, undefined, 0, undefined, NULL, undefined }, *Void = &_void;
type _char   = { t_char,  1, 0, 0, 0, 0, undefined, NULL, undefined }, *Char = &_char;
type _uchar  = { t_char,  1, 0, 1, 0, 0, undefined, NULL, undefined }, *Uchar = &_uchar;
type _short  = { t_short, 1, 0, 0, 0, 0, undefined, NULL, undefined }, *Short = &_short;
type _ushort = { t_short, 1, 0, 1, 0, 0, undefined, NULL, undefined }, *Ushort = &_ushort;
type _int    = { t_int,   1, 0, 0, 0, 0, undefined, NULL, undefined }, *Int = &_int;
type _uint   = { t_int,   1, 0, 1, 0, 0, undefined, NULL, undefined }, *Uint = &_uint;
type _long   = { t_long,  1, 0, 0, 0, 0, undefined, NULL, undefined }, *Long = &_long;
type _ulong  = { t_long,  1, 0, 1, 0, 0, undefined, NULL, undefined }, *Ulong = &_ulong;


member *
new_member (char *n, type *t)
{
  member *new = (member *) xmalloc(sizeof(member));
  new->name = n;
  new->t = t;
  return new;
}

/* return 1 if given type is const-qualified, 0 otherwise
 * a type is const-qualified if:
 * - it has explicit const-qualifier,
 * - it is an array (and, in keen internals, belongs to previous category as
 *   an array is implicitly const-qualified)
 * - it is a struct or union with at least one const-qualified member, recursively,
 *   with the notable exception that an array (multidimensional or not) within a structure 
 *   is not considered as const, but only its element type is tested for this purpose.
 */
int
is_const (type * t)
{
  // check each member of a struct or union
  if (is_struct_or_union(t))
    {
	  member *m;
	  FOR_EACH (m, t->u.members, e)
	    {
		  type *mtype = m->t;
		  while (is_array(mtype)) mtype = mtype->u.pointer_on;
		  if (is_pointer(mtype))
			{
			  if (mtype->qualifier != undefined && (mtype->qualifier & t_const)) return 1;
				/* we do not use function is_const() recursively on pointers
 				   to avoid potential infinite loop if pointer on same structure
				   (directly or indirectly) */
			}
		  else if (is_const(mtype)) return 1;
	    }
	  // fallthrough, as the struct itself could be const-qualified
    }

  // check the qualifier for the type itself
  return (t->qualifier != undefined && (t->qualifier & t_const));
}


/* Return LVALUE if the type represents a valid l-value, or NOT_LVALUE otherwise
 * A valid l-value represents an object or is a derivative type of an object, and is not
 * of type void
 */
int
is_lvalue (struct xirnode *i)
{
  if (GET_TYPE(i)->kind == t_void) return NOT_LVALUE;
  if (!GET_SYMBOL(i)) return NOT_LVALUE;
  return LVALUE;
}

/* Return LVALUE if the type represents a valid modifiable l-value, NOT_LVALUE if the type is not
 * an l-value, and NOT_LVALUE_READ_ONLY if the type is read only.
 * a modifiable l-value must be an l-lvalue, complete, not a function or array, and not
 * const-qualified.
 */
int
is_modifiable_lvalue (type * t)
{
  if ((!t->is_complete) || (t->kind == t_func) || (is_array(t))) return NOT_LVALUE;
  if (is_const (t)) return NOT_LVALUE_READ_ONLY;
  return LVALUE;
}


/* compares 2 types, with or without qualifiers (const/volatile)
   aggregate types are compared recursively 
   FIXME for enumerations, we should check tag names and values as well */
int
_are_compatible_type (type * a, type * b, int with_qualifiers, List compatible_type_struct_list )
{

  if (!compatible_type_struct_list) compatible_type_struct_list = newList();
	/* during this compatibility check, we may encounter recursive (directl or indirect)
  	   loop within structures. To avoid infinite loop, we store already encountered struct
	   types and break if we encounter the same structure type twice */

  if (!a || !b) return 0;
  
  if (a->kind != b->kind) return 0;
  
  if (with_qualifiers && (a->qualifier != b->qualifier)) return 0;
  
  if (a->is_unsigned != b->is_unsigned) return 0;
  
  if (  is_array(a) && a->array_size != undefined
     && is_array(b) && b->array_size != undefined
	 && a->array_size != b->array_size) return 0;
		/* if defined, both array size shall be the same */
		
  if (is_pointer(a)) return _are_compatible_type (a->u.pointer_on, b->u.pointer_on,with_qualifiers, compatible_type_struct_list);
		/* this covers arrays as well */
		
  if (is_struct_or_union(a))
    {
	 
	 if (size(a->u.members) != size(b->u.members)) return 0;
			/* they are obviously not compatible */
			
	 if (find( compatible_type_struct_list, a) || find (compatible_type_struct_list, b)) return 1;
			/* we avoid here infinite loop */

	 // new structures, we store them to avoid future infinite loop
	 add (compatible_type_struct_list, a);
	 add (compatible_type_struct_list, b);
	 
	 if (size(a->u.members) != 0)
		{
		// For complete struct and unions, we compare pairs of members in same order, 
		// and for same type and name (if named)
		member *ma, *mb;
		ListElem ea = get_first(a->u.members);
		ListElem eb = get_first(b->u.members);
		while (ea)
			{
			ma = object (ea); mb = object (eb);
		  
			if ((ma->name && !mb->name) || (!ma->name && mb->name)) return 0;
				/* one member is named and the other is not */
				
			if (ma->name && mb->name && strcmp(ma->name,mb->name)!=0) return 0;
				/* they don't have same name */
				
			if (!_are_compatible_type(ma->t, mb->t, with_qualifiers, compatible_type_struct_list)) return 0;
				/* they are not compatible */
				
			ea = get_next(ea);
			eb = get_next(eb);
			}
		}
	}
	
  return 1;
}

int 
are_compatible_type (type *a, type *b)
{
 return _are_compatible_type(a,b,1,NULL);
}

int 
are_compatible_type_wo_qualifiers (type *a, type *b)
{
 return _are_compatible_type(a,b,0,NULL);
}

/* return the composite type of two types a and b
 * Both types a and b are supposed to be compatible. Do not forget
 * to test this prior to call this function as this could crash at
 * runtime because we suppose here that both types have same
 * structure.
 * FIXME today quite dummy function because no support for function
 * types
 */
type *
get_composite_type ( type *a, type *b )
{
  type *composite;
/* FIXME  
  if (is_array(a))
    {
	  if (a->array_size == b->array_size)
	  if (a->array_size == undefined && b->array_size != undefined)
	}
  
  // otherwise, nothing particular to do...
  composite = type_dup ( a );
*/ 
  return composite;
}


type *
type_dup (type * ty)
{
  type *t = (type *) xmalloc (sizeof (type));
  memcpy (t, ty, sizeof (type));
  return t;
}

type *
type_func (void)
{
  type *t = (type *) xcalloc (1, sizeof (type));
  t->kind = t_func;
  t->is_unsigned = undefined;
  t->is_complete = 0;
  t->is_enum = 0;
  t->qualifier = undefined;
  return t;
}

type *
type_pointer (type * ty)
{
  type *t = (type *) xcalloc (1, sizeof (type));
  t->kind = t_pointer;
  t->u.pointer_on = ty;
  t->is_unsigned = undefined;
  t->is_complete = 1;
  t->is_enum = 0;
  t->qualifier = undefined;
  return t;
}

type *
type_struct (memberList ty)
{
  type *t = (type *) xcalloc (1, sizeof (type));
  t->kind = t_struct;
  t->u.members = ty;
  t->is_unsigned = undefined;
  t->is_complete = 0; // a priori
  t->su_size = undefined; // a priori
  t->is_enum = 0;
  t->qualifier = undefined;
  return t;
}

type *
type_union (memberList ty)
{
  type *t = (type *) xcalloc (1, sizeof (type));
  t->kind = t_union;
  t->u.members = ty;
  t->is_unsigned = undefined;
  t->is_complete = 0; // a priori
  t->su_size = undefined; // a priori
  t->is_enum = 0;
  t->qualifier = undefined;
  return t;
}

type *
type_array (type * ty, int num_elem)
{
  type *t = type_pointer (ty);
  t->is_array = 1;
  t->qualifier = t_const;
  t->array_size = num_elem;
  t->is_enum = 0;
  t->is_complete = (num_elem != undefined);
  return t;
}

/* typenames below are used in debug routines */
char *type_basename[] = {
  "void",
  "char",
  "int",
  "short",
  "long",
  "pointer",
  "struct",
  "union",
  "function"
};

/* print details of type ty to debug file */
void
print_type (type * ty)
{
  keen_debug("(");
  if (ty->qualifier != undefined && ty->qualifier & t_const) keen_debug ("const ");
  if (ty->qualifier != undefined && ty->qualifier & t_volatile) keen_debug ("volatile ");
  if (ty->qualifier == undefined) keen_debug ("unqualified ");
  if (is_arithmetic(ty)) 
    { 
	  if (ty->is_unsigned == 1) keen_debug ("unsigned "); 
	    else if (ty->is_unsigned == 0) keen_debug ("signed "); 
		  else keen_debug ("undefined-sign "); 
	}
  if (ty->is_complete == 1) keen_debug ("complete "); else keen_debug ("uncomplete ");	
  if (ty->kind == t_pointer)
    {
      if (ty->is_array) 
	{
	  keen_debug("array of: "); 
	}
      else keen_debug ("pointer on: ");
      print_type (ty->u.pointer_on);
    }
  else if (is_struct_or_union(ty))
    {
	  if (ty->kind == t_struct) keen_debug ("struct ");
	  if (ty->kind == t_union) keen_debug ("union ");
	  if (0) // FIXME we do not print members to avoid potential infinite loop
		// we should improve this...
	  //if (is_complete(ty))
	    {
		  keen_debug ("of members below --\n");
		  member *m;
		  FOR_EACH (m, ty->u.members, e)
		    {
			  print_type (m->t);
			}
		  keen_debug ("-- end of struct or union\n");
		}
	}
  else if (is_enum(ty))
    {
	  keen_debug ("enumeration ");
	  return;
    }
  else keen_debug (type_basename[ty->kind]);
  keen_debug(")\n");
  // FIXME for functions
}


int
get_depth (type *t)
{
 if (is_array(t)) {
     return 1 + get_depth (t->u.pointer_on);
   }
 else if (is_pointer(t)) {
     return 1;
   }
 else if (is_struct_or_union(t))
   {
     int d = 1;
     member *m;
     FOR_EACH (m,t->u.members,e)
       	{
	  int i = get_depth (m->t); 
	  if (i + 1 > d) d = i + 1;
	}
     return d;
   }
 else return 0; 
}

type *
get_type_depth (type *t, int *indexes, int maxindex, int currentindex)
{
 if (currentindex > maxindex) return t;

 if (is_pointer(t)) {
     /* array or pointer */
     return get_type_depth (t->u.pointer_on, indexes , maxindex, currentindex + 1 );
   }
 else if (is_struct_or_union(t))
   {
     return get_type_depth( ((member *) object(get_ith(t->u.members,indexes[currentindex])))->t, indexes, maxindex, currentindex + 1 );
   }
 return t;
 // should not get here
 abort();
}


