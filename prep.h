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

#ifndef _PREP_H
#define _PREP_H

#include "config.h"

void init_preprocess (void);
void preprocess (char *filename);
char *init_preprocess_buffer (void);
char *get_preprocess_buffer (void);
void predefine_macro (char *name, char *value);

#undef INCLUDE_SEARCH

/* include path can contain '*', which is replaced at runtime by the first directory available */

#ifdef TARGET_LINK_LDLINUX
#define INCLUDE_SEARCH	"/usr/include:/usr/include/linux:/usr/local/include:/usr/lib/gcc/*/*/include"
#endif

#ifdef TARGET_LINK_CYGWIN
#define INCLUDE_SEARCH	"/usr/include:/usr/include/cygwin"
#endif
#endif
