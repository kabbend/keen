/*
* Keen compiler 
* Copyright (C) 2006 Karim Ben Djedidia <kabend@free.fr> 
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

#ifndef _REG_H
#define _REG_H

#include "asm.h"
#include "temp.h"

#define NOREG	-1

#define REGCLASS_GENERAL	0x0000000	/* general register */
#define REGCLASS_FLOAT		0x0000001	/* floating point */
#define REGCLASS_SPECIAL	0x0000002	/* special regs */

#define REGCLASS_RV		0xF000000	/* return value */
#define REGCLASS_FP		0xF100000	/* frame pointer */
#define REGCLASS_SP		0xF200000	/* stack pointer */
#define REGCLASS_ZERO		0xF400000	/* zero reg */
#define REGCLASS_ARG		0xF800000	/* pass arg to function */
#define REGCLASS_CALLEE		0xF010000	/* callee save */
#define REGCLASS_CALLER		0xF020000	/* caller save */

typedef struct xreg
{
  int id;
  char *name;
  int class;
  int mode;
  int subreg_of;
  int *subregs;
  tempList temps;
} reg;

extern reg hardregset[];

int get_reg_index (char *s);
reg *get_reg_by_name (char *s);

int get_reg_in_family (int reg_index, int requested_mode);
	
#define GET_REG_NAME(i)		(hardregset[i].name)
#define GET_REG_INDEX(s)	(get_reg_index(s))
#define GET_REG_BY_INDEX(i)	(hardregset[i])
#define GET_REG_BY_NAME(s)	(get_reg_by_name(s))

#define RV()			(new_indexed_temp(REG_RV))
#define FP()			(new_indexed_temp(REG_FP))

#endif
