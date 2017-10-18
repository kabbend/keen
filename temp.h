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

#ifndef _TEMP_H_
#define _TEMP_H_

#include "list.h"
#include "ir.h"

/*
 * temporary
 */
typedef struct xtemp temp;
typedef List tempList;

struct xtemp
{
  int index;		/* incremental id */
  char *name;		/* human readable name (register if known, of the form "t"index otherwise */
  int r;		/* register index the temporary represents, when known */
  int mode;		/* mode m8, m16, m32 etc. */
  int is_kburg_product;	/* is this temporary created during translation phase (flag is 0) or during
			   kburg phase to hold intermediate results between kburg rules (flag is 1).
			   In such case, these temporaries can be optimized under some circumstances. */
  int is_a_spill;	/* flag set if temp was chosen previously for spilling */
  int usedef_count;	/* total count of uses and defs for this temporary */
  List insn_usedefs;	/* list of insn in the final trace where the temp is a use and/or a def */
};

tempList registers_templist;	/* pre-existing temporaries. Each one maps an actual register */

temp *new_temp (void);		/* returns a brand new temporary. mode is FBY_mode (4-bytes) by default */
temp *new_temp_mode (int mode);	/* returns a brand new temporary with given mode */
temp *new_named_temp (char *);	/* returns mapped temporary knowing its register-name */
temp *new_indexed_temp (int r);	/* returns mapped temporary knowing its register-index */
char *get_temp_name (temp * t);	/* returns readable name of the temporary (either register name, if mapped, or "t%d") */
char *get_temp_index (temp * t);	/* returns a string that contains temporary index (convenient function for string manipulations) */
int is_mapped (temp * t);	/* 1 if temporary is mapped to a register */
int get_temp_number ();		/* get total number of temporaries */
int get_temp_index_number (temp * t);	/* returns temporary index */
temp *get_temp (int index);	/* returns temporary knowing its index */
int get_temp_map (temp * t);	/* returns register index of the (supposed mapped) temporary */
void map_temp (int t_index, int reg);	/* maps a temporary (t_index) with register index reg */
int get_temp_mode (int t_index);	/* returns mode for temp */
int inc_usedef( temp *); /* increment use-def for this temp */
int get_usedef( temp *);

void temp_is_a_spill( temp * );
#define is_a_spill(a_temp)	(a_temp->is_a_spill)	

void cleanup_temps();

/*
 * label
 */
typedef struct xlabel label;
typedef List labelList;

int same_label (label *, label *);	/* 1 if labels are the same (based on name) */
label *new_label (void);	/* creates a new (internal) label */
label *new_named_label (char *name);	/* creates a new label with given name */
char *get_label_name (label * l);	/* returns label name */
int is_internal_label (label *);	/* 1 if label is created by compiler, 0 if created by user code */
int label_usage (label * l);	/* returns number of jumps to this label */
int change_label_usage (label * l, int howmuch);	/* change current usage (numbers of jumps to label) by howmuch value */
irnode *label_location (label * l);	/* returns insn where the label is declared */
void set_label_location (label * l, irnode * i);	/* set insn where the label is declared */

#endif
