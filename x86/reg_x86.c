/*
* Keen compiler 
* Copyright (C) 2006 Karim Ben Djedidia <kabend@free.fr> 
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
#include "reg.h"
#include "mode.h"

reg hardregset[] = {
  {0,  "eax", REGCLASS_GENERAL | REGCLASS_RV, m32, NOREG, NULL, NULL} ,
  {1,  "ecx", REGCLASS_GENERAL, m32, NOREG, NULL, NULL} ,
  {2,  "edx", REGCLASS_GENERAL, m32, NOREG, NULL, NULL} ,
  {3,  "ebx", REGCLASS_GENERAL, m32, NOREG, NULL, NULL} ,
  {4,  "edi", REGCLASS_GENERAL, m32, NOREG, NULL, NULL} ,
  {5,  "esi", REGCLASS_GENERAL, m32, NOREG, NULL, NULL} ,
  {6,  "ebp", REGCLASS_FP,      m32, NOREG, NULL, NULL} ,
  {7,  "ax",  REGCLASS_GENERAL, m16, 0, NULL, NULL} ,
  {8,  "al",  REGCLASS_GENERAL, m8,  7, NULL, NULL} ,
  {9,  "ah",  REGCLASS_GENERAL, m8,  7, NULL, NULL} ,
  {10, "bx",  REGCLASS_GENERAL, m16, 3, NULL, NULL} ,
  {11, "bl",  REGCLASS_GENERAL, m8,  10, NULL, NULL} ,
  {12, "bh",  REGCLASS_GENERAL, m8,  10, NULL, NULL} ,
  {13, "cx",  REGCLASS_GENERAL, m16, 1, NULL, NULL} ,
  {14, "cl",  REGCLASS_GENERAL, m8,  13, NULL, NULL} ,
  {15, "ch",  REGCLASS_GENERAL, m8,  13, NULL, NULL} ,
  {16, "dx",  REGCLASS_GENERAL, m16, 2, NULL, NULL} ,
  {17, "dl",  REGCLASS_GENERAL, m8,  16, NULL, NULL} ,
  {18, "dh",  REGCLASS_GENERAL, m8,  16, NULL, NULL} ,
  /* special registers */
  {19, "esp", REGCLASS_FP,      m32, NOREG, NULL, NULL}
};
