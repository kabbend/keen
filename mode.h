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

#ifndef _MODE_H_
#define _MODE_H_

/* IR modes */
enum {
 NUL_mode = 0,  /* no or unknown mode */

 SBY_mode = 1,  /* Single Byte mode */
 m8       = 1,  

 TBY_mode = 2,  /* Two Bytes mode */
 m16      = 2,

 FBY_mode = 4,  /* Four Bytes mode */
 m32      = 4,

 EBY_mode = 8,  /* Eight Bytes mode */
 m64      = 8,

 BIT_mode = 10, /* bit mode */

 STR_mode = 11, /* string mode */

 CC_mode  = 12  /* condition code mode */
};

char *mode_name (int mode);

#endif
