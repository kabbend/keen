/*
 * Keen compiler 
 *
 * prep.c
 *
 * Preprocessor routines for keen
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
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "error.h"
#include "prep.h"
#include "list.h"
#include "xmalloc.h"
#include <dirent.h>

/* list of directories to search for #include directives */
List include_search_list;

/* maximum number of nested inclusions */
#define MAX_INCLUDE_DEPTH       32

/* unit size to read files */
#define BUFFER_READ_SIZE        4096

/* unit size of translation unit buffer ( = output buffer ) */
#define OUTPUT_BUFFER_BLOCK     4096

/* unit size of logical line (used during preprocessing) */
#define LOGICAL_LINE_SIZE  	4096

char logical_line[LOGICAL_LINE_SIZE];

/* final buffer in which the preprocessor writes result 
 * tu stands for Translation Unit */
char *tu = NULL;

/* position in this buffer */
char *tu_pos = NULL;

/* end of this buffer */
char *tu_end = NULL;

/*
 * Buffered_file structure
 * Preprocessor preliminary copy files to memory as they are needed. A buffered_file
 * structure represents such a memory-mapped file for reading. Note that several 
 * buffered_file may represent the same file, if different reading loops are needed
 * (in such case duplicate is 1) 
 */
typedef struct
{

  /* name of the file (including path) */
  char *filename;

  /* 1 if file is currently used by preprocessor
   * When a file is no more used (preprocessed until the end), it is not
   * released so it is immediatly available (without disk operation) if
   * another include directive requires it */
  int in_use;

  /* current size of the memory-mapped file */
  ssize_t size;

  /* current memory space allocated for the file */
  ssize_t limit;

  /* pointer to file content */
  char *text;

  /* current read-pointer in file content */
  char *position;

  /* indicates that several buffered_file point to same file text */
  int duplicate;

  /* indicates nesting level of the file */
  int nesting_level;

} buffered_file;

/* read next char from buffered file b */
int
bf_getc (buffered_file * b)
{
  int c;
  if (b->position >= b->text + b->size)
    c = EOF;
  else
    c = *(b->position);
  (b->position)++;
  return c;
}

/* undo last read operation on buffered file b */
void
bf_ungetc (buffered_file * b)
{
  (b->position)--;
}

/* buffered files list */
List bft;

/* IF BLOCKS STATUS */

/* a block is either an IF or ELSE block
 * elif are treated same as if
 */
#define BLOCK_IF        0x0001
#define BLOCK_ELSE      0x0002

/* Have the instructions within block to be treated or
 * discarded by preprocessor ? 
 */
#define TREAT           0x0004	/* treat instructions */
#define DONT_TREAT      0x0008	/* discard instructions */
#define ELIF_DONT_TREAT 0x0010	/* special value used for elif blocks */
#define SKIP            0x0020	/* discard instructions and the block itself 
				   typically, when several #if blocks are
				   included and the outermost is evaluated to false
				 */

/* IF blocks stack */
typedef struct xif_block
{
  int status;
} if_block;

if_block *
new_if_block (int status)
{
  if_block *new = (if_block *) xmalloc (sizeof (if_block));
  new->status = status;
  return new;
}

List if_stack;

int
last_if_stack ()
{
  ListElem e = get_last (if_stack);
  assert (e);
  return ((if_block *) object (e))->status;
}

enum macro_type
{
  KEEN_MACRO_OBJECTLIKE,
  KEEN_MACRO_FUNCTIONLIKE
};

typedef struct xmacro
{
  int type;			// function-like or object-like
  char *name;			// name of the macro
  int pnum;			// number of parameters, if function-like
  char *rl;			// template for substitution
  int applied;			// internal flag to indicate if macro has already been applied in substitution pass
  int ellipsis;			// flag for ellipsis
} macro;

/* macro chained list */
List macro_list;

/* returns macro with name given as argument, of NULL if not found */
macro *
get_macro_by_name (char *name)
{
  macro *m;
  FOR_EACH (m, macro_list, e) if (strcmp (name, m->name) == 0) return m;
  return NULL;
}

void
macro_unapply_all (void)
{
  macro *m;
  FOR_EACH (m, macro_list, e) m->applied = 0;
}

/* creates a macro entry with macro name set. Other fields should be
 * filled manually afterwards. Returns a pointer to the newly created
 * entry. No test on macro name is done (supposed to be unique) */
macro *
declare_macro (char *name)
{
  macro *new = (macro *) xmalloc (sizeof (macro));
  new->name = string (name);
  new->applied = 0;
  new->rl = NULL;
  return new;
}

void
predefine_macro (char *name, char *value)
{
  macro *m;
  // delete previous definition if any
  if (m = get_macro_by_name (name))
    {
      find_n_drop (macro_list, m);
    }
  m = declare_macro (name);
  m->type = KEEN_MACRO_OBJECTLIKE;
  m->rl = string (value);
  add (macro_list, m);
}

/* removes a macro entry, provided is name. If no macro exists with
 * that name, does nothing */
void
undef_macro (char *name)
{
  macro *m = get_macro_by_name (name);
  find_n_drop (macro_list, m);
  if (m)
    {
      free (m->name);
      free (m);
    }
}

/* return the buffered_file corresponding to filename,
 * or NULL if not found */
buffered_file *
get_buffered_file_by_name (char *filename)
{
  buffered_file *f;
  FOR_EACH (f, bft, e) if (strcmp (filename, f->filename) == 0) return f;
  return NULL;
}

/* set all files as unused and remove duplicate entries */
void
clean_buffered_files (void)
{
  ListElem e = get_first (bft);
  while (e)
    {
      ListElem next = get_next (e);
      buffered_file *f = object (e);
      if (f->duplicate)
	{
	  drop (bft, e);
	  free (f->filename);
	  free (f);
	}
      else
	{
	  f->in_use = 0;
	  f->position = f->text;	// reset position counter
	}
      e = next;
    }
}

/* write string into the translation unit buffer. The trailing '\0' is
 * written too. The translation unit buffer is expanded if necessary */
void
write_string_to_output_buffer (char *string)
{
  if (tu_pos + strlen (string) + 1 > tu_end)
    {
      // need to allocate more space
      int position = tu_pos - tu;
      int required_space =
	(((strlen (string) + 1 - (tu_end - tu_pos)) / OUTPUT_BUFFER_BLOCK) +
	 1) * OUTPUT_BUFFER_BLOCK;
      required_space += tu_end - tu;
      tu = (char *) xrealloc (tu, required_space);
      tu_end = tu + required_space;
      tu_pos = tu + position;
    }
  strcpy (tu_pos, string);
  tu_pos += strlen (string);
}

