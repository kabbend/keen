/*
* Keen compiler 
*
* frame.c
*
* implement frame functions common to all architectures
* Architecture-specific functions are provided in
* their respective directories (i.e. /x86 )
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

#include "frame.h"

data *
create_data (int mode, int padding, char *value)
{
 data *new = (data *) xmalloc(sizeof(data));
 new->mode = mode;
 new->padding = padding;
 new->value = value;
 return new;
}

