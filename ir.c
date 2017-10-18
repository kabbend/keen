/*
* Keen compiler 
*
* ir.c
*
* implement constructors for IR trees
*
* Copyright (C) 2003 Karim Ben Djedidia <kabend@free.fr> 
*
* This program is free software{ } you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation{ } either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY{ } without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program{ } if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  
* USA
*
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>
#include "mode.h"
#include "ir.h"
#include "temp.h"
#include "frame.h"

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
#define DEF_NODE0( t, c, s, arg ) { n_##t , c, s, arg } ,
#define DEF_NODE1( t, c, s, arg ) { n_##t , c, s, arg } ,
#define DEF_NODE2( t, c, s, arg ) { n_##t , c, s, arg } ,
#define DEF_NODE3( t, c, s, arg ) { n_##t , c, s, arg } ,
#define DEF_NODE4( t, c, s, arg ) { n_##t , c, s, arg } ,
#define DEF_NODE5( t, c, s, arg ) { n_##t , c, s, arg } ,
#define DEF_NODE6( t, c, s, arg ) { n_##t , c, s, arg } ,
#define DEF_NODE7( t, c, s, arg ) { n_##t , c, s, arg } ,
static irdef irdef_table[] = {
#include "ir.def"
};

#undef  DEF_NODE0
#undef  DEF_NODE1
#undef  DEF_NODE2
#undef  DEF_NODE3
#undef  DEF_NODE4
#undef  DEF_NODE5
#undef  DEF_NODE6
#undef  DEF_NODE7
#define DEF_NODE0( t, c, s, arg ) #t ,
#define DEF_NODE1( t, c, s, arg ) #t ,
#define DEF_NODE2( t, c, s, arg ) #t ,
#define DEF_NODE3( t, c, s, arg ) #t ,
#define DEF_NODE4( t, c, s, arg ) #t ,
#define DEF_NODE5( t, c, s, arg ) #t ,
#define DEF_NODE6( t, c, s, arg ) #t ,
#define DEF_NODE7( t, c, s, arg ) #t ,
static char *node_names[] = {
#include "ir.def"
};

// readable modes, for debugging
char *
mode_name (int mode)
{
  switch (mode)
    {
    case CC_mode:
      return ":cc";
    case FBY_mode:
      return ":4b";
    case TBY_mode:
      return ":2b";
    case SBY_mode:
      return ":1b";
    case BIT_mode:
      return ":bit";
    case NUL_mode:
      return "";
    case STR_mode:
      return ":str";
    default:
      fprintf (stderr, "unknown mode\n");
      abort ();
    }
}

/* generic function to create a new IR node
 * By default the IR mode is set as the same as 1st arg, if applicable, or NUL_mode otherwise
 * Use IRm() instead to set a mode explicitly
 * 'e' = an irnode expression (n_CLASS_EXPR)
 * 'E' = a list of irnode expressions
 * 'b' = an irnode binop (n_SUBCLASS_BINOP)
 * 'i' = an integral value
 * 'l' = a label
 * 'L' = a list of labels
 * 't' = a temporary
 * 'T' = a list of temporaries
 * 'M' = an irnode memory (n_SUBCLASS_MEM) or temporary (n_SUBCLASS_TEMP)
 * 'S' = a string
 * 's' = an irnode statement (n_CLASS_STMT)
 * 'x' = any irnode
 */
irnode *
IR (enum ir_nodetype kind, ...)
{

  char *fmt = irdef_table[kind].def;
  irnode *e;
  frame *f;
  irnodeList il;
  tempList tl;
  labelList ll;
  dataList dl;
  int i;
  label *l;
  temp *t;
  char *s;

  /* create new irnode */
  irnode *new = (irnode *) xmalloc (sizeof (irnode));
  new->kind = kind;
  new->mode = NUL_mode;
  new->class = irdef_table[kind].c;
  new->subclass = irdef_table[kind].s;
  new->t = Void ;
  new->lvalue = NOT_LVALUE;
  new->s = NULL;
  new->argnum = strlen (irdef_table[kind].def);
  new->argdef = irdef_table[kind].def;
  new->literal = 0;
  new->internal_pointer = 0;

  /* allocate space for args */
  new->args = (void **) xcalloc (new->argnum, sizeof (void *));

  int n = 0;
  va_list args;
  va_start (args, kind);
  while (*fmt)
    {
      switch (*fmt++)
	{
	case 'e':		/* irnode expression */
	  e = va_arg (args, irnode *);
	  if (GET_CLASS (e) != n_CLASS_EXPR)
	    {
	      fprintf (stderr, "IR, no CLASS_EXPR\n");
	      abort ();
	    }
	  e->rank = n;
	  e->father = new;
	  new->args[n++] = (void *) e;
	  break;
	case 'b':		/* irnode binop */
	  e = va_arg (args, irnode *);
	  if (GET_SUBCLASS (e) != n_SUBCLASS_BINOP)
	    {
	      fprintf (stderr, "IR, no SUBCLASS_BINOP");
	      abort ();
	    }
	  e->rank = n;
	  e->father = new;
	  new->args[n++] = (void *) e;
	  break;
	case 'M':		/* irnode memory */
	  e = va_arg (args, irnode *);
	  if (GET_SUBCLASS (e) != n_SUBCLASS_MEM)
	    {
	      fprintf (stderr, "IR, no SUBCLASS_MEM");
	      abort ();
	    }
	  e->rank = n;
	  e->father = new;
	  new->args[n++] = (void *) e;
	  break;
	case 'S':		/* string */
	  s = va_arg (args, char *);
	  new->args[n++] = (void *) s;
	  break;
	case 's':		/* irnode sequence */
	  e = va_arg (args, irnode *);
	  if (GET_CLASS (e) != n_CLASS_STMT)
	    {
	      fprintf (stderr, "IR, no CLASS_STMT");
	      abort ();
	    }
	  e->rank = n;
	  e->father = new;
	  new->args[n++] = (void *) e;
	  break;
	case 'x':
	  e = va_arg (args, irnode *);
	  e->rank = n;
	  e->father = new;
	  new->args[n++] = (void *) e;
	  break;
	case 'i':		/* integral */
	  i = va_arg (args, int);
	  int *new_int = (int *) xmalloc (sizeof (int));
	  *new_int = i;
	  new->args[n++] = (void *) new_int;
	  break;
	case 'l':		/* label */
	  l = va_arg (args, label *);
	  new->args[n++] = (void *) l;
	  break;
	case 't':		/* temporary */
	  t = va_arg (args, temp *);
	  new->args[n++] = (void *) t;
	  break;
	case 'E':
	  il = va_arg (args, irnodeList);
	  new->args[n++] = (void *) il;
	  break;
	case 'L':
	  ll = va_arg (args, labelList);
	  new->args[n++] = (void *) ll;
	  break;
	case 'T':
	  tl = va_arg (args, tempList);
	  new->args[n++] = (void *) tl;
	  break;
	case 'D':
	  dl = va_arg (args, dataList);
	  new->args[n++] = (void *) dl;
	  break;
	case 'f':
	  f = va_arg (args, frame *);
	  new->args[n++] = (void *) f;
	  break;
	default:
	  assert (0);
	}
    }
  va_end (args);
  return new;
}

