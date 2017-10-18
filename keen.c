/*
 * Keen compiler 
 *
 * keen.c
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
#include <string.h>
#include <getopt.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>
#include "keen.h"
#include "grammar.h"
#include "config.h"
#include "prep.h"
#include "type.h"
#include "translate.h"
#include "canon.h"
#include "error.h"
#include "regalloc.h"
#include "link.h"
#include "list.h"
#include "xmalloc.h"

extern n_unit *unit;

void yy_switch_to_buffer ();
void *yy_scan_buffer ();

/* input (parsed) file, needed by lexical analyser flex */
extern FILE *yyin;

char *outputFilename = NULL;
char *tmpdir = NULL;

void
usage (void)
{
  printf ("usage: keen [options] files...\n");
  printf ("options:\n");
  printf ("  -w: skip compiler warning messages\n");
  printf ("  -E: preprocess only. Do not compile\n");
  printf ("  -S: produce assembly code and exit. Do not assemble and link\n");
  printf ("  -c: produce assembly and object code. Do no link\n");
  printf ("  -o: defines output filename\n");
  printf ("  -d | --debug: debug mode\n");
  printf ("  --debug-verbose: verbose debug mode\n");
  printf ("  -v | --version: display keen version and exit\n");
  printf ("  -h | --help: display this help and exit\n");
  printf ("  -Dmacro | -Dmacro=value: defines a macro for preprocessing\n");
  printf ("  --bb-optimization=[0-2]: select basic block optimizations (for debugging)\n");
  printf ("            0 all optimizations are done (same as no option at all)\n");
  printf ("            1 remove fallthrough only\n");
  printf ("            2 same as 1, plus remove useless labels\n");
}

/* options list */
enum p_option
{
  o_all,			/* perform all phases, from preprocess to link */
  o_preprocess_only,		/* stop after preprocessing. Do not compile */
  o_assembly_only,		/* stop after assembly code (*.s) */
  o_object_only
}				/* stop after assembler pass (produce object code) */
option_phases = o_all;
int option_debug = NO_DEBUG;	/* debug is OFF */
int option_skip_warning = 0;	/* skip warning is OFF */
int option_bb_optimization_off = 0;	/* all basic blocks optimizations are ON */

struct option long_options[] = {
  {"help", 0, 0, 'h'},
  {"debug", 0, 0, 'd'},
  {"debug-verbose", 0, 0, 'z'},
  {"version", 0, 0, 'v'},
  {"bb-optimization", 1, 0, 'b'},
  {0, 0, 0, 0}
};

/*
 * parse options and return remaining list of source files
 */

enum {
  SUFFIX_C,
  SUFFIX_H,
  SUFFIX_O,
  SUFFIX_S,
  SUFFIX_I,
  SUFFIX_HI,
  SUFFIX_OTHER
} file_suffix;

char *file_suffix_string[] = {
 "c", "h", "o", "s", "i", "hi", NULL 
};

#define IGNORE		0x1000		/* do nothing */
#define PREPROCESS	0x0001		/* produce preprocessed file */
#define ASSEMBLER	0x0002		/* produce assembler code */
#define PASS_TO_ASM	0x0004		/* pass file to assembler ( = it is assembler code ) */
#define PASS_TO_LINK	0x0008		/* pass file to linker ( = it is object code ) */

#define IS_LAST_ACTION(sf,act)	((sf->action)<(act<<1))

