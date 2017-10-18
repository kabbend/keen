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

#ifndef _IR_H_
#define _IR_H_

#include "type.h"
#include "symbol.h"

/* declare IR classes, subclasses and nodes */
#undef  DEF_NODE0
#undef  DEF_NODE1
#undef  DEF_NODE2
#undef  DEF_NODE3
#undef  DEF_NODE4
#undef  DEF_NODE5
#undef  DEF_NODE6
#undef  DEF_NODE7
#define DEF_NODE0( a, b, c, d )
#define DEF_NODE1( a, b, c, d )
#define DEF_NODE2( a, b, c, d )
#define DEF_NODE3( a, b, c, d )
#define DEF_NODE4( a, b, c, d )
#define DEF_NODE5( a, b, c, d )
#define DEF_NODE6( a, b, c, d )
#define DEF_NODE7( a, b, c, d )
#undef  DEF_SUBCLASS
#define DEF_SUBCLASS( a , b )
#define DEF_CLASS( c ) c ,
enum ir_class
{
#include "ir.def"
};

/* declare IR subclasses */
#undef  DEF_NODE0
#undef  DEF_NODE1
#undef  DEF_NODE2
#undef  DEF_NODE3
#undef  DEF_NODE4
#undef  DEF_NODE5
#undef  DEF_NODE6
#undef  DEF_NODE7
#define DEF_NODE0( a, b, c, d )
#define DEF_NODE1( a, b, c, d )
#define DEF_NODE2( a, b, c, d )
#define DEF_NODE3( a, b, c, d )
#define DEF_NODE4( a, b, c, d )
#define DEF_NODE5( a, b, c, d )
#define DEF_NODE6( a, b, c, d )
#define DEF_NODE7( a, b, c, d )
#undef  DEF_CLASS
#define DEF_CLASS( a )
#undef  DEF_SUBCLASS
#define DEF_SUBCLASS( s, c ) s ,
enum ir_subclass
{
#include "ir.def"
};

/* IR node types */
#undef  DEF_CLASS
#define DEF_CLASS( a )
#undef  DEF_SUBCLASS
#define DEF_SUBCLASS( s, c )
#undef  DEF_NODE0
#undef  DEF_NODE1
#undef  DEF_NODE2
#undef  DEF_NODE3
#undef  DEF_NODE4
#undef  DEF_NODE5
#undef  DEF_NODE6
#undef  DEF_NODE7
#define DEF_NODE0( t, c, s, arg ) n_##t ,
#define DEF_NODE1( t, c, s, arg ) n_##t ,
#define DEF_NODE2( t, c, s, arg ) n_##t ,
#define DEF_NODE3( t, c, s, arg ) n_##t ,
#define DEF_NODE4( t, c, s, arg ) n_##t ,
#define DEF_NODE5( t, c, s, arg ) n_##t ,
#define DEF_NODE6( t, c, s, arg ) n_##t ,
#define DEF_NODE7( t, c, s, arg ) n_##t ,
enum ir_nodetype
{
#include "ir.def"
};

/* store class, subclass and arg-desc for each node type */
typedef struct xirdef
{
  enum ir_nodetype t;
  enum ir_class c;
  enum ir_subclass s;
  char *def;
} irdef;

#define NODETYPE(n)		((n)->kind)
#define GET_MODE(n)		((n)->mode)
#define GET_TYPE(n)		((n)->t)
#define GET_SYMBOL(n)		((n)->s)
#define GET_LVALUE(n)		((n)->lvalue)
#define GET_CLASS(n)		((n)->class)
#define GET_SUBCLASS(n)		((n)->subclass)
#define GET_IRCHILD(i,num)	((irnode *)((i)->args[num]))
#define GET_IRCHILD_PTR(i,num)	((irnode **)((i)->args + num))
#define GET_FATHER(n)		((n)->father)
#define GET_PTR(n)		GET_IRCHILD_PTR(GET_FATHER(n),GET_RANK(n))
#define IS_LITERAL(n)		((n)->literal)
#define GET_LITERAL(n)		((n)->literal_value)
#define SET_MODE(n,m)		(n)->mode = m
#define SET_TYPE(n,ty)		(n)->t = ty
#define SET_SYMBOL(n,sy)	(n)->s = sy
#define SET_LVALUE(n,l)		(n)->lvalue = l
#define SET_LITERAL(n)		(n)->literal = 1
#define SET_LITERAL_VALUE(n,v)	(n)->literal_value = (v) 
#define SET_IRCHILD(i,num,e)	(*GET_IRCHILD_PTR(i,num) = (e))

/*
 * IR node 
 */
typedef struct xirnode
{

  /* IR node kind. returned by NODETYPE() macro */
  enum ir_nodetype kind;

  /* IR node machine mode. returned by GET_MODE() macro */
  int mode;

  /* IR class and subclass, as defined in ir.def */
  enum ir_class class;
  enum ir_subclass subclass;

  /* IR type, when applicable */
  type *t;

  /* set to 1 when irnode represents a literal value 
   * (arithmetic or string constant for instance)
   * By default an new irnode does not represent a literal
   */
  int literal;
  void *literal_value; /* if literal is set, hold pointer on literal value */

  /* IR flag for lvalue, when applicable */
  enum { LVALUE, NOT_LVALUE, NOT_LVALUE_READ_ONLY } lvalue;

  /* IR initial symbol, when applicable */
  symbol *s;

  /* for internal use in flowgraph construction */
  void *internal_pointer;
  void *basic_block;
  ListElem holder;

  /* IR args number and def, as defined in ir.def */
  int argnum;
  char *argdef;

  /* rank of the IR in the args list of its own father */
  int rank;

  /* irnode father */
  struct xirnode *father;

  /* IR arguments, as defined in ir.def. This table has argnum elements. Args are stored as (void *) */
  /* args should be accessed using GET_IRCHILD( node, i ) or GET_IRVOID( node, i) */
  void **args;

} irnode;

/* generic functions to create a new IR node */
irnode *IR (enum ir_nodetype, ...);
irnode *IRm (enum ir_nodetype, int mode, ...);

#include "irmacro.h"

/* irnodeList hold list of IR (irnode *) */
typedef List irnodeList;

/* irnodeRefList hold list of pointer on IR (irnode **) */ 
typedef List irnodeRefList;

// for debbugging
char *get_node_name_and_mode (irnode * n, char *name);

// cleanup. Free node and all its children recursively
void freenode (irnode *);

#endif
