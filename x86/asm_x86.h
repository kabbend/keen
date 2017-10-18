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

#ifndef _ASM_X86_H_
#define _ASM_X86_H_

#define ASM_REGNAME(name)	(stringf("%%%s",name))
#define ASM_LABELBASE		".L"

#undef ASM_FUNCBASE
#ifdef TARGET_LINK_CYGWIN
#define ASM_FUNCBASE		"_"
#else
#define ASM_FUNCBASE		""
#endif

#define DEFAULT_MODE		FBY_mode /* 32 bits */

#define ASM_CJUMP_FALLTHROUGH_FALSE_LABEL	0

#define N_REG			20	
#define REG_RV			0
#define REG_FP			6
#define REG_SP			19	
#define REG_FIRST_CALLEE	3
#define REG_LAST_CALLEE		5
#define REG_FIRST_CALLER	0
#define REG_LAST_CALLER		2
#define REG_FIRST_SPECIAL	19

#endif
