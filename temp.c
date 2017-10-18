/*
 * Keen compiler 
 *
 * temp.c
 *
 * Temporaries and labels routines
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
#include "temp.h"
#include "reg.h"
#include "asm.h"
#include "list.h"
#include "ir.h"
#include "mode.h"
#include "error.h"

/*
 * internal temp list
 */
tempList TEMPS = NULL;

void
cleanup_temps()
{
  tempList new = newList();
  temp *t;
  int i = 0, p = 0;
  FOR_EACH ( t, TEMPS, e )
    {
      if (i < size(registers_templist) || get_usedef(t) != 0) 
		{ 		
		  keen_debug("temp cleanup: renaming t%d to t%d\n",p,i);
		  t->index = i++; 
  		  sprintf (t->name, "t%d", t->index);
		  add(new,t); 
		}
 	p++;
    }
  TEMPS = new;
}

int 
inc_usedef( temp *t )
{
 return ++(t->usedef_count);
}

int
get_usedef( temp *t )
{
 return t->usedef_count;
}

temp *
new_temp (void)
{
  temp *t = (temp *) xmalloc (sizeof (temp));
  t->index = size (TEMPS);
  t->r = NOREG;
  t->mode = FBY_mode;
  t->name = (char *) xmalloc (10);
  t->is_a_spill = 0;
  t->usedef_count = 0;

  t->insn_usedefs = 0; 
	// as the trace can be rewritten several times during register allocation phase, we need to
	// initialize this list every time, not just here once

  t->is_kburg_product = 0; 
	// a priori, should be changed by the caller when needed
		
  sprintf (t->name, "t%d", t->index);
  add (TEMPS, t);
  return t;
}

temp *
new_temp_mode (int mode)
{
  temp *new = new_temp ();
  new->mode = mode;
  return new;
}

temp *
new_named_temp (char *s)
{
  return (temp *) object (get_ith (TEMPS, GET_REG_INDEX (s)));
}

temp *
new_indexed_temp (int r)
{
  return (temp *) object (get_ith (TEMPS, r));
}

char *
get_temp_name (temp * t)
{
  if (t->r == NOREG)
    return t->name;
  else
    return GET_REG_NAME (t->r);
}

char *
get_temp_index (temp * t)
{
  return t->name;
}

int
get_temp_index_number (temp * t)
{
  return t->index;
}

int
is_mapped (temp * t)
{
  return t->r != NOREG;
}

int
get_temp_number ()
{
  return size (TEMPS);
}

temp *
get_temp (int index)
{
  return (temp *) object (get_ith (TEMPS, index));
}

int
get_temp_map (temp * t)
{
  return t->r;
}

void
map_temp (int index, int r)
{
  temp *t = (temp *) object (get_ith (TEMPS, index));
  t->r = r;
}

int
get_temp_mode (int t_index)
{
  return ((temp *) object (get_ith (TEMPS, t_index)))->mode;
}

void
init_temporaries (void)
{

  int i;

  // init lists
  TEMPS = newList ();
  registers_templist = newList ();

  // init mapped temporaries in specific registers_templist list
  // one temporary is created for each register, in register order
  // these temporaries are added in TEMPS list at the same time
  for (i = 0; i < N_REG; i++)
    {
      temp *t = new_temp_mode (hardregset[i].mode);
      t->r = i;			/* store register index */
      add (registers_templist, t);
    }

}


/*
 * Label routines 
 */
struct xlabel
{
  int internal;			/* 0 if user-label, 1 if created by compiler */
  int usage;			/* number of jumps to this label */
  char *name;			/* name of the label */
  irnode *insn;			/* insn where the label is declared */
};

static int label_index = 0;

int
same_label (label * a, label * b)
{
  if (a == NULL)
    return 0;
  if (b == NULL)
    return 0;
  if (strcmp (get_label_name (a), get_label_name (b)) == 0)
    return 1;
  return 0;
}

label *
new_label (void)
{
  char s[128];
  label *l = (label *) xmalloc (sizeof (label));
  sprintf (s, "%s%d", ASM_LABELBASE, label_index++);
  l->name = (char *) xmalloc (strlen (s) + 1);
  strcpy (l->name, s);
  l->internal = 1;
  l->usage = 0;
  return l;
}

label *
new_named_label (char *name)
{
  if (name == NULL)
    return NULL;
  label *l = (label *) xmalloc (sizeof (label));
  l->name = (char *) xmalloc (strlen (name) + 1);
  strcpy (l->name, name);
  l->internal = 0;
  return l;
}

char *
get_label_name (label * l)
{
  return l->name;
}

int
is_internal_label (label * l)
{
  return l->internal;
}

int
label_usage (label * l)
{
  return l->usage;
}

int
change_label_usage (label * l, int howmuch)
{
  l->usage += howmuch;
  return l->usage;
}

irnode *
label_location (label * l)
{
  return l->insn;
}

void
set_label_location (label * l, irnode * i)
{
  l->insn = i;
}

void
temp_is_a_spill( temp *t )
{
 t->is_a_spill = 1;
}