int action_table[][3] = {

  /* actions to perform on each file depending on file-suffix and required compilation option
   */
  { SUFFIX_C,      o_preprocess_only, PREPROCESS },
  { SUFFIX_H,      o_preprocess_only, PREPROCESS },
  { SUFFIX_O,      o_preprocess_only, IGNORE,    },
  { SUFFIX_S,      o_preprocess_only, IGNORE,    },
  { SUFFIX_I,      o_preprocess_only, IGNORE,    },
  { SUFFIX_HI,     o_preprocess_only, IGNORE,    },
  { SUFFIX_OTHER,  o_preprocess_only, IGNORE,    },

  { SUFFIX_C,      o_assembly_only,   PREPROCESS|ASSEMBLER },
  { SUFFIX_H,      o_assembly_only,   IGNORE,              },
  { SUFFIX_O,      o_assembly_only,   IGNORE,              },
  { SUFFIX_S,      o_assembly_only,   IGNORE,              },
  { SUFFIX_I,      o_assembly_only,   ASSEMBLER,           },
  { SUFFIX_HI,     o_assembly_only,   IGNORE,              },
  { SUFFIX_OTHER,  o_assembly_only,   IGNORE,              },

  { SUFFIX_C,      o_object_only,     PREPROCESS|ASSEMBLER|PASS_TO_ASM },
  { SUFFIX_H,      o_object_only,     IGNORE,                          },
  { SUFFIX_O,      o_object_only,     IGNORE,                          },
  { SUFFIX_S,      o_object_only,     PASS_TO_ASM,                     },
  { SUFFIX_I,      o_object_only,     ASSEMBLER|PASS_TO_ASM,           },
  { SUFFIX_HI,     o_object_only,     IGNORE,                          },
  { SUFFIX_OTHER,  o_object_only,     IGNORE,                          },

  { SUFFIX_C,      o_all,             PREPROCESS|ASSEMBLER|PASS_TO_ASM|PASS_TO_LINK },
  { SUFFIX_H,      o_all,             IGNORE,                                       },
  { SUFFIX_O,      o_all,             PASS_TO_LINK,                                 },
  { SUFFIX_S,      o_all,             PASS_TO_ASM|PASS_TO_LINK                      },
  { SUFFIX_I,      o_all,             ASSEMBLER|PASS_TO_ASM|PASS_TO_LINK,           },
  { SUFFIX_HI,     o_all,             IGNORE,                                       },
  { SUFFIX_OTHER,  o_all,             PASS_TO_LINK,                                 },

  { -1,            -1,                -1                                            }

};

typedef struct xsourcefile {
  char *original_name;			/* original name on command line */
  char *basename;			/* basename (with path, without suffix) */
  int  suffix;				/* original suffix */
  int  action;			 	/* required action(s) on this file */
  enum { l_memory, l_file } location;	/* indicate location of last production */
  char *file_location;			/* if last production is file */
  int  cleanup_required; 		/* cleanup last file when used */
} sourcefile;

int
get_action_n_suffix( sourcefile *sf, int option )
{
 int i = 0;
 while ( action_table[i][0] != -1 )
   {
     if ( action_table[i][0] == sf->suffix && action_table[i][1] == option) 
       {
        sf->action = action_table[i][2];
        return 0;
       }
     i++;
   }
 return -1;
}

sourcefile *
new_sourcefile( char *a, char *b, char *c ) 
{
  sourcefile *new = (sourcefile *) xmalloc(sizeof(sourcefile));
  new->original_name = string(a);
  new->basename = string(b);
  new->location = l_file;
  new->file_location = string(a);
  new->cleanup_required = 0;
  int i = 0;
  while (file_suffix_string[i]) 
    {
      if ( strcmp( c, file_suffix_string[i] ) == 0 ) 
        {
          new->suffix = i;
          break;
        }
      i++;
    }
  if ( file_suffix_string[i] == NULL ) new->suffix = SUFFIX_OTHER; 
  return new;
}

typedef List sourcefileList;

