/*
* Keen compiler 
* Copyright (C) 2007 Karim Ben Djedidia <kabend@free.fr> 
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

#ifndef _LINK_H_
#define _LINK_H_

#include "config.h"
#include "list.h"

typedef List stringList;

/* 
 * assemble one file, produce assembly code
 */
int assemble_file (char *asmfile, char *objectfile);

/*
 * link several object code files into one executable
 */
int link_file (stringList objectlist, char *exefile);

/*
 * define specs for assembly and link command depending on target os
 * %o stands for output file 
 * %x stands for input file
 */

#define AS_SPEC		"as -o %o %x"

#undef LINK_SPEC

#ifdef TARGET_LINK_LDLINUX
#define LINK_SPEC	"ld -dynamic-linker /lib/ld-linux.so.2 -o %o /usr/lib/crt1.o /usr/lib/crti.o %x /usr/lib/crtn.o -lc"
#endif

#ifdef TARGET_LINK_CYGWIN
#define LINK_SPEC	"ld -Bdynamic -o %o /usr/lib/crt0.o %x -lc -lcygwin -luser32 -lkernel32 -lshell32"
#endif

#endif
