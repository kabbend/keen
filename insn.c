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

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "ir.h"
#include "insn.h"
#include "asm.h"
#include "temp.h"
#include "reg.h"
#include "list.h"
#include "error.h"
#include "xmalloc.h"
#include "mode.h"

int
is_move (irnode * i)
{
  return NODETYPE (i) == n_INSN_MOVE;
}

void
store_insn (insnList l, irnode * i)
{
  if (!l) l = newList();
  add (l, i);
}

void
print_insnList (insnList INSN_LIST)
{
  tempList tl;
  labelList ll;
  int rank;
  int lineno;
  int column;
  int l, t;
  ListElem e;

  lineno = 1;
  ListElem p = get_first (INSN_LIST);
  while (p)
    {

      lineno++;
      irnode *node = object(p);
      if (!node)
	{
	  /* move instructions with same dest and src have been removed ( = set NULL )
	     during register allocation phase
	   */
          p = get_next (p);
	  continue;
	}
      char *c = (char *) GET_IRCHILD (node, 0);
      if (strlen(c)==0) 
        {
          /* empty instructions need not be written */
          p = get_next (p);
	  continue;
        }
      keen_debug ("%s%4d[r%4d]", (lineno == 1) ? "BL" : "  ", lineno,
		  *(int *) GET_IRCHILD (node, 5));
      column = 12;
      if (NODETYPE (node) != n_INSN_LABEL
	  && NODETYPE (node) != n_INSN_ASMD
	   && NODETYPE (node) != n_INSN_ASML)
	{
	  keen_debug ("        ");
	  column += 8;
	}
      if (NODETYPE (node) == n_INSN_ASML)
	{
	  // assembly literal. Do not parse for %d or %s markers
	  keen_debug ("%s\n", c);
          p = get_next (p);
	  continue;
	}
      while (*c)
	{
	  switch (*c)
	    {
	    case '%':
	      c++;
	      switch (*c)
		{
		case 'd':
		  tl = (tempList) GET_IRCHILD (node, 1);
		  t = 1;
		  l = 0;
		  break;
		case 's':
		  tl = (tempList) GET_IRCHILD (node, 2);
		  t = 1;
		  l = 0;
		  break;
		case 'l':
		  ll = (labelList) GET_IRCHILD (node, 3);
		  t = 0;
		  l = 1;
		  break;
		default:
		  abort ();
		}
	      c++;
	      int requested_mode = 0;
	      if (*c == '(')
		{
		  c++;
		  if (*c == '8') requested_mode = m8;
		  else if (*c == '1') { c++; if (*c == '6') requested_mode = m16; } 
		  else if (*c == '3') { c++; if (*c == '2') requested_mode = m32; } 
	 	  c++;
		  if (*c != ')') keen_error("unknown mode");
		  c++;
		}
	      rank = *c - '0';
	      if (t)
		{
		  e = get_ith (tl, rank);
		  assert (e);
		  temp *te = object (e);
		  if (is_mapped (te))
		    {
		      keen_debug ("%s", ASM_REGNAME (get_temp_name (te)));
		      column += strlen (ASM_REGNAME (get_temp_name (te)));
		    }
		  else
		    {
		      keen_debug ("%s", get_temp_name (te));
		      column += strlen (get_temp_name (te));
		    }
		}
	      else
		{
		  e = get_ith (ll, rank);
		  assert (e);
		  label *le = object (e);
		  keen_debug ("%s", get_label_name (le));
		  column += strlen (get_label_name (le));
		}
	      break;
	    default:
	      keen_debug ("%c", *c);
	      column++;
	    }
	  c++;
	}
      /* print dest temp list */
      while (column < 60)
	{
	  column++; keen_debug (" ");
	}
      keen_debug ("D(");
      column += 2;
      tl = (tempList) GET_IRCHILD (node, 1);
      e = get_first (tl);
      while (e)
	{
	  temp *te = object (e);
	  keen_debug ("%s", get_temp_index (te));
	  column += strlen (get_temp_index (te));
	  if (is_mapped (te))
	    {
	      keen_debug ("=%s", ASM_REGNAME (get_temp_name (te)));
	      column += strlen (ASM_REGNAME (get_temp_name (te))) + 1;
	    }
	  if (e != get_last (tl))
	    {
	      keen_debug (",");
	      column++;
	    }
	  e = get_next (e);
	}
      keen_debug (")");
      column++;
      /* print src temp list */
      while (column < 80)
	{
	  column++; keen_debug (" ");
	}
      keen_debug ("S(");
      column += 2;
      tl = (tempList) GET_IRCHILD (node, 2);
      e = get_first (tl);
      while (e)
	{
	  temp *te = object (e);
	  keen_debug ("%s", get_temp_index (te));
	  column += strlen (get_temp_index (te));
	  if (is_mapped (te))
	    {
	      keen_debug ("=%s", ASM_REGNAME (get_temp_name (te)));
	      column += strlen (ASM_REGNAME (get_temp_name (te))) + 1;
	    }
	  //if(tl->next) { keen_debug( ","); column++; }
	  if (e != get_last (tl))
	    {
	      keen_debug (",");
	      column++;
	    }
	  e = get_next (e);
	}
      keen_debug (")");
      column++;
      /* print jump list */
      while (column < 100)
	{
	  column++; keen_debug (" ");
	}
      keen_debug ("J(");
      column += 2;
      ll = (labelList) GET_IRCHILD (node, 3);
      e = get_first (ll);
      while (e)
	{
	  label *le = object (e);
	  keen_debug ("%s", get_label_name (le));
	  column += strlen (get_label_name (le));
	  //if(ll->next) { keen_debug( ","); column++; }
	  if (e != get_last (ll))
	    {
	      keen_debug (",");
	      column++;
	    }
	  e = get_next (e);
	}
      keen_debug (")\n");

      p = get_next (p);
    }

}


