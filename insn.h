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

#ifndef _INSN_H_
#define _INSN_H_

#include "ir.h"

typedef irnodeList insnList;

void store_insn (insnList , irnode *);
void print_insnList (insnList);
irnode *genInsn (insnList , irnode *);
int is_move (irnode *);

void print_insn_to_file (irnode * insn, int fd);

#endif
