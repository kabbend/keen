/*
 * Keen compiler 
 *
 * xmalloc.c
 *
 * wrap malloc() and calloc() calls
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

#include <string.h>
#include <stdarg.h>
#include "xmalloc.h"
#include "error.h"

void *
xmalloc (size_t size)
{
  void *p = (void *) malloc (size);
  if (p == NULL)
    keen_error ("resource shortage");
  return p;
}

void *
xcalloc (int n, size_t size)
{
  void *p = (void *) calloc (n, size);
  if (p == NULL)
    keen_error ("resource shortage");
  return p;
}

void *
xrealloc (void *p, size_t size)
{
  void *r = (void *) realloc (p, size);
  if (r == NULL)
    keen_error ("resource shortage");
  return r;
}

char *
string (char *s)
{
  if (s == NULL)
    return NULL;
  char *r = (char *) xmalloc (strlen (s) + 1);
  strcpy (r, s);
  return r;
}

char *
stringf (char *fmt, ...)
{
  char buffer[4096]; /* FIXME this should be dynamic as some string may be very large (nDATA node for instance) */
  va_list (args);
  va_start (args, fmt);
  vsnprintf (buffer, 4096, fmt, args);
  va_end (args);
  return string (buffer);
}

char *
stringn (char *start, char *end)
{
  char c = *end;
  *end = 0;
  char *ret = string (start);
  *end = c;
  return ret;
}