sourcefileList
parse_commandline (int argc, char **argv)
{

  // option defaults
  outputFilename = NULL;
  char *s, *p;

  while (1)
    {

      int option_index = 0;
      int c =
	getopt_long (argc, argv, "dD:vhwESco:", long_options, &option_index);
      if (c == -1)
	break;			// no more option

      switch (c)
	{
	case 'v':
	  display_version ();
	  exit (EXIT_SUCCESS);
	case 'h':
	  usage ();
	  exit (EXIT_SUCCESS);
	case 'd':
	  option_debug = DEBUG_NORMAL;
	  break;
	case 'z':
	  option_debug = DEBUG_VERBOSE;
	  break;
	case 'D':
	  s = string (optarg);
	  p = strchr (s, '=');
	  if (p)
	    {
	      *p = 0;
	      predefine_macro (s, p + 1);
	    }
	  else
	    {
	      predefine_macro (s, NULL);
	    }
	  free (s);
	  break;
	case 'b':
	  if (strlen (optarg) != 1 || (optarg[0] != '0' &&
				       optarg[0] != '1' && optarg[0] != '2'))
	    {
	      usage ();
	      exit (EXIT_FAILURE);
	    }
	  option_bb_optimization_off = optarg[0] - '0';
	  break;
	case 'o':
	  outputFilename = string (optarg);
	  break;
	case 'w':
	  option_skip_warning = 1;
	  break;
	case 'E':
	  option_phases = o_preprocess_only;
	  break;
	case 'S':
	  option_phases = o_assembly_only;
	  break;
	case 'c':
	  option_phases = o_object_only;
	  break;
	case '?':
	default:
	  usage ();
	  exit (EXIT_FAILURE);
	}

    }

  // determine temp dir
  if ((tmpdir = getenv ("TMPDIR")) == NULL)
    {
      tmpdir = stringf ("%s", TMPDIR);
    }

  // if no filename provided, give up
  if (optind == argc)
    {
      usage ();
      exit (EXIT_FAILURE);
    }

  // return list of source files, with full name, basename and suffix 
  sourcefileList source_files = newList ();

  while (optind < argc)
    {
      char *dot;
      sourcefile *sf;

      dot = strrchr (argv[optind], '.');
      if (dot == NULL) 
        {
          sf = new_sourcefile( argv[optind], argv[optind], "" );          
        }
      else
        {
	  char *original = string( argv[optind] );
          *dot = 0; 
          sf = new_sourcefile( original , argv[optind] , dot+1 );          
          free( original );
        }

      add (source_files, sf );
      optind++;
    }

  return source_files;
}

