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

#ifndef _ASM_H_
#define _ASM_H_

#include "config.h"

#ifdef TARGET_ARCH_X86
#include "x86/asm_x86.h"
#endif

#ifndef ASM_REGNAME
#define ASM_REGNAME(s) 	(s)
#endif

#ifndef ASM_LABELBASE
#define ASM_LABELBASE	(".L")
#endif

#ifndef ASM_FUNCBASE
#define ASM_FUNCBASE	("_")
#endif

#ifndef ASM_CJUMP_FALLTHROUGH_FALSE_LABEL
#error "please define ASM_CJUMP_FALLTHROUGH_FALSE_LABEL value in asm_(target-arch).h"
#endif

#ifndef N_REG
#error "please define N_REG value in asm_(target-arch).h. Total number of hard registers"
#endif

#ifndef REG_RV
#error "please define REG_RV value in asm_(target-arch).h. Index of Return Value register"
#endif

#ifndef REG_FP
#error "please define REG_FP value in asm_(target-arch).h. Index of Frame Pointer register"
#endif

#ifndef REG_SP
#error "please define REG_SP value in asm_(target-arch).h. Index of Stack Pointer register"
#endif

#ifndef REG_FIRST_CALLEE
#error "please define REG_FIRST_CALLEE value in asm_(target-arch).h"
#endif

#ifndef REG_LAST_CALLEE
#error "please define REG_LAST_CALLEE value in asm_(target-arch).h"
#endif

#ifndef REG_FIRST_CALLER
#error "please define REG_FIRST_CALLER value in asm_(target-arch).h"
#endif

#ifndef REG_LAST_CALLER
#error "please define REG_LAST_CALLER value in asm_(target-arch).h"
#endif

#ifndef REG_FIRST_SPECIAL
#error "please define REG_FIRST_SPECIAL value in asm_(target-arch).h"
#endif

#endif