/* write c as many times as requested into the translation unit buffer. A ending '\0' is
 * written afterwards. The translation unit buffer is expanded if necessary */
void
write_char_to_output_buffer (char c, int times)
{
  if (tu_pos + times + 1 > tu_end)
    {
      // need to allocate more space
      int position = tu_pos - tu;
      int required_space =
	(((times + 1 - (tu_end - tu_pos)) / OUTPUT_BUFFER_BLOCK) +
	 1) * OUTPUT_BUFFER_BLOCK;
      required_space += tu_end - tu;
      tu = (char *) xrealloc (tu, required_space);
      tu_end = tu + required_space;
      tu_pos = tu + position;
    }
  for (; times > 0; times--, tu_pos++)
    {
      *tu_pos = c;
    }
  *tu_pos = '\0';
}

/* simply returns a pointer to the translation unit buffer */
char *
get_preprocess_buffer (void)
{
  return tu;
}

/* write file into buffered_file entry. We read the whole file */
void
map_file_to_memory (buffered_file * bf, char *filename, int nesting_level)
{
  ssize_t nb_char;
  ssize_t requested_read;
  //char buffer[BUFFER_READ_SIZE];
  // open file
  int fd = open (filename, O_RDONLY);
  if (fd < 0)
    {
      keen_error ("cannot open %s", filename);
      exit (EXIT_FAILURE);
    }
  // free current buffer if already used
  if (bf->text)
    free (bf->text);
  bf->text = (char *) xmalloc (BUFFER_READ_SIZE);
  bf->size = 0;
  bf->limit = BUFFER_READ_SIZE;
  requested_read = BUFFER_READ_SIZE;
  // read file until EOF, error, or MAX_BUFFER_SIZE is reached
  while ((nb_char = read (fd, bf->text + bf->size, requested_read)) > 0)
    {
      bf->size += nb_char;
      if (nb_char == requested_read)
	{
	  // the buffer is full and there are input chars left
	  requested_read = BUFFER_READ_SIZE;
	  bf->limit += BUFFER_READ_SIZE;
	  bf->text = (char *) xrealloc (bf->text, bf->limit);
	}
      else
	{
	  // try to fill in the current buffer before allocating
	  // a larger one
	  requested_read -= nb_char;
	}
    }
  if (nb_char < 0)
    keen_error ("error while reading source file");
  close (fd);
  // Store other information on file 
  if (bf->filename)
    free (bf->filename);
  bf->filename = (char *) xmalloc (strlen (filename) + 1);
  strcpy (bf->filename, filename);
  // indicate that file is in use
  bf->in_use = 1;
  bf->position = bf->text;
  bf->duplicate = 0;
  bf->nesting_level = nesting_level;
}

/* Do some initialization stuff with preprocessor. This function
 * MUST be called before any use of the preprocessor */
void
init_preprocess (void)
{

  /* creates new empty macro list */
  macro_list = newList ();

  /* creates include directory path search (for #include) */
  include_search_list = newList ();

  char *inclist = string (INCLUDE_SEARCH);
  char *c = inclist;

  while (c)
    {

      /* split include path in components separated by : */
      char *d = strchr (c, ':');
      if (d) *d = 0;

      /* if search path contains special character '*', replace with actual directory */
      char *new_path = c, *special;

search_for_star:

      special = new_path;
      while (*special && *special != '*') special++;
      if (*special == '*')
        {
	  *special = 0; // remove the star
          special++; // go one further

	  // search for a possible directory
          struct dirent **namelist;
 	  int n = scandir ( new_path , &namelist, 0 , alphasort);
          int found = 0;
          while (--n) 
            {
              if (strcmp(namelist[n]->d_name,".")==0) { free(namelist[n]); continue; }
              if (strcmp(namelist[n]->d_name,"..")==0) { free(namelist[n]); continue; }
	      if (!found) { new_path = stringf("%s%s%s", new_path , namelist[n]->d_name, special); found = 1; }
	      free (namelist[n]);
            }
	  free (namelist);

          goto search_for_star; /* maybe not the only one, search again */
        }

      /* add this patch to the final list */
      add (include_search_list, string(new_path) );

      if (d) c = d + 1; else c = 0;

    }
  free (inclist);

  /* initialize buffered file table */
  bft = newList ();

  /* error reporting level is file:line in preprocessor */
  SET_LOC (location_line);
}

char *
init_preprocess_buffer (void)
{
  if (tu)
    free (tu);
  tu = (char *) xmalloc (OUTPUT_BUFFER_BLOCK);
  tu_pos = tu;
  tu_end = tu + OUTPUT_BUFFER_BLOCK;
}

#define is_identifier(c)	(isalnum(c)||(c)=='_')
#define is_punctuator(c)	(!(is_identifier(c)||isspace(c)))

char *
get_next_id_token (char *start, char **next)
{
  char *p, *start_id;
  int in_identifier = 0;
  int in_string = 0;
  int in_char = 0;
  int in_number = 0;
  int escaped = 0;
  for (p = start; *p; p++)
    {
      // already in string
      if (in_string)
	{
	  if (!escaped && *p == '"')
	    {
	      // end of string !
	      in_string = 0;
	      continue;
	    }
	  if (escaped)
	    {
	      escaped = 0;
	      continue;
	    }
	  if (*p == '\\')
	    {
	      escaped = 1;
	      continue;
	    }
	  continue;
	}
      // already in char
      if (in_char)
	{
	  if (!escaped && *p == '\'')
	    {
	      // end of char !
	      in_char = 0;
	      continue;
	    }
	  if (escaped)
	    {
	      escaped = 0;
	      continue;
	    }
	  if (*p == '\\')
	    {
	      escaped = 1;
	      continue;
	    }
	  continue;
	}
      // already in identifier
      if (in_identifier)
	{
	  if (!is_identifier (*p))
	    {
	      *next = p;
	      keen_debug ("get_next_id:'%s'\n", start_id);
	      return start_id;
	    }
	  continue;
	}
      // already in number
      if (in_number)
	{
	  if (*p == 'l' || *p == 'L')
	    {
	      in_number = 0;
	      continue;
	    }
	  if (!isdigit (*p))
	    {
	      p--;
	      in_number = 0;
	      continue;
	    }
	}

      // else normal text
      if (is_identifier (*p) && !isdigit (*p))
	{
	  start_id = p;
	  in_identifier = 1;
	  continue;
	}
      if (*p == '"')
	{
	  in_string = 1;
	  continue;
	}
      if (*p == '\'')
	{
	  in_char = 1;
	  continue;
	}
      if (isdigit (*p))
	{
	  in_number = 1;
	  continue;
	}
      // else
      in_string = 0;
      in_char = 0;
      in_number = 0;
    }

  if (in_identifier)
    {
      *next = p;
      keen_debug ("get_next_id:'%s'\n", start_id);
      return start_id;
    }

  // else, no identifier found
  *next = p;
  keen_debug ("get_next_id:NULL\n");
  return NULL;
}