void
print_insn_to_file (irnode * p, int fd)
{
  tempList tl;
  labelList ll;
  int t, l, rank;
  char s[1024];
  ListElem e;
  if (!p) return;
  char *c = (char *) GET_IRCHILD (p, 0);
  if (strlen(c)==0) return;
  if (NODETYPE (p) != n_INSN_LABEL && NODETYPE (p) != n_INSN_ASMD && NODETYPE (p) != n_INSN_ASML)
    write (fd, "        ", 8);
  if (NODETYPE (p) == n_INSN_ASML)
    {
      // assembly literal. Do not parse for %d or %s markers
      write (fd, c, strlen (c));
      write (fd, "\n", 1);
      return;
    }
  while (*c)
    {
      switch (*c)
	{
	case '%':
	  c++;
	  switch (*c)
	    {
	    case 'd':
	      tl = (tempList) GET_IRCHILD (p, 1);
	      t = 1;
	      l = 0;
	      break;
	    case 's':
	      tl = (tempList) GET_IRCHILD (p, 2);
	      t = 1;
	      l = 0;
	      break;
	    case 'l':
	      ll = (labelList) GET_IRCHILD (p, 3);
	      t = 0;
	      l = 1;
	      break;
	    default:
	      abort ();
	    }
	  c++;
	  int requested_mode = 0;
	  if (*c == '(')
		{
		  c++;
		  if (*c == '8') requested_mode = m8;
		  else if (*c == '1') { c++; if (*c == '6') requested_mode = m16; } 
		  else if (*c == '3') { c++; if (*c == '2') requested_mode = m32; } 
	 	  c++;
		  if (*c != ')') keen_error("unknown mode");
		  c++;
		}
	  rank = *c - '0';
	  if (t)
	    {
	      e = get_ith (tl, rank);
	      assert (e);
	      temp *te = object (e);
	      if (is_mapped (te))
		{
		  char *temp_name;
		  if (requested_mode) 
			{
			 int r = get_reg_in_family ( te->r , requested_mode );
			 temp_name = GET_REG_NAME(r); 
			}
		  else
			{
			  temp_name = get_temp_name (te);
			}
		  sprintf (s, "%s", ASM_REGNAME ( temp_name ));
		  write (fd, s, strlen (s));
		}
	      else
		{
		  sprintf (s, "%s", get_temp_name (te));
		  write (fd, s, strlen (s));
		}
	    }
	  else
	    {
	      e = get_ith (ll, rank);
	      assert (e);
	      label *le = object (e);
	      sprintf (s, "%s", get_label_name (le));
	      write (fd, s, strlen (s));
	    }
	  break;
	default:
	  sprintf (s, "%c", *c);
	  write (fd, s, 1);
	}
      c++;
    }
  write (fd, "\n", 1);
}
