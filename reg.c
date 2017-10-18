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

#include <stdio.h>
#include "reg.h"

reg *
get_reg_by_name (char *s)
{
  reg *r;
  int i;
  for (i = 0, r = hardregset; i < N_REG; i++, r++)
    if (strcmp (s, r->name) == 0)
      return r;
  return NULL;
}

int
get_reg_index (char *s)
{
  reg *r;
  int i;
  for (i = 0, r = hardregset; i < N_REG; i++, r++)
    if (strcmp (s, r->name) == 0)
      return i;
  return NOREG;
}

int
get_reg_in_family (int r, int mode)
{
  if (hardregset[r].mode == mode) return r;
  int i;
  for(i=0;i<N_REG;i++) if (hardregset[i].subreg_of == r) return get_reg_in_family(i,mode);
  return -1;
}