#define SKIP_BLANK_B(p) while(*p && isblank(*p)) p++; \
	 		if (!*p) break;

#define SKIP_BLANK(p) 	while(*p && isblank(*p)) p++;

typedef List stringList;

/* search for string s in list l. Return -1 if not found,
 * or the index whithin l if found
 */
int
string_exist (stringList l, char *s)
{
  ListElem e = get_first (l);
  int i = 0;
  while (e)
    {
      if (strcmp (s, object (e)) == 0)
	return i;
      i++;
      e = get_next (e);
    }
  return -1;
}

/*
 * Given raw_string the raw macro definition string, determine final template string
 * with # and ## taken into account, and - if function macro - parameters translated
 * to %n sequences where n is the index of the parameter plus 1 (ie. %1 represents
 * the parameter index 0)
 * the special marker %0 is reserved for ellipsis replacement (__VA_ARGS__)
 * to avoid collisions with existing % characters, % is written twice %%
 * when originally present in raw_string 
 * this routine works for both function- and object-macros
 */
char *
determine_macro_template (char *raw_string, stringList parameters, int ellipsis_expected)
{

  char *p, *next;
  int i = 0;
  int space_required = 0;
  int quotes_required = 0;	// for # command

  /* conservative approx. of needed size: if all parameters are only 1 char long
   * and replaced by a sequence of x digits */
  int needed_digits = size (parameters);
  char *rstring = (char *) xmalloc (strlen (raw_string) * (needed_digits + 1));

  p = raw_string;
  SKIP_BLANK (p);
  while (*p)
    {
      if (is_identifier (*p) && !isdigit (*p))
	{
	  // candidate for parameter replacement
	  next = p;
	  while (is_identifier (*next)) next++;
	  char *candidate = stringn (p, next);
	  int index;
	  if (ellipsis_expected && strcmp("__VA_ARGS__",candidate)==0)
	    {
	    	index = 0;
	    }
	  else
	    {
	  	index = string_exist (parameters, candidate);
	  	if (index == -1)
	    	{
	      	  // not a parameter. Copy as if it was raw text
	      	  while (p < next)
			{
			if (isspace(*p)) { if (rstring[i-1] == ' ') p++; else rstring[i++] = ' '; continue; }
			rstring[i++] = *p;
		  	if (*p == '%') rstring[i++] = '%';
		  	p++;
			}
	        }
		else index++;
	    }
	  if (index != -1) 
	    {
	      // this is a parameter. Replace it with a tag
	      if (quotes_required) rstring[i++] = '\"';
	      rstring[i++] = '%';
	      char *si = stringf ("%d", index);
	      strcpy (rstring + i, si);
	      i += strlen (si);
	      free (si);
	      if (quotes_required) rstring[i++] = '\"';
	      quotes_required = 0;	// in any case
	      p = next;
	    }
	}
      else if (*p == '#')
	{
	  p++;
	  if (*p == '#')
	    {
	      // concat command
	      p++;
	      SKIP_BLANK (p);
	    }
	  else
	    {
	      // string command
	      quotes_required = 1;
	    }
	}
      else
	{
	  // raw text
	  if (isspace(*p)) { if (rstring[i-1] == ' ') p++; else rstring[i++] = ' '; continue; }
	  rstring[i++] = *p;
	  if (*p == '%') rstring[i++] = '%';
	  p++;
	}

    }

  /* remove trailing blanks if any */
  while (i > 0 && isblank (rstring[i - 1])) i--;

  rstring[i] = 0;
  return rstring;
}

/* parse and create a function-macro based on string s, with name macro_name
 * s must point at the opening ( parenthesis of the macro
 * returns the newly created macro
 */
macro *
declare_function_macro (char *macro_name, char *s)
{

  keen_debug ("declare_function_macro() called on '%s'\n", macro_name);

  assert (*s == '(');
  s++;

  // initialize macro
  macro *m = (macro *) xmalloc (sizeof (macro));
  m->name = string (macro_name);
  m->type = KEEN_MACRO_FUNCTIONLIKE;
  m->ellipsis = 0; // a priori, may be changed below

  // get all parameters
  stringList parameters = newList ();
  char *p = s, *next, c;
  int need_parameter = 0;
  while (*p && *p != ')')
    {
      SKIP_BLANK_B (p);

      if (*p == '.' && *(p+1) == '.' && *(p+2) == '.')
        {
	  m->ellipsis = 1;
	  p+=3;
      	  need_parameter = 0;
          SKIP_BLANK_B (p);
	  continue;
	}        
      else if (!is_identifier (*p) || isdigit (*p))
	keen_error ("not a valid parameter in macro definition");

      next = p;
      while (is_identifier (*next))
	next++;
      char *param = stringn (p, next);
      // before adding, check that parameter does not already exist
      if (string_exist (parameters, param) != -1)
	keen_error ("parameter %s already exist", param);
      add (parameters, param);	// add new parameter
      p = next;
      need_parameter = 0;
      SKIP_BLANK_B (p);
      switch (*p)
	{
	case ',':
	  need_parameter = 1;
	  p++;
          SKIP_BLANK_B (p);
	  continue;
	case ')':
	  continue;
	default:
	  keen_error ("syntax error in macro definition");
	}
    }
  if (need_parameter)
    keen_error ("a parameter is missing in macro definition");
  if (!*p)
    keen_error ("missing closing parenthesis ) in macro definition");
  assert (*p == ')');
  p++;

  m->pnum = size (parameters);

  // determine result string
  // each parameter is replaced with a sequence %n where n is index
  // of the parameter in param list
  m->rl = determine_macro_template (p, parameters,m->ellipsis);

  keen_debug ("declaring macro '%s' with template '%s'\n", m->name, m->rl );

  freel (parameters);

  return m;
}

char *macro_replacement_on_zone ();

