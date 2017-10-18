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

#ifndef _TYPE_H_
#define _TYPE_H_

/* Types
   We declare here all C types in a machine-independant way. All machine-dependant
   details (size, alignment...) will be defined in each architecture separately. 
   The generic methods on types are declared here */

#include "list.h"

typedef struct xtype type;
  /* to manipulate C types in a machine independant-way. The machine-dependant
     attributes (as alignment or size) are provided by each architecture, and
     must be defined in conformance with declarations in generic header file 
     frame.h */

typedef struct xparameter parameter;
typedef List parameterList;
  /* to manipulate function parameters (in fact, an optional name associated
     with a type) */

typedef struct xmember member;
typedef List memberList;
  /* to manipulate struct/union members */

/* We define here all C types in a machine-independant way. All machine-dependant
   details (size, alignment...) will be defined in each architecture separately. 
   Types are used in keen in 2 ways typically:
   - as symbol attribute (among others) when abstract syntax is parsed,
   - as IR irnode (optional) attribute when IR fragments are generated in translation 
     phase. */

struct xtype
{

  enum
  { 
    t_void,
	/* void type */

    t_char,
    t_int,
    t_short,
    t_long,
	/* integral types, both signed and unsigned. Actual sign is stored in is_unsigned 
           field below */
	/* Note that an enumeration is also of type t_int, and flagged in field is_enum
           below. The values of an enumeration are t_int but not flagged, as they are
           really an integer. */

    t_pointer,
	/* both pointers and arrays. There is no separate type for arrays as they are
           equivalent to (const) pointers in many aspects. Arrays are flagged in is_array field
           below. */

    t_struct,
    t_union,
	/* struct or unions */

    t_func
	/* function type. FIXME today function type is a dummy type and does not hold
           any valuable information, all information on a function are stored directly
           in symbol table binding and attached to a given symbol name. This has to be
           changed when pointers on functions is implemented, and the type should store
           return type and parameters as well */

  } kind;

  int is_complete;
	/* for arrays, struct, union, enumeration and function types, when a declaration has 
           been encountered but no definition yet. Other types are always complete. */

  int is_array; 
	/* array type iif kind == t_pointer && is_array. As for standard pointers, the type of 
           the array elements is stored in field pointer_on below */

  int array_size; 
	/* number of elements in the array, if known (or "undefined" otherwise) */

  int is_unsigned;
	/* indicates sign for integral types */
	
  int is_enum;	
	/* indicates that a t_int is also an enumeration */

  int qualifier;
	/* undefined, or t_const, t_volatile or both */

  union
  {
    struct xtype *pointer_on;
	/* type pointed to by current type, if pointer or array */
	
    memberList members;		
	/* for struct and union. May be empty if type is incomplete */

  } u; 

  int su_size;
	/* total size (in bytes) of struct or union. This size is aligned. */

};

#define undefined	-1
#define t_const		0x01	
#define t_volatile	0x10
	/* values for sign and type qualifiers */


/* Parameter
   A function parameter is a name (optional, as a function declaration
   may be abstract) and a corresponding type */
struct xparameter
{
  char *name;
  type *t;
};

/* Union or struct member 
   A member is made of a name (FIXME: today mandatory) 
   a type, and an offset (in bytes) relative to beginning of
   struct (and always null in case of union) */
struct xmember
{
  char *name;
  type *t;
  int offset;	/* in bytes */
  int padding;	/* in bytes */
};

void init_env (void);
void open_scope();
void close_scope();

member *new_member (char *n, type *t);

/* 
 * Functions to create new types
 */

extern type *Void, *Int, *Uint, *Char, *Uchar, 
            *Long, *Ulong, *Short, *Ushort;
	/* basic types are defined once for all in type.c */

type *type_pointer (type * t);
	/* creates a new pointer on type t */

type *type_array (type * t, int num_elem);
	/* creates a new array with elements of type t and num_elem size array. If array size
           is not known, use 'undefined' instead, and the array will be flagged as incomplete. If
           a size is provided, the array is complete */

type *type_struct (memberList t);
type *type_union (memberList t);
	/* creates a new struct or union type with given members */

type *type_func (void);
	/* creates a new function type */

type *type_dup (type * t);
	/* duplicates a type */

/*
 * Functions on types 
 */

int are_compatible_type (type * a, type * b);
int are_compatible_type_wo_qualifiers (type *a, type *b);

struct xirnode;
int is_lvalue (struct xirnode *i);
	/* returns LVALUE or NOT_LVALUE if the irnode represents a valid lvalue */

int is_modifiable_lvalue (type * t);
	/* returns LVALUE, NOT_LVALUE or NOT_LVALUE_READ_ONLY */

int is_const (type * t);

#define is_arithmetic(t)	((t)->kind == t_int || (t)->kind == t_short || (t)->kind == t_char || (t)->kind == t_long)
#define is_struct_or_union(t)	((t)->kind == t_struct || (t)->kind == t_union)
#define is_array(t)		((t)->kind == t_pointer && (t)->is_array)
#define is_enum(t)		((t)->kind == t_int && (t)->is_enum)
#define is_func(t)		((t)->kind == t_func)
#define is_pointer(t)		((t)->kind == t_pointer)
#define is_scalar(t)		(is_arithmetic(t) || (is_pointer(t) && !is_array(t)))
#define is_complete(t)		((t)->is_complete)
#define get_parameter_type(p)	((p)->t)
#define get_parameter_name(p)	((p)->name)
#define get_array_size(t)	((t)->array_size)
#define points_on(t,k)		((t)->u.pointer_on->kind == (k))

void print_type (type * t);
	/* print type details to debug file */

int get_depth (type *);
type *get_type_depth( type *, int *indexes, int maxindex, int current);

#endif
