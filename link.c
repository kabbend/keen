/*
* Keen compiler 
*
* link.c
*
* Copyright (C) 2007 Karim Ben Djedidia <kabend@free.fr> 
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
#include <string.h>
#include "link.h"
#include "xmalloc.h"
#include "list.h"
#include "error.h"

#ifndef LINK_SPEC
#error LINK_SPEC undefined in link.h
#endif

int
execute_command (char *input_file, char *output_file, char *spec)
{

  char **location = (char **) xcalloc (sizeof (char *), 2);
  int loc_index = 0;
  char *c;

  // order input and output values depending on spec %x or %o markers
  c = spec;
  while (*c)
    {
      switch (*c)
	{
	case '%':
	  c++;
	  switch (*c)
	    {
	    case 'o':
	      if (loc_index >= 2)
		abort ();	// should not happen
	      location[loc_index++] = output_file;
	      *c = 's';
	      c++;
	      break;
	    case 'x':
	      if (loc_index >= 2)
		abort ();	// should not happen
	      location[loc_index++] = input_file;
	      *c = 's';
	      c++;
	      break;
	    }
	  break;
	default:
	  c++;
	}
    }

  // write actual link command
  char *command = stringf (spec, location[0], location[1]);

  int ret = system (command);

  if (debug_level () >= DEBUG_VERBOSE)
    {
      keen_debug ("keen: execute command: %s\n (%d)", command, ret);
    }

  free (spec);
  free (command);

  return (ret == 0)? EXIT_SUCCESS: EXIT_FAILURE ;
}

int
assemble_file (char *asm_file, char *object_file)
{
  return execute_command (asm_file, object_file, string (AS_SPEC));
}

int
link_file (stringList objectlist, char *exe_file)
{
  char final_object_list[4096];
  char *c = final_object_list;
  ListElem e = get_first (objectlist);
  while (e)
    {
      sprintf (c, "%s ", (char *) object (e));
      c += strlen (object (e)) + 1;
      e = get_next (e);
    }
  return execute_command (final_object_list, exe_file, string (LINK_SPEC));
}