char *
macro_replacement_on_identifier (char *token, macro * mac, char *limit,
				 int force)
{

  keen_debug ("MAC_ID:'%s',mac=%d,force=%d\n", token, mac, force);
  int needed, available;

  if (!mac && force)
    {
      // identifier is not a valid macro
      // but we force it to zero "0" anyway
      needed = 1;
      available = strlen (token);
      // FIXME: allocate more space when needed
      memmove (token + needed, token + available, limit - token - available);
      strcpy (token, "0");
      limit += needed - available;
      *limit = '\0';
      keen_debug ("REPLACED='%s'\n", token);
      return limit;
    }

  char *p, *q;
  int need_parameter = 0;
  stringList parameters = newList ();

  // else proceed with replacement
  switch (mac->type)
    {

    case KEEN_MACRO_OBJECTLIKE:
      if (mac->rl)
	{
	  needed = strlen (mac->rl);
	}
      else
	{
	  needed = 0;
	}
      available = strlen (token);
      // FIXME: allocate more space when needed
      memmove (token + needed, token + available, limit - token - available);
      if (mac->rl)
	{
	  strcpy (token, mac->rl);
	}
      limit += needed - available;
      *limit = '\0';
      break;

    case KEEN_MACRO_FUNCTIONLIKE:

      // get parameters

      /* get on opening parenthesis */
      p = token;
      while (*p && *p != '(') p++;
      if (*(p+1)==')') goto no_param;

      /* read params one by one until closing parenthesis found */
      int nesting_level = 0;
      int on_start = 1;
      while (*p)
	{

	  SKIP_BLANK_B (p);

	  // check if no parameter at all
	  if (*p == ')' && !need_parameter)
	    break;

	  // else
	  q = p;
	  int exit = 0;

	  while (*q && !exit)
	    {
	      switch (*q)
		{
		case '(':
		  nesting_level++;
		  q++;
		  break;
		case ')':
		  nesting_level--;
		  if (nesting_level == 0)
		    exit = 1;
		  else
		    q++;
		  break;
		case '"':	/* eat string */
		  q++;
		  while (*q != '"')
		    {
		      if (*q == '\\')
			{
			  q++;
			  if (*q == '"')
			    q++;
			}	// ignore escaped "
		      q++;
		    }
		  q++;
		  break;
		case '\'':	/* eat char sequence */
		  q++;
		  while (*q != '\'')
		    {
		      if (*q == '\\')
			{
			  q++;
			  if (*q == '\'')
			    q++;
			}	// ignore escaped '
		      q++;
		    }
		  q++;
		  break;
		case ',':
		  if (nesting_level == 1)
		    exit = 1;
		  else
		    q++;
		  break;
		default:
		  q++;
		}
	    }
	  // we should be either on comma or closing parenthesis here
	  if (!*q || (*q != ',' && *q != ')'))
	    keen_error ("no matching parenthesis in macro call");

	  char *param;
	  if (*q == ',' || *q == ')')
	    {
	      char *save_q = q;
	      if (on_start)
		{
		  p++;
		  SKIP_BLANK (p);
		  on_start = 0;
		}
	      // remove trailing blanks if any
	      if (q > p)
		{
		  while (isblank (*(q - 1)))
		    q--;
		}
	      if (p == q)
		{
		  param = NULL;
		  	// parameter is empty string
		}
	      else
		{
		  char temporary_string[4096];

		  param = stringn( p,q );
			/* we get the parameter */

		  strcpy( temporary_string, param);
			/* we copy the parameter before substition into an allocated array */

		  char *end = temporary_string + strlen(temporary_string);
		  end = macro_replacement_on_zone (temporary_string, end, 0, force); 
		  *end = '\0';
			/* we perform substitution onto this parameter before we store it */

		  param = string (temporary_string);
			/* we store the (maybe transformed) parameter */
		}

	      // add this parameter to list
	      add (parameters, param);
	      // go beyond last comma if new parameter to search for
	      if (*save_q == ',')
		{
		  p = save_q + 1;
		  need_parameter = 1;
		}
	      else
		{
		  need_parameter = 0;
		  p = save_q;
		}
	    }

	}

      if (!*p)
	keen_error ("no closing parenthesis ')' in macro call");

no_param:

      // check number of parameters
      if (!mac->ellipsis && mac->pnum != size (parameters))
	{
	  keen_error ("macro %s is called with %d parameters but requires %d",
		      mac->name, size (parameters), mac->pnum);
	}

      // apply template
      char applied[4096];
      int i = 0;
      p = mac->rl;
      while (*p)
	{
	  if (*p == '%')
	    {
	      p++;
	      if (*p == '%')
		{
		  applied[i++] = '%';
		  p++;
		}
	      else if (isdigit (*p))
		{
		  int rank;
		  sscanf (p, "%d", &rank);
		  while (isdigit (*p)) p++;
		  if (rank == 0)
		    {
			// ellipsis replacement
			if (size(parameters) > mac->pnum) 
			{
			 ListElem surnumerous = get_ith ( parameters, mac->pnum);
			 while (surnumerous)
			  {
		            char *replace = object (surnumerous);
		            if (replace)
		              {
		      	        strcpy (applied + i, object (surnumerous));
		      	        i += strlen (object(surnumerous));
			      }
			    surnumerous = get_next (surnumerous);
			    if (surnumerous) 
				{
		      		  applied[i++] = ',';
				}
			  }	
			}
		    }
		  else 
		    {
		      rank--;
		      char *replace = object (get_ith (parameters, rank));
		      if (replace)
		        {
		          strcpy (applied + i, object (get_ith (parameters, rank)));
		          i += strlen (object (get_ith (parameters, rank)));
			}
		    }
		}
	      else
		assert (0);
	    }
	  else
	    {
	      applied[i++] = *p++;
	    }
	}
      applied[i] = 0;

      needed = strlen (applied);
      available = strlen (token);

      // FIXME: allocate more space when needed
      memmove (token + needed, token + available, limit - token - available);
      strcpy (token, applied);
      limit += needed - available;
      *limit = '\0';

      break;

    default:
      assert (0);

    }				/* end switch */

  mac->applied = 1;
	/* before we recursively call for macro substitution on (newly created) zone,
 	   we flag this particular macro as already applied to avoid infinite loop.
	   We will clean the flag afterwards, as we get back in the recursion */
  
  limit = macro_replacement_on_zone (token, limit, 0, force);
  *limit = '\0';

  mac->applied = 0;
	/* clean flag. This macro may be applied several times, provided this is not
 	   recursively on same zone */

  return limit;

}