irnode *
IRm (enum ir_nodetype kind, int mode, ...)
{

  char *fmt = irdef_table[kind].def;
  irnode *e;
  frame *f;
  irnodeList il;
  tempList tl;
  labelList ll;
  dataList dl;
  int i;
  label *l;
  temp *t;
  char *s;

  /* create new irnode */
  irnode *new = (irnode *) xmalloc (sizeof (irnode));
  new->kind = kind;
  new->mode = mode;
  new->class = irdef_table[kind].c;
  new->subclass = irdef_table[kind].s;
  new->t = Void ;
  new->lvalue = NOT_LVALUE;
  new->s = NULL;
  new->argnum = strlen (irdef_table[kind].def);
  new->argdef = irdef_table[kind].def;
  new->literal = 0;
  new->internal_pointer = 0;

  /* allocate space for args */
  new->args = (void **) xcalloc (new->argnum, sizeof (void *));

  int n = 0;
  va_list args;
  va_start (args, mode);
  while (*fmt)
    {
      switch (*fmt++)
	{
	case 'e':		/* irnode expression */
	  e = va_arg (args, irnode *);
	  if (GET_CLASS (e) != n_CLASS_EXPR)
	    abort ();
	  e->rank = n;
	  e->father = new;
	  new->args[n++] = (void *) e;
	  break;
	case 'b':		/* irnode binop */
	  e = va_arg (args, irnode *);
	  if (GET_SUBCLASS (e) != n_SUBCLASS_BINOP)
	    abort ();
	  e->rank = n;
	  e->father = new;
	  new->args[n++] = (void *) e;
	  break;
	case 'M':		/* irnode memory */
	  e = va_arg (args, irnode *);
	  if (GET_SUBCLASS (e) != n_SUBCLASS_MEM)
	    abort ();
	  e->rank = n;
	  e->father = new;
	  new->args[n++] = (void *) e;
	  break;
	case 'S':		/* string */
	  s = va_arg (args, char *);
	  new->args[n++] = (void *) s;
	  break;
	case 's':		/* irnode sequence */
	  e = va_arg (args, irnode *);
	  if (GET_CLASS (e) != n_CLASS_STMT)
	    abort ();
	  e->rank = n;
	  e->father = new;
	  new->args[n++] = (void *) e;
	  break;
	case 'x':
	  e = va_arg (args, irnode *);
	  e->rank = n;
	  e->father = new;
	  new->args[n++] = (void *) e;
	  break;
	case 'i':		/* integral */
	  i = va_arg (args, int);
	  int *new_int = (int *) xmalloc (sizeof (int));
	  *new_int = i;
	  new->args[n++] = (void *) new_int;
	  break;
	case 'l':		/* label */
	  l = va_arg (args, label *);
	  new->args[n++] = (void *) l;
	  break;
	case 't':		/* temporary */
	  t = va_arg (args, temp *);
	  new->args[n++] = (void *) t;
	  break;
	case 'E':
	  il = va_arg (args, irnodeList);
	  new->args[n++] = (void *) il;
	  break;
	case 'L':
	  ll = va_arg (args, labelList);
	  new->args[n++] = (void *) ll;
	  break;
	case 'T':
	  tl = va_arg (args, tempList);
	  new->args[n++] = (void *) tl;
	  break;
	case 'D':
	  dl = va_arg (args, dataList);
	  new->args[n++] = (void *) dl;
	  break;
	case 'f':
	  f = va_arg (args, frame *);
	  new->args[n++] = (void *) f;
	  break;
	default:
	  assert (0);
	}
    }
  va_end (args);
  return new;
}

/* returns a readable node name and mode for debugging */
char *
get_node_name_and_mode (irnode * n, char *name)
{
  sprintf (name, "%s%s", node_names[NODETYPE (n)], mode_name (n->mode));
  return name;
}

void
freenode (irnode *n)
{
  if (!n) return;
  while (--(n->argnum) >= 0)
    {
      switch(n->argdef[n->argnum])
        {
	  case 'e': case 'b': case 'M': case 'x': case 's':
	    freenode (GET_IRCHILD(n,n->argnum));    
        }
    }
}