int
main (int argc, char **argv)
{
  char **_arg;
  char *objects = (char *) xmalloc (BASE_LENGTH);
  char *op = objects;
  char *prep_buffer;
  int  return_code = 0;

  stringList objectlist = newList ();
  sourcefileList source_files = parse_commandline (argc, argv);

  SET_LOC( location_none );

  /* determine required actions on each source file */
  /* count valid files */
  int valid = 0;
  ListElem e = get_first (source_files);
  while (e)
    {
      sourcefile *sf = object (e);
      get_action_n_suffix ( sf, option_phases );
      if (!(sf->action & IGNORE)) valid++;
      assert (sf->action >= 0);
      e = get_next (e);
    }

  if (valid == 0) 
    {
      keen_error("nothing to do");
      exit(EXIT_FAILURE);
    }

  if (valid > 1 && outputFilename != NULL && option_phases != o_all )
    {
      /* no executable is required, -o option was given, but there are several source files... */
      keen_error("invalid option -o because several source files to treat");
      exit (EXIT_FAILURE);
    }

  init_preprocess ();

  /* treat one source at a time */
  e = get_first (source_files);
  while (e)
    {

      /* get source file */
      sourcefile *sf = object (e);

      SET_LOC (location_file);
      SET_FILE (string (sf->original_name));

      if (sf->action & IGNORE) 
        {
          keen_warning("file ignored, no action is needed");
          e = get_next (e);
          continue;
        }

      /* set debug output file */
      if (debug_level ())
	{
	  set_debug_file (stringf ("%s.debug", sf->basename));
	}

      /* perform required actions ... */

      /* PREPROCESS */
      if (sf->action & PREPROCESS) 
        {

          init_preprocess_buffer ();
          preprocess (sf->original_name);

          if (IS_LAST_ACTION (sf,PREPROCESS))
            {
              if (outputFilename == NULL) 
                {
                  /* print to stdout */
	          printf ("%s\n", get_preprocess_buffer ());
                }
              else 
                {
                  /* print to file */
	          int prep_fd = open (outputFilename, O_RDWR | O_CREAT | O_TRUNC);
                  if (prep_fd < 0) keen_error("unable to open file %s for writing", outputFilename);
	          chmod (outputFilename, S_IRWXU);
                  write (prep_fd, get_preprocess_buffer(), strlen( get_preprocess_buffer())); 
                  close (prep_fd);
                }

            }

          /* in any case, last location is memory (preprocess buffer) */
          sf->location = l_memory;
          sf->cleanup_required = 0; 

        }

      /* PRODUCE ASSEMBLER */
      if (sf->action & ASSEMBLER) 
        {
          
          struct yy_buffer_state *bufferState = yy_scan_buffer (get_preprocess_buffer (), 
                                                strlen (get_preprocess_buffer ()) + 2) ;

          /* prepare parsing */
          if (sf->location == l_memory)
            {
              yy_switch_to_buffer ( bufferState );
            }
          else 
            {
		/* FIXME */
                abort();
            }

          /* produce parse tree */
          init_env ();		// initialize symbol tables environment (needed during lexical analysis)
          init_temporaries ();	// initialize "precolored" temporaries (associated to registers)
          parse ();		// result is produced into unit extern variable 

          yy_delete_buffer( bufferState );  // delete buffer for next iteration, 
                                            // otherwise some parsing side effect may occur

          /* apply some transformations to asbtract-tree */
          init_env ();		// re-init symbol tables environment (whole tree will be parsed again)

          /* translate and canonalize abstract-tree to IR (irnodes) representation */
          trace global_trace = get_canonical_trace (translate (unit));

          /* perform register allocation */
          trace final_trace = register_allocation (global_trace);

          /* produce assembly code to file */
          char *asmFilename;
          int asm_fd;
          if (IS_LAST_ACTION(sf,ASSEMBLER))
	    {
	      if (outputFilename) 
                {
                  asmFilename = outputFilename;
                } 
              else
                {
                  asmFilename = stringf ("%s.s", sf->basename);
                }
	      asm_fd = open (asmFilename, O_RDWR | O_CREAT | O_TRUNC);
	      chmod (asmFilename, S_IRWXU);
              sf->cleanup_required = 0; 
	    }
          else
	    {
              // forge a temporary name for assembler code
	      asmFilename = stringf ("%s/keensXXXXXX", TMPDIR);
	      asm_fd = mkstemp (asmFilename);
              sf->cleanup_required = 1; 
	    }

          sf->location = l_file;
          sf->file_location = asmFilename;

          SET_LOC (location_file);
          if (asm_fd == -1) keen_error ("unable to open file %s for writing", asmFilename);

          output_trace (final_trace, asm_fd);

          close (asm_fd);

        }

      /* PRODUCE OR GET OBJECT CODE */
      if (sf->action & PASS_TO_ASM)
        {
          
           char *objectFilename = stringf ("%s.o", sf->basename);
           assemble_file (sf->file_location, objectFilename);
         
           if (sf->cleanup_required)
             {
               unlink (sf->file_location);	// assembly file is kept only when option_assembly_only is set 
             }
 
           sf->location = l_file;
           sf->file_location = objectFilename;
           sf->cleanup_required = 0;

        }

      if (sf->action & PASS_TO_LINK)
        {
         
           add (objectlist, sf->file_location);	// store object filename for final linking

        } 

      if (debug_level ())
	{
	  close_debug_file ();
	}

      e = get_next (e);
    }

  /* final link phase */
  if (option_phases == o_all)
    {

      if (outputFilename == NULL)
        {
          outputFilename = string( "a.out" );
        }

      if (debug_level ())
	{
	  set_debug_file (stringf ("%s.debug", outputFilename));
	}

      return_code = link_file (objectlist, outputFilename);

      if (debug_level ())
	{
	  close_debug_file ();
	}
    }

  exit (return_code);

}