char *
macro_replacement_on_zone (char *start, char *limit, int unapply, int force)
{

  char *token, *next, *new_end, *p, *q;
  char c;


  while ((token = get_next_id_token (start, &next)) != NULL)
    {

      if (unapply) macro_unapply_all ();
      keen_debug ("ZONE:(%d) '%s'\n", strlen (token), token);

      /* try to match this id with a macro */
      c = *next;
      *next = '\0';
      macro *mac = get_macro_by_name (token);

      /* indicates if a macro substitution has finally been performed */
      int was_replaced = 0;

      /* the id does not match a macro, but we anyway replace it with
       * an integer value 0 */
      if (!mac && force)
	{
	  limit = macro_replacement_on_identifier (token, NULL, limit, force);
	  was_replaced = 1;
	}

      /* the id matches a macro, and this macro was not already applied */
      if (mac && !mac->applied)
	{

	  switch (mac->type)
	    {

	    case KEEN_MACRO_OBJECTLIKE:
	      limit =
		macro_replacement_on_identifier (token, mac, limit, force);
	      was_replaced = 1;
	      break;

	    case KEEN_MACRO_FUNCTIONLIKE:
	      *next = c;	// restore last char
	      p = next;
	      SKIP_BLANK (p);
	      if (*p != '(')
		{
		  // not a macro finally !
		  was_replaced = 0;
		  break;
		}
	      keen_debug ("ZONE:about to find params for '%s'\n", p);
	      // p points on opening (
	      q = p;
	      // find closing parenthesis
	      int exit = 0, level = 0;
	      while (*q && !exit)
		{
		  switch (*q)
		    {
		    case '(':
		      level++;
		      q++;
		      break;
		    case ')':
		      level--;
		      if (level == 0)
			exit = 1;
		      else
			q++;
		      break;
		    case '"':	/* eat string */
		      q++;
		      while (*q != '"')
			{
			  if (*q == '\\')
			    {
			      q++;
			      if (*q == '"')
				q++;
			    }	// ignore escaped "
			  q++;
			}
		      q++;
		      break;
		    case '\'':	/* eat char sequence */
		      q++;
		      while (*q != '\'')
			{
			  if (*q == '\\')
			    {
			      q++;
			      if (*q == '\'')
				q++;
			    }	// ignore escaped '
			  q++;
			}
		      q++;
		      break;
		    default:
		      q++;
		    }
		}
	      if (!*q || level != 0)
		keen_error ("missing closing parenthesis in macro call");
	      next = q + 1;
	      c = *next;
	      *next = 0;
	      // perform replacement
	      keen_debug ("ZONE:launch on_id for '%s'\n", token);
	      limit =
		macro_replacement_on_identifier (token, mac, limit, force);
	      was_replaced = 1;
	      break;

	    default:
	      assert (0);
	    }

	}

      if (was_replaced)
	{
	  *limit = '\0';
	  new_end = start + strlen (start);
	  *new_end = c;
	  start = new_end;
	}
      else
	{
	  *next = c;
	  start = next;
	}

      keen_debug ("ONZONE:will loop on '%s'\n", start);

    }

  return limit;
}

/* perform macro substitution on string start.
 * if force is 1, all identifiers are substituted with a value, even
 * unknown macros which are replaced with "0". This is useful for 
 * properly evaluating #if directives in which all identifiers must
 * be replaced with a valid integer value.
 */
void
macro_replacement (char *start, int force)
{
  macro_replacement_on_zone (start, start + strlen (start), 1, force);
}

/* write next logical line into global buffer logical_line[].
 * A logical line is a line 
 * - with comments replaced by one space,
 * - with well formed parenthesis (all opening parenthesis have a
 *   corresponding closing one) 
 * - all trail characters \ processed (several actual lines in initial file are
 * concatened). 
 * Note that one logical line may thus represent several lines in original 
 * file. This number of lines is returned by the function (or 0 if no new 
 * line found)
 */
int
get_new_logical_line (buffered_file * bf, int lineno)
{

  int c; 
  int i = 0, save_i = 0; 
	/* index in logical_line[] buffer */

  int linefeed = 0;
	/* number of lines the logical_line represents in file */

  int tray_on_previous = 0;
	/* flag if last read gave a trail character \ */

  int parenthesis_level = 0;
  int first_opening_parenthesis_line = 0;
	/* track parenthesis level and first appearance of parenthesis */

  while ((c = bf_getc (bf)) != EOF)
    {

	/* previous raw line ended with a tray. This was treated during
	past iteration and flag is now set to normal */
	if (tray_on_previous) tray_on_previous = 0;
		
	/* FOUND CARRIAGE RETURN */
	// This is probably the end of the logical line, except if there is a 
	// pending parenthesis 
	// Note that this is the only point in the loop where we can break
	if (c == '\n')
	  {
	  
	  linefeed++;
          
	  if (parenthesis_level == 0) 
	    {
		break;
            }
	  else
            {
		// if pending parenthesis, replace the newline 
		// with one space and continue parsing
		logical_line[i++] = ' ';
		continue;
            }
	  }  

	/* MAYBE FOUND A COMMENT */
	if (c == '/')
	  {
		c = bf_getc (bf);
		
		// IS A STANDARD /* */ COMMENT
		if (c == '*')
	    	{
	      	// Find the end. No character will be written
		// to buffer, except that the whole comment is replaced by a single space
	      	while ((c = bf_getc (bf)) != EOF)
		  {
		    if (c == '\n') linefeed++; // we don't break even if we find a carriage return
		    if (c == '*')
		    {
		   	c = bf_getc (bf);
		 	if (c == '/') break; 
				// we found the sequence '*/' that marks the end of the comment. We
				// break this loop and gets below to write a space
			bf_ungetc(bf);
				// if we get here previous sequence was not the end of the comment. 
				// We put last read char and continue searching for closing comment 
				// sequence
		    }
		  }
		// we have found the end of the comment, or the end of the file !
		if (c == EOF) keen_error ("a comment /* is not properly closed");
	      	else
			{
			  // replace comment with one space
			  logical_line[i++] = ' ';
			  continue;
			}
	    	}
		
	    	// IS A // COMMENT
	    	else if (c == '/')
	    	{
	      	// Find the end-of-line. No character will be written to buffer except
		// the whole comment is replaced with a single space
	      	while ( ((c = bf_getc (bf)) != EOF) && c != '\n');
	
		// replace comment with one space
		if (c == '\n') bf_ungetc(bf);
	      	logical_line[i++] = ' ';
	      	continue;
		  
	    	}
		
	    	else
	    	{
		
	      	// not a comment finally, treat this character as usual...
	      	logical_line[i++] = '/';
	      	bf_ungetc (bf);
		  
		continue;
		  
	    	}
		
	  } // end of comment parsing

	/* MAYBE FOUND A TRAY */
      	if (c == '\\')
	  {
		c = bf_getc (bf);
		if (c == '\n')
	    	{
	      		// found a tray. Ignore carriage return
	      		linefeed++;
	      		tray_on_previous = 1;
	      		continue;
	    	}
	    	else
	    	{
	      		logical_line[i++] = '\\';
	      		bf_ungetc (bf);
	      		continue;
	    	}
	  }

      	// FOUND CHARACTER SEQUENCES...
      	else if (c == '\'')
        {
          logical_line[i++] = c;
	  while ((c = bf_getc (bf)) != EOF)
	      {
	     	if (c == '\n') keen_error ("unexpected carriage-return in character sequence");
		if (c == EOF) keen_error ("unexpected end-of-file in character sequence");
	        logical_line[i++] = c;
		if (c == '\'') break;
              }
	  continue;
	}

      	// FOUND STRING...
      	else if (c == '"')
        {
		
	  logical_line[i++] = '"'; 

	  while ((c = bf_getc (bf)) != EOF)
	    {
          	if (c == '"') break; // end of string
	      	if (c == '\n') keen_error ("unexpected carriage-return in string");
	      	if (c == '\\')
                {
                  c = bf_getc (bf);
                  if (c == '\n')
                    {
                      // found a tray
                      linefeed++;
                      continue;
                    }
                  else
                    {
                      logical_line[i++] = '\\';
                    }
                }
	      	logical_line[i++] = c;
            }

	  logical_line[i++] = '"'; 
	  continue;
	}
		
      	// FROM THIS POINT WE ARE OUTSIDE COMMENTS, STRINGS AND CHARACTER SEQUENCES
	  
	// treat parenthesis
      if (c == '(')
        {
          if (parenthesis_level==0) first_opening_parenthesis_line = lineno + linefeed +1;
          parenthesis_level++;
        }
      else if (c == ')')
        {
          parenthesis_level--;
          if (parenthesis_level<0) keen_warning ("unexpected closing parenthesis");
        }
	  
      // at the end, append character
      logical_line[i++] = c;
      
    }

  if (c == EOF && tray_on_previous)
    {
      keen_error ("a file must not end with a tray \\");
    }

  if (c == EOF && parenthesis_level != 0)
    {
      int save = GET_LINE();
      SET_LINE( first_opening_parenthesis_line);
      keen_warning ("unclosed parenthesis");
      SET_LINE ( save );
    }

  // remove last \r character if any
  if (i > 0 && logical_line[i - 1] == '\r')
    {
      i--;
    }

  // add trailing \0
  logical_line[i] = 0;

  if (c == EOF)
    keen_debug ("LL=EOF\n");
  else
    keen_debug ("LL(%d)(%d)='%s'\n", i, linefeed, logical_line);

  return linefeed;

}


