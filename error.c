/*
 * Keen compiler 
 *
 * error.c
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

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <assert.h>
#include "error.h"

/* stores current location kind */
enum lockind current_significant_location;

/* these variables are set by other modules during compilation */
static int error_reporting_line;
static int error_reporting_column;
static char *error_reporting_filename;

/* current debugging level */
extern int option_debug;

/* current debug output file. Can be changed by set_output_file() */
static FILE *debug_output_file = NULL;

void
SET_LOC (enum lockind level)
{
  current_significant_location = level;
}

void
SET_FILE (char *filename)
{
  error_reporting_filename = filename;
}

char *
GET_FILE ()
{
  return error_reporting_filename;
}

void
SET_LINE (int lineno)
{
  error_reporting_line = lineno;
}

int
GET_LINE ()
{
  return error_reporting_line;
}

void
INC_LINE ()
{
  error_reporting_line++;
}

void
SET_COLUMN (int col)
{
  error_reporting_column = col;
}

int
GET_COLUMN ()
{
  return error_reporting_column;
}

void
INC_COLUMN ()
{
  error_reporting_column++;
}

void
SET_POS (position pos)
{
  error_reporting_filename = pos.filename;
  error_reporting_line = pos.line;
  error_reporting_column = pos.column;
}

void
keen_error (char *fmt, ...)
{
  int i;
  va_list (args);
  switch (current_significant_location)
    {
    case location_none:
      fprintf (stderr, "error: ");
      break;
    case location_file:
      fprintf (stderr, "%s: error: ", error_reporting_filename);
      break;
    case location_line:
      fprintf (stderr, "%s:%d: error: ", error_reporting_filename,
	       error_reporting_line);
      break;
    case location_column:
      fprintf (stderr, "%s:%d,%d: error: ", error_reporting_filename,
	       error_reporting_line, error_reporting_column);
      break;
    default:
      assert (0);
    }
  va_start (args, fmt);
  vfprintf (stderr, fmt, args);
  va_end (args);
  fprintf (stderr, "\n");
  exit (EXIT_FAILURE);
}

void
keen_errorn (char *fmt, ...)
{
  int i;
  va_list (args);
  switch (current_significant_location)
    {
    case location_none:
      fprintf (stderr, "error: ");
      break;
    case location_file:
      fprintf (stderr, "%s: error: ", error_reporting_filename);
      break;
    case location_line:
      fprintf (stderr, "%s:%d: error: ", error_reporting_filename,
	       error_reporting_line);
      break;
    case location_column:
      fprintf (stderr, "%s:%d,%d: error: ", error_reporting_filename,
	       error_reporting_line, error_reporting_column);
      break;
    default:
      assert (0);
    }
  va_start (args, fmt);
  vfprintf (stderr, fmt, args);
  va_end (args);
  fprintf (stderr, "\n");
}

extern int option_skip_warning;

void
keen_warning (char *fmt, ...)
{
  int i;
  if (option_skip_warning)
    return;
  va_list (args);
  switch (current_significant_location)
    {
    case location_none:
      fprintf (stderr, "warning: ");
      break;
    case location_file:
      fprintf (stderr, "%s: warning: ", error_reporting_filename);
      break;
    case location_line:
      fprintf (stderr, "%s:%d: warning: ", error_reporting_filename,
	       error_reporting_line);
      break;
    case location_column:
      fprintf (stderr, "%s:%d,%d: warning: ", error_reporting_filename,
	       error_reporting_line, error_reporting_column);
      break;
    default:
      assert (0);
    }
  va_start (args, fmt);
  vfprintf (stderr, fmt, args);
  va_end (args);
  fprintf (stderr, "\n");
}

int
debug_level ()
{
  return option_debug;
}

void
_keen_debug (char *fmt, ...)
{
  if (!debug_output_file) return;
  va_list (args);
  va_start (args, fmt);
  vfprintf (debug_output_file, fmt, args);
  fflush (debug_output_file);
  va_end (args);
}

int
set_debug_file (char *filename)
{
  if (debug_output_file)
    fclose (debug_output_file);
  debug_output_file = fopen (filename, "w+");
  return (debug_output_file != NULL);
}

void
close_debug_file ()
{
  fclose (debug_output_file);
  debug_output_file = NULL;
}
