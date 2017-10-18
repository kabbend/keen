/*
 * Keen compiler 
 *
 * error.h
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

#include "grammar.h"

/* indicates current location kind : */
enum lockind
{
  location_none,		/* no particular location */
  location_file,		/* inside a file, but no more precision */
  location_line,		/* inside a file, and line # is significant */
  location_column		/* inside a file, and line & column # is significant */
};

/* modifies current location kind */
void SET_LOC (enum lockind level);
void SET_FILE (char *filename);
char *GET_FILE ();
void SET_LINE (int lineno);
int GET_LINE ();
void INC_LINE ();
void SET_COLUMN (int column);
int GET_COLUMN ();
void INC_COLUMN ();
void SET_POS (position pos);

void keen_errorn (char *fmt, ...);	/* display non-blocking error and resumes treatment */
void keen_error (char *fmt, ...);	/* display blocking error */
void keen_warning (char *fmt, ...);	/* display warning */

/* debug routines */

/* message level */
#define NO_DEBUG	0
#define DEBUG_NORMAL	1
#define DEBUG_VERBOSE	2

#undef keen_debug
#ifdef KEEN_DEBUG_ON
#define keen_debug(...)		if (debug_level()) _keen_debug(__VA_ARGS__)
#else
#define keen_debug(...)		
#endif

int debug_level ();			/* return applicable debug level ( 0 = no debug ) */
int set_debug_file (char *filename);	/* set current output debug file to filename. Create this file if
					   does not already exist. Close any previous debug file */
void close_debug_file ();		/* close current debug file */