/* preprocessor routine  */
void
do_preprocess (char *filename, int close_buffer, int nesting_level)
{


  int lineno = 0;
  int linefeed = 0;
  int saved_line = 0;
  char *saved_filename = NULL;
  char *p, *q, *r, *next;
  char c;
  macro *mac;
  buffered_file *bf = NULL;
  buffered_file *bf_dup = NULL;
  char *echo_line_no = (char *) xmalloc (strlen (filename) + 128);
  int value;
  int paren;

  keen_debug ("Now including file %s\n", filename);

  // CHECK NESTING LEVEL
  if (nesting_level > MAX_INCLUDE_DEPTH)
    {
      keen_error ("too many nested files: %s", filename);
    }

  // MAP FILE TO MEMORY
  SET_LOC (location_line);
  SET_FILE (string (filename));
  SET_LINE (1);

  // check if the requested file is already in memory
  bf = get_buffered_file_by_name (filename);
  if (bf == NULL)
    {

      // new file, put it into list 
      bf = (buffered_file *) xcalloc (sizeof (buffered_file), 1);
      add (bft, bf);

      // write entire file to memory
      map_file_to_memory (bf, filename, nesting_level);

    }
  else
    {
      if (bf->in_use)
	{
	  bf_dup = (buffered_file *) xcalloc (sizeof (buffered_file), 1);
	  bf_dup->filename = bf->filename;
	  bf_dup->text = bf->text;
	  bf_dup->position = bf->text;
	  bf_dup->size = bf->size;
	  bf_dup->limit = bf->limit;
	  bf_dup->duplicate = 1;
	  bf_dup->in_use = 1;
	  bf_dup->nesting_level = nesting_level;
	  add (bft, bf_dup);
	  bf = bf_dup;
	}
      else
	{
	  // reinit buffered_file
	  bf->in_use = 1;
	  bf->nesting_level = nesting_level;
	  bf->position = bf->text;
	}
    }

  // PREPROCESS

  // trace first line 
  sprintf (echo_line_no, "#1,%s\n", GET_FILE ());
  write_string_to_output_buffer (echo_line_no);


  // Read the file one line at a time and produce logical lines
  lineno = 0;
  while (linefeed = get_new_logical_line (bf, lineno))
    {

      lineno += linefeed;
      SET_LINE (lineno);

      /* check if line is null */
      p = logical_line;
      if (*p == 0)
	goto print_and_go_on;

      /* check if line is empty or totally blank */
      if (isblank (*p))
	{
	  SKIP_BLANK (p);
	  if (!*p)
	    {
	      // nothing to do
	      goto print_and_go_on;
	    }
	  else
	    {
	      // not blank. rewind
	      p = logical_line;
	    }
	}

      /* CHECK FOR PREPROCESSOR DIRECTIVE */
      if (*p == '#')
	{
	  p++;
	  SKIP_BLANK (p);
	  if (!*p)
	    {
	      // "# \n" directive. nothing to do
	      goto end_directive;
	    }
	  if (!is_identifier (*p) || isdigit (*p))
	    {
	      keen_debug
		("invalid syntax in preprocessor directive. Line ignored\n");
	      keen_debug ("(%s)\n", logical_line);
	      goto end_directive;
	    }

	  get_next_id_token (p, &next);
	  c = *next;
	  *next = '\0';

	  if (strcmp (p, "ifdef") == 0)
	    {
	      *next = c;
	      for (p = next; *p && isblank (*p); p++);
	      if (!*p)
		{
		  keen_error ("uncomplete #ifdef directive");
		}
	      if (!is_identifier (*p) || isdigit (*p))
		{
		  keen_error ("not a valid identifier");
		}
	      get_next_id_token (p, &next);
	      c = *next;
	      *next = '\0';
	      if (size (if_stack) > 0
		  && (((if_block *) object (get_last (if_stack)))->
		      status & (SKIP | DONT_TREAT)))
		{
		  add (if_stack, new_if_block (BLOCK_IF | DONT_TREAT | SKIP));
		}
	      else
		{
		  add (if_stack,
		       new_if_block (BLOCK_IF |
				     ((get_macro_by_name (p) !=
				       NULL) ? TREAT : DONT_TREAT)));
		}
	      goto end_directive;
	    }

	  if (strcmp (p, "ifndef") == 0)
	    {
	      *next = c;
	      for (p = next; *p && isblank (*p); p++);
	      if (!*p)
		{
		  keen_error ("uncomplete #ifndef directive");
		}
	      if (!is_identifier (*p) || isdigit (*p))
		{
		  keen_error ("not a valid identifier");
		}
	      get_next_id_token (p, &next);
	      c = *next;
	      *next = '\0';
	      if (size (if_stack) > 0
		  && (((if_block *) object (get_last (if_stack)))->
		      status & (SKIP | DONT_TREAT)))
		{
		  add (if_stack, new_if_block (BLOCK_IF | DONT_TREAT | SKIP));
		}
	      else
		{
		  add (if_stack,
		       new_if_block (BLOCK_IF |
				     ((get_macro_by_name (p) !=
				       NULL) ? DONT_TREAT : TREAT)));
		}
	      goto end_directive;
	    }

	  if (strcmp (p, "else") == 0)
	    {
	      *next = c;
	      if (is_empty (if_stack) || (last_if_stack () & BLOCK_ELSE))
		{
		  keen_error ("unexpected #else");
		}
	      if (last_if_stack () & SKIP)
		{
		  drop (if_stack, get_last (if_stack));
		  add (if_stack,
		       new_if_block (BLOCK_ELSE | DONT_TREAT | SKIP));
		}
	      else
		{
		  int status = last_if_stack ();
		  drop (if_stack, get_last (if_stack));
		  add (if_stack,
		       new_if_block (BLOCK_ELSE |
				     ((status & TREAT) ? DONT_TREAT :
				      TREAT)));
		}
	      for (p = next; *p && isblank (*p); p++);
	      if (*p)
		{
		  keen_error ("unexpected tokens after #else directive");
		}
	      goto end_directive;
	    }

	  if (strcmp (p, "endif") == 0)
	    {
	      *next = c;
	      if (is_empty (if_stack))
		{
		  keen_error ("unexpected #endif");
		}
	      drop (if_stack, get_last (if_stack));
	      for (p = next; *p && isblank (*p); p++);
	      if (*p)
		{
		  keen_error ("unexpected tokens after #endif directive");
		}
	      goto end_directive;
	    }

	  if (strcmp (p, "if") == 0 || strcmp (p, "elif") == 0)
	    {

	      /* elif block will replace previous if block
	       * then we will evaluate it as an "if"
	       */
	      if (strcmp (p, "elif") == 0)
		{
		  if (is_empty (if_stack) || (last_if_stack () & BLOCK_ELSE))
		    keen_error ("unexpected #elif");
		  if (last_if_stack () & SKIP)
		    {
		      /* previous IF was ignored. And so this elif block */
		      goto end_directive;
		    }
		  else
		    {
		      /* previous IF was not skipped, we need to check if this elif clause
		       * has to be evaluated or not */
		      if (last_if_stack () & TREAT)
			{
			  /* elif must not be treated */
			  drop (if_stack, get_last (if_stack));
			  add (if_stack,
			       new_if_block (BLOCK_IF | TREAT |
					     ELIF_DONT_TREAT));
			  goto end_directive;
			}
		      else
			{
			  /* that elif block has to be treated as an if. Remove last if block before */
			  drop (if_stack, get_last (if_stack));
			}
		    }
		}

	      /* from here we consider both if and elif as a "if" block */
	      *next = c;
	      p = next;
	      SKIP_BLANK (p);
	      if (!*p)
		keen_error ("uncomplete directive");

	      if (!is_empty (if_stack)
		  && (last_if_stack () & (SKIP | DONT_TREAT)))
		{
		  add (if_stack, new_if_block (BLOCK_IF | DONT_TREAT | SKIP));
		}
	      else
		{
		  // eval "defined" keywords
		  q = get_next_id_token (next, &p);
		  while (q)
		    {
		      c = *p;
		      *p = '\0';
		      if (strcmp ("defined", q) == 0)
			{
			  *p = c;
			  // skip blanks, if any
			  SKIP_BLANK (p);
			  if (!*p)
			    {
			      keen_error
				("uncomplete defined() clause in #if directive");
			    }
			  // ignore parenthesis if any
			  paren = 0;
			  if (*p == '(')
			    {
			      paren = 1;
			      p++;
			    }
			  // get macro name & evaluate
			  SKIP_BLANK (p);
			  if (!*p)
			    {
			      keen_error
				("uncomplete defined() clause in #if directive");
			    }
			  if (!is_identifier (*p) || isdigit (*p))
			    {
			      keen_error
				("not a valid identifier in defined() clause");
			    }
			  get_next_id_token (p, &r);
			  c = *r;
			  *r = '\0';
			  value = (get_macro_by_name (p) != NULL);
			  *r = c;
			  // skip closing paren if needed
			  if (paren)
			    {
			      SKIP_BLANK (r);
			      if (!*r)
				{
				  keen_error
				    ("uncomplete defined() clause in #if directive");
				}
			      if (*r != ')')
				{
				  keen_error
				    ("missing closing ')' in defined clause");
				}
			    }
			  else
			    {
			      r--;
			    }
			  // replace "defined" keyword with value
			  if (value)
			    *q = '1';
			  else
			    *q = '0';
			  for (q++; q <= r; q++)
			    *q = ' ';
			}
		      else
			{
			  *p = c;
			}
		      q = get_next_id_token (p, &p);
		    }
		  // macro replacement
		  macro_replacement (next,
				     1
				     /* force substitution, even for unknown identifiers */
				     );
		  // evaluate expression
		  keen_debug ("EVAL='%s'\n", next);
		  add (if_stack,
		       new_if_block (BLOCK_IF |
				     ((prep_expr (next) !=
				       0) ? TREAT : DONT_TREAT)));
		}
	      goto end_directive;
	    }

	  if (!is_empty (if_stack))
	    {
	      // we are in an IF, ELSE or ELIF block
	      // check if the line has to be interpreted by preprocessor or simply discarded 
	      if (last_if_stack () & (DONT_TREAT | ELIF_DONT_TREAT))
		{
		  goto end_directive;
		}
	    }

	  if (strcmp (p, "include") == 0)
	    {
	      *next = c;

	      // proceed with macro replacement
	      // FIXME: not fully compliant because (a) macro replacement should not take
	      // place inside <file> ptoken, and (b) we should check syntax after macro
	      // replacement and before any other treatment
	      macro_replacement (next, 0 /* don't force */ );

	      int standard_include;
	      for (p = next; *p && isblank (*p); p++);
	      switch (*p)
		{
		case '<':
		  standard_include = 1;
		  break;
		case '"':
		  standard_include = 0;
		  break;
		case '\0':
		  keen_error ("uncomplete #include directive");
		default:
		  keen_error ("#include invalid syntax");
		}
	      p++;

	      // get end delimiter
	      for (q = p; *q && *q != ((standard_include) ? '>' : '"'); q++);
	      if (!*q)
		{
		  keen_error ("#include invalid syntax");
		}
	      *q = '\0';

	      // if standard include, search for file
	      char *candidate;
	      if (standard_include)
		{
		  int candidate_found = 0;
		  ListElem e = get_first (include_search_list);
		  while (e)
		    {
		      char *base = object (e);
		      candidate = stringf ("%s/%s", base, p);
		      int fd = open (candidate, O_RDONLY);
		      if (fd != -1)
			{
			  candidate_found = 1;
			  close (fd);
			  break;
			}
		      e = get_next (e);
		    }
		  if (!candidate_found)
		    {
		      keen_errorn ("from file %s:", GET_FILE ());
		      keen_error ("#include: <%s> not found", p);
		    }
		}
	      else
		{
		  candidate = string (p);
		}

	      // save current line and recursively call preprocessor
	      saved_line = GET_LINE ();
	      saved_filename = GET_FILE ();

	      do_preprocess (candidate, 0, nesting_level + 1);

	      // mark line when we return in current file
	      lineno = saved_line;
	      SET_FILE (saved_filename);
	      SET_LINE (lineno);

	      sprintf (echo_line_no, "#%d,%s\n", GET_LINE (), GET_FILE ());
	      write_string_to_output_buffer (echo_line_no);
	      goto end_directive;

	    }

	  if (strcmp (p, "define") == 0)
	    {
	      *next = c;
	      for (p = next; *p && isblank (*p); p++);
	      if (!*p)
		{
		  keen_error ("uncomplete #define directive");
		}
	      if (!is_identifier (*p) || isdigit (*p))
		{
		  keen_error ("macro name must be a valid identifier");
		}
	      get_next_id_token (p, &next);
	      c = *next;
	      *next = '\0';
	      macro *previous_macro_declaration = get_macro_by_name (p);
	      char *macro_name = string (p);
	      *next = c;
	      p = next;
	      if (*p == '(')
		{
		  // function-like macro
		  mac = declare_function_macro (macro_name, p);
		  keen_debug ("declare macro %s() -> '%s'\n", macro_name,
			      mac->rl);
		}
	      else
		{
		  // object-like macro
		  mac = declare_macro (macro_name);
		  mac->type = KEEN_MACRO_OBJECTLIKE;
		  mac->pnum = 0;
		  SKIP_BLANK (p);
		  if (*p)
		    {
		      for (q = p + strlen (p) - 1; isblank (*q); q--);
		      *(q + 1) = '\0';
		      mac->rl =
			determine_macro_template (p,
						  newList () /* no parameters */,
						  0 /* ellipsis not expected */ );
		    }
		  keen_debug ("declare macro %s -> '%s'\n", macro_name,
			      mac->rl);
		}
	      if (previous_macro_declaration)
		{
		  /* this macro is redeclared. Check for equality or warn user */
		  if (previous_macro_declaration->type != mac->type ||
		      previous_macro_declaration->pnum != mac->pnum ||
		      (previous_macro_declaration->rl == NULL
		       && mac->rl != NULL)
		      || (previous_macro_declaration->rl != NULL
			  && mac->rl == NULL)
		      || strcmp (previous_macro_declaration->rl,
				 mac->rl) != 0)
		    {

		      keen_warning ("macro %s is redeclared", macro_name);

		    }
		  /* delete previous declaration in any case */
		  find_n_drop (macro_list, previous_macro_declaration);
		}
	      add (macro_list, mac);
	      goto end_directive;
	    }

	  if (strcmp (p, "undef") == 0)
	    {
	      *next = c;
	      for (p = next; *p && isblank (*p); p++);
	      if (!*p)
		{
		  keen_error ("uncomplete #undef directive");
		}
	      if (!is_identifier (*p) || isdigit (*p))
		{
		  keen_error ("macro name must be a valid identifier");
		}
	      get_next_id_token (p, &next);
	      c = *next;
	      *next = '\0';
	      undef_macro (p);
	      *next = c;
	      for (p = next; *p && isblank (*p); p++);
	      if (*p)
		{
		  keen_warning ("unexpected tokens after #undef directive");
		}
	      goto end_directive;
	    }

	  if (strcmp (p, "error") == 0)
	    {
	      *next = c;
	      for (p = next; *p && isblank (*p); p++);
	      keen_error ("#error %s", p);
	      goto end_directive;
	    }

	  if (strcmp (p, "line") == 0)
	    {
	      *next = c;
	      // FIXME: implement me
	      goto end_directive;
	    }

	  if (strcmp (p, "pragma") == 0)
	    {
	      *next = c;
	      // FIXME: implement me
	      goto end_directive;
	    }

	  // else
	  keen_error ("unknown directive #%s", p);

	end_directive:

	  /* a directive is not printed to output file */
	  logical_line[0] = '\0';
	  goto print_and_go_on;

	}

      /* NOT A PREPROCESSOR DIRECTIVE */

      if (!is_empty (if_stack))
	{
	  // we are in an IF block
	  if (last_if_stack () & DONT_TREAT)
	    {
	      logical_line[0] = '\0';
	      goto print_and_go_on;
	    }
	}

      macro_replacement (logical_line, 0 /* don't force */ );

    print_and_go_on:

      // print logical line to final buffer
      write_string_to_output_buffer (logical_line);
      write_char_to_output_buffer ('\n', linefeed);

    }				// end of while get_new_logical_line()

  // file is not used anymore
  bf->in_use = 0;

  if (close_buffer && !is_empty (if_stack))
    {
      // there must be no pending if-block at highest level
      keen_error ("if-block is not properly closed");
    }

  // close buffer if requested
  if (close_buffer)
    {
      write_char_to_output_buffer ('\0', 2);
    }

}

/* wrapper for preprocessor call. As the preprocessor is recursive
 * (due to "#include" directives), only the first call must close
 * the Translation Unit buffer. This is the meaning of the 1 passed
 * as argument
 */
void
preprocess (char *filename)
{

  if (debug_level () == DEBUG_VERBOSE)
    {
      keen_debug ("include search list is:\n");
      ListElem e = get_first (include_search_list);
      while (e)
	{
	  keen_debug ("%s\n", (char *) object (e));
	  e = get_next (e);
	}
      keen_debug ("end of include search list.\n");
    }

  // initialize IF-blocks stack for the file
  if_stack = newList ();

  do_preprocess (filename, 1, 0 /* nesting level */ );

  clean_buffered_files ();

}
