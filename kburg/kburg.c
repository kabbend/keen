/*
 * Keen compiler 
 *
 * kburg/kburg.c
 *
 * Copyright (C) 2005 Karim Ben Djedidia <kabend@free.fr> 
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
#include <assert.h>
#include "kburg.h"
#include "list.h"
#include "xmalloc.h"

FILE *file_in;
FILE *file_out;
char *base;

List RULES = NULL;

/*
 * type var 
 */
typedef struct xvar
{
  char *name;
  char *pos; /* holds position within clause if clause-variable
                or position within list [src|dest|jump].%d if affectation variable
              */
} var;

var *
new_var (char *s, char *p)
{
  var *new = (var *) xmalloc (sizeof (var));
  new->name = string (s);
  new->pos = string (p);
  return new;
}

void
free_var (var * v)
{
  if (!v)
    return;
  free (v->name);
  free (v->pos);
}

rule *
new_rule (clause * c, bodyList b)
{
  rule *new = (rule *) xmalloc (sizeof (rule));
  new->id = size (RULES);
  new->c = c;
  new->bodies = b;
  add (RULES, new);
  return new;
}

body *
new_body_fc (char *nodetype, tList dest, tList src, tList l, char *t)
{
  // check that move insn is really a move insn
  if (strcmp(nodetype,"INSN_MOVE")==0)
    {
      if (size(dest) != 1 || size(src) != 1 || !is_empty(l)) keen_error("kburg: invalid MOVE instruction");
    }
  body *new = (body *) xmalloc (sizeof (body));
  new->kind = body_functioncall;
  new->insn_type = nodetype;
  new->dest_list = dest;
  new->src_list = src;
  new->label_list = l;
  new->template = string (t);
  return new;
}

body *
new_body_internal ( tList dest, tList src, tList l )
{
  body *new = (body *) xmalloc (sizeof (body));
  new->kind = body_internal;
  new->insn_type = "INSN_OPER";
  new->dest_list = dest;
  new->src_list = src;
  new->label_list = l;
  new->template = "";
  return new;
}

body *
new_body_multi (char *t)
{
  body *new = (body *) xmalloc (sizeof (body));
  new->kind = body_multi;
  new->template = string (t);
  return new;
}

body *
new_body_string (char *nodetype, tList dest, tList src, tList l, char *t)
{
  // check that move insn is really a move insn
  if (strcmp(nodetype,"INSN_MOVE")==0)
    {
      if (size(dest) != 1 || size(src) != 1 || !is_empty(l)) keen_error("kburg: invalid MOVE instruction");
    }
  body *new = (body *) xmalloc (sizeof (body));
  new->kind = body_template;
  new->insn_type = nodetype;
  new->dest_list = dest;
  new->src_list = src;
  new->label_list = l;
  new->template = string (t);
  return new;
}

clause *
new_clause (knode *node, int has_value, int value, clauseList children)
{
  clause *new = (clause *) xmalloc (sizeof (clause));
  new->node = node;
  new->child = children;
  new->has_value = has_value;
  new->value = value;
  return new;
}

knode *
new_node (char *node, char *mode)
{
  knode *new = (knode *) xmalloc( sizeof(knode));
  new->node = string(node);
  if (mode)
    {
      if (mode[0] == 'm') new->mode = string(mode);
      else new->mode = stringf("%s_mode", mode);
  }
  else new->mode = 0;
  return new;
}

arg *
new_arg_reg (char *var, char *v)
{
  arg *new = (arg *) xmalloc(sizeof(arg));
  new->kind = arg_register;
  new->value = string(v);
  new->var = string(var);
  return new;
}

arg *
new_arg_fc (char *var, char *v)
{
  arg *new = (arg *) xmalloc(sizeof(arg));
  new->kind = arg_functioncall;
  new->value = string(v);
  new->var = string(var);
  return new;
}

arg *
new_arg_var (char *var, char *v)
{
  arg *new = (arg *) xmalloc(sizeof(arg));
  new->kind = arg_variable;
  new->value = string(v);
  new->var = string(var);
  return new;
}

char *
gen_nodetype (char *caller_rank)
{
  char *result = (char *) xmalloc (40 * strlen (caller_rank));	/* conservative approximation of total length */
  int len = strlen (caller_rank);
  if (len == 0)
    return;
  if (len == 1)
    {
      sprintf (result, "GET_IRCHILD( node , %c )", caller_rank[0]);
    }
  else
    {
      char *less = string (caller_rank);
      less[strlen (less) - 1] = 0;
      sprintf (result, "GET_IRCHILD( %s , %c )", gen_nodetype (less),
	       caller_rank[len - 1]);
    }
  return result;
}

void
output_nodetype (char *caller_rank)
{
  int len = strlen (caller_rank);
  if (len == 0)
    return;
  if (len == 1)
    {
      fprintf (file_out, "GET_IRCHILD( node , %c ) ", caller_rank[0]);
    }
  else
    {
      fprintf (file_out, "GET_IRCHILD( ");
      char *less = string (caller_rank);
      less[strlen (less) - 1] = 0;
      output_nodetype (less);
      fprintf (file_out, ", %c ) ", caller_rank[len - 1]);
    }
}

/* emit recursively condition code for if statement, and store variables */
void
output_clauseList (List clauselist, char *caller_rank, List varlist)
{

  if (is_empty (clauselist))
    return;

  int i = 0;
  char *rank = (char *) xmalloc (strlen (caller_rank) + 2);
  char *irank = (char *) xmalloc (strlen (caller_rank) + 3);

  strcpy (rank, caller_rank);
  strcpy (irank, caller_rank);
  int len = strlen (rank);

  clause *c;
  ListElem e = get_first (clauselist);
  while (e)
    {
      c = (clause *) object (e);
      knode *node = c->node;

      rank[len] = '0' + i;
      rank[len + 1] = 0;
      irank[len] = '0' + i;
      irank[len + 1] = '0';
      irank[len + 2] = 0;

      /* wildcard */
      if (node->node[0] == '*') goto child;

      /* identifier starting with underscore is a variable */
      if (node->node[0] == '$')
	{
	  add (varlist, new_var (c->node->node, rank));
	  goto child;
	}

      if (strcmp (node->node, "null") != 0)
	{

	  /* default case */
	  fprintf (file_out, " && ( NODETYPE( ");
	  output_nodetype (rank);
	  fprintf (file_out, " ) == n_%s ) \n", c->node->node);

	  /* if has value */
	  if (c->has_value)
	    {
	      fprintf (file_out, " && ( *((int *)%s) == %d ) \n",
		       gen_nodetype (irank), c->value);
	    }

        /* if has mode */
	  if (node->mode)
	    {
            fprintf (file_out, " && ( GET_MODE( ");
	      output_nodetype (rank);
	      fprintf (file_out, " ) == %s ) \n", node->mode);
	    }

	}
      else
	{
	  /* compare to NULL */
	  fprintf (file_out, " && ( ");
	  output_nodetype (rank);
	  fprintf (file_out, " == NULL ) \n");
	  goto next;
	}

    child:
      if (c->child)
	{
	  output_clauseList (c->child, rank, varlist);
	}

    next:
      i++;
      e = get_next (e);
    }

}

void
generate_arglist( int body_num, tList a_list, char *list_kind, List var_list, List var_affectation_list, int rid, int test_for_optimize )
{

  if (a_list == NULL || is_empty(a_list)) return;

  ListElem l = get_first (a_list);
  while (l)
    {
      arg *current_arg = (arg *) object (l);
      assert (current_arg);
      int i;

      /* if argument holds an affectation as well ($var = arg), store it for further reference,
         before resolving the argument itself */
      if ( current_arg->var )
          {
            ListElem v = get_first (var_list);
            while (v)
              {
                if (strcmp (((var *) object (v))->name, current_arg->var) == 0) 
                  {
                    keen_error ("kburg: variable '%s' is redefined in rule %d", current_arg->var, rid);
                  }
                v = get_next (v);
              }
            v = get_first (var_affectation_list);
            while (v)
              {
                if (strcmp (((var *) object (v))->name, current_arg->var) == 0) 
                  {
                    keen_error ("kburg: variable '%s' is redefined in rule %d", current_arg->var, rid);
                  }
                v = get_next (v);
              }

            // we store this affectation in kburg internal list, so we can recognize the affectation name
            // later ($var) and find out the concerned list (src|dest|label) and body number
            add ( var_affectation_list, 
                  new_var ( current_arg->var, stringf("%s %d", list_kind, body_num ) ) );

            // the actual index in concerned list is not known, however, in kburg, but only
            // at runtime. So we use a similar affectation list into generated code, in order to store
            // the runtime index. So when an affectation needs to be used later, we will have to take kburg
            // information (match the affectation name and find out the list and body number), then to produce
            // code to find out actual index in previous list.
            fprintf (file_out, "  int index%d = size(%s_list%d) ;\n" , size(var_affectation_list), list_kind, body_num );
            fprintf (file_out, "  add( affectation_list , &index%d );\n",  size(var_affectation_list) ); 
          }

      /* now treat the argument ifself */
      switch( current_arg->kind )
        {
          case arg_variable:

            /* is the argument a clause-variable ? */
	      i = 0;
            ListElem v = get_first (var_list);
            while (v)
              {
                if (strcmp (((var *) object (v))->name, current_arg->value) == 0) break;
                v = get_next (v); i++;
              }
            if (v) 
              {
                fprintf (file_out, "  e = preamble_t[%d];\n", i);
	    	if (test_for_optimize)
		{
                 fprintf (file_out, "  // test if this and previous insn can be optimized\n");
                 fprintf (file_out, "  temp *last_dest_temp_%d = (temp *) object (get_first( (List) GET_IRCHILD(e,1) ));\n",i);
                 fprintf (file_out, "  if (last_dest_temp_%d->is_kburg_product) can_optimize = 1;\n",i,i);
		}
                fprintf (file_out, "  %s_list%d = join( %s_list%d, (List) GET_IRCHILD(e,1) );\n",
			       list_kind, body_num, list_kind, body_num);
                break;
              }

            /* is it an affectation-variable ? */
            i = 0;
	    v = get_first (var_affectation_list);
            while (v)
              {
                if (strcmp (((var *) object (v))->name, current_arg->value) == 0) break;
                v = get_next (v); i++;
              }
            if (v)
              {
                int entry;
                char list_name[10];
                /* retrieve concerned list name and body number from internal kburg list */
                sscanf ( ((var *) object(v))->pos , "%s %d", list_name, &entry);

                /* produce code to retrieve at runtime the index in the previous list */
                fprintf (file_out, "  aff_index = *(int *) object( get_ith(affectation_list,%d) ) ;\n", i );

                /* produce code to replace affectation-reference by actual temp or label */
                fprintf (file_out,
			       "  %s_list%d = join( %s_list%d, add( newList() , object( get_ith(%s_list%d,aff_index) ) ) );\n",
			       list_kind, body_num, list_kind, body_num, list_name, entry);
                break;
              }

            /* otherwise... */
            keen_error ("kburg: unknown variable '%s' in rule %d", current_arg->value, rid);
		      
          case arg_register:
            fprintf (file_out, "  add( %s_list%d, new_named_temp(\"%s\") );\n",
		         list_kind, body_num, 
                     (current_arg->value)+1 /* discard heading % symbol */ );
            break;

          case arg_functioncall:
             fprintf (file_out, "  add( %s_list%d, %s( node ) );\n",
			    list_kind, body_num, current_arg->value);
             break;

          default: assert(0);

        } // end switch arg->kind

	    l = get_next (l);
       }
}

void
output ()
{

  fprintf (file_out, "\n/* generated by kburg */\n");
  fprintf (file_out, "/* do not edit. Edit %s/%s.md instead */\n\n", base,base);
  fprintf (file_out, "#include <string.h>\n");
  fprintf (file_out, "#include <stdio.h>\n");
  fprintf (file_out, "#include <stdlib.h>\n");
  fprintf (file_out, "#include \"ir.h\"\n");
  fprintf (file_out, "#include \"irmacro.h\"\n");
  fprintf (file_out, "#include \"temp.h\"\n");
  fprintf (file_out, "#include \"insn.h\"\n");
  fprintf (file_out, "#include \"list.h\"\n");
  fprintf (file_out, "#include \"mode.h\"\n");
  fprintf (file_out, "#include \"gen_%s.h\"\n", base);
  fprintf (file_out, "extern int option_debug;\n\n");
  fprintf (file_out, "int optimized = 0;\n\n");
  
  fprintf (file_out, "irnode *\n");
  fprintf (file_out, "genInsn( insnList insnlist, irnode *node )\n");
  fprintf (file_out, "{\n");
  fprintf (file_out, "  int i, can_optimize;\n");
  fprintf (file_out, "  label *l;\n");
  fprintf (file_out, "  temp *t;\n");
  fprintf (file_out, "  irnode *new_insn           = NULL;\n");
  fprintf (file_out, "  irnode *e                  = NULL;\n");
  fprintf (file_out, "  irnode **preamble_t        = NULL;\n");
  fprintf (file_out, "  char parsed[1024], value[1024], *c;\n\n");

  /* emit one if per rule */
  rule *r;
  ListElem e = get_first (RULES);
  while (e)
    {
      r = object (e);

      /* clause generation for the rule */

      fprintf (file_out, " /* rule %d */\n\n", r->id);

      // check node kind
      fprintf (file_out, " if (( NODETYPE(node) == n_%s ) \n", r->c->node->node);

      // check node mode if applicable
      if (r->c->node->mode)
        {
          fprintf (file_out, " && (GET_MODE(node) == %s ) \n", r->c->node->mode);
        }

      // emit "if" code recursively for the rest of the clause
      // in the same pass, store variable ( "$identifier" ) names and position if any
      List var_list = newList ();
      if (r->c->child)
	{
	  output_clauseList (r->c->child, "", var_list);
	}
      
      fprintf (file_out, " ) { \n");
      fprintf (file_out, "  int aff_index;\n");
      fprintf (file_out, "  List affectation_list = newList();\n");
      fprintf (file_out, "  irnodeList multi = NULL; ListElem multip;\n");
      
      /* evaluate previous variables, if any */
      char c;
      char *s;
      if (size (var_list) > 0)
	{
	  fprintf (file_out,
		   "  preamble_t = (irnode **) xcalloc( %d, sizeof( irnode * ) );\n",
		   size (var_list));
	  ListElem evl = get_first (var_list);
	  int i = 0;
	  while (evl)
	    {
	      var *v = object (evl);
	      fprintf (file_out,
		       "  preamble_t[%d] = genInsn( insnlist, %s ); /* %s */\n",
		       i, gen_nodetype (v->pos), v->name);
	      i++;
	      evl = get_next (evl);
	    }
	}

      /* end of clause-generation */
      /* for each rule-body, generate code */

      /* id of the body inside the current rule */
      int body_num = 0; 

      /* list of affectation within arguments dest/src/jump lists ("$identifier = ...") */
      List var_affectation_list = newList();

      int entry, rank;
      
      ListElem ebl = NULL;
      if (r->bodies) ebl = get_first (r->bodies);

      while (ebl)
	{

	  body *b = (body *) object (ebl);

        fprintf (file_out, "\n  /* body %d: '%s' */\n\n", body_num, b->template);

        if (b->kind == body_multi)
          {
            // special case where irnode instructions are generated on-the-fly by a C custom function
            fprintf (file_out, "  /* call body-multi custom function '%s'*/\n", b->template);
	      fprintf (file_out, "  multi = %s (node, %d);\n", b->template, r->id);
	      fprintf (file_out, "  multip = get_first (multi);\n");
	      fprintf (file_out, "  while(multip) {\n");
	      fprintf (file_out, "    new_insn = object(multip);\n");
	      fprintf (file_out, "    store_insn( insnlist,  new_insn );\n");
	      fprintf (file_out, "    multip = get_next (multip);\n");
	      fprintf (file_out, "  }\n");
            body_num++;
            ebl = get_next (ebl);
            continue;
          }

	  fprintf (file_out, "  tempList  dest_list%d  = newList();\n", body_num);
	  fprintf (file_out, "  tempList  src_list%d   = newList();\n", body_num);
	  fprintf (file_out, "  labelList label_list%d = newList();\n", body_num);
	  fprintf (file_out, "  can_optimize = 0;\n");

        generate_arglist( body_num, b->dest_list, "dest", var_list, var_affectation_list, r->id , 0 ); 

	int test_for_optimize = ( strcmp(b->insn_type, "INSN_MOVE") == 0 );  
        generate_arglist( body_num, b->src_list, "src", var_list, var_affectation_list, r->id , test_for_optimize ); 

        generate_arglist( body_num, b->label_list, "label", var_list, var_affectation_list, r->id , 0 ); 

        if (test_for_optimize)
		{
		  fprintf( file_out, "  if (!can_optimize) { \n", body_num );
		}

        if (b->kind == body_functioncall)
          {
            // assembly template for the instruction is generated by a C function

	      fprintf (file_out,
		       "  new_insn = n%s( %s(node) , dest_list%d , src_list%d , label_list%d , node , %d );\n",
		       b->insn_type, b->template, body_num, body_num, body_num, r->id);
          }
         else
          {
            // assembly template for the instruction is provided as a string to parse
	      fprintf (file_out, "  c = parsed;\n");
	      char *p = b->template;
	      char eval_rank[10];
	      char *eval_rank_p;
	      int not_null = 0;
	      char str[1024];
	      int string_index = 0;
	      int string_length = 0;
	      str[0] = 0;
	      while (*p)
		{
		  if (*p == '%')
		    {
		      p++;
		      switch (*p)
			{
			case 'I':
			  not_null = 1;
			  /* fall through */
			case 'i':
			  if (string_index)
			    {
			      fprintf (file_out, "  sprintf(c,\"%s\");\n",
				       str);
			      fprintf (file_out, "  c+=%d;\n", string_length);
			      str[0] = 0;
			      string_index = 0;
			      string_length = 0;
			    }
			  p++;
			  eval_rank_p = eval_rank;
			  while (*p >= '0' && *p <= '9')
			    *eval_rank_p++ = *p++;
			  *eval_rank_p = 0;
			  fprintf (file_out, "  i = *((int *)%s);\n",
				   gen_nodetype (eval_rank));
			  if (not_null)
			    {
			      fprintf (file_out, "  if ( i != 0 ) {\n");
			    }
			  fprintf (file_out, "  sprintf(value,\"%%d\",i);\n");
			  fprintf (file_out, "  sprintf(c,\"%%s\",value);\n");
			  fprintf (file_out, "  c+=strlen(value);\n");
			  if (not_null)
			    {
			      fprintf (file_out, "  }\n");
			    }
			  break;
			case 'n':
			  if (string_index)
			    {
			      fprintf (file_out, "  sprintf(c,\"%s\");\n",
				       str);
			      fprintf (file_out, "  c+=%d;\n", string_length);
			      str[0] = 0;
			      string_index = 0;
			      string_length = 0;
			    }
			  p++;
			  eval_rank_p = eval_rank;
			  while (*p >= '0' && *p <= '9')
			    *eval_rank_p++ = *p++;
			  *eval_rank_p = 0;
			  fprintf (file_out, "  l = (label *) %s;\n",
				   gen_nodetype (eval_rank));
			  fprintf (file_out,
				   "  sprintf(value,\"%%s\",get_label_name(l));\n");
			  fprintf (file_out, "  sprintf(c,\"%%s\",value);\n");
			  fprintf (file_out, "  c+=strlen(value);\n");
			  break;
			case 't':
			  if (string_index)
			    {
			      fprintf (file_out, "  sprintf(c,\"%s\");\n",
				       str);
			      fprintf (file_out, "  c+=%d;\n", string_length);
			      str[0] = 0;
			      string_index = 0;
			      string_length = 0;
			    }
			  p++;
			  eval_rank_p = eval_rank;
			  while (*p >= '0' && *p <= '9')
			    *eval_rank_p++ = *p++;
			  *eval_rank_p = 0;
			  fprintf (file_out, "  t = (temp *) %s;\n",
				   gen_nodetype (eval_rank));
			  fprintf (file_out,
				   "  sprintf(value,\"%%s\",get_temp_name(t));\n");
			  fprintf (file_out, "  sprintf(c,\"%%s\",value);\n");
			  fprintf (file_out, "  c+=strlen(value);\n");
			  break;
			case 's':
			case 'd':
			case 'l':
			  /* do nothing particular */
			  str[string_index++] = '%';
			  str[string_index++] = '%';
			  str[string_index] = 0;	// end of string
			  /* real string length is lowered by 1, because double-% will be replaced by only one finally */
			  string_length += 1;
			  //fprintf( file_out, "  *c++ = '%';\n");
			  break;
			default:
			  keen_error
			    ("kburg: invalid element while evaluating '%s'",
			     b->template);
			}
		    }
		  else
		    {
		      str[string_index++] = *p;
		      str[string_index] = 0;	// end of string
		      string_length += 1;
		      p++;
		    }
		}
	      if (string_index)
		{
		  fprintf (file_out, "  sprintf(c,\"%s\");\n", str);
		  fprintf (file_out, "  c+=%d;\n", string_length);
		  str[0] = 0;
		  string_index = 0;
		  string_length = 0;
		}
	      fprintf (file_out, "  *c = 0;\n");
	      fprintf (file_out,
		       "  new_insn = n%s( string(parsed) , dest_list%d , src_list%d , label_list%d , node , %d );\n",
		       b->insn_type, body_num, body_num, body_num, r->id);
	    }
	 
          /* finally store instruction into the instruction flow, 
             except if internal
           */ 
          if (b->kind != body_internal) 
            {
              fprintf (file_out, "  store_insn( insnlist, new_insn );\n");
            }

          if (test_for_optimize)
		{
		  fprintf( file_out, "  } else { \n" );
		  fprintf( file_out, "    // Optimize this insn. We do not generate current insn, \n");
		  fprintf( file_out, "    // and we modify dest source of previous one\n" );
		  fprintf( file_out, "    List previous_dest_list = (List) GET_IRCHILD ( e , 1 );\n");
		  fprintf( file_out, "    temp *to = (temp *) object( get_first( dest_list%d ) );\n", body_num);
		  fprintf( file_out, "    change (get_first(previous_dest_list) , to ) ;\n");
		  fprintf( file_out, "    //SET_IRCHILD ( e, 1, (void *)dest_list%d );\n", body_num);
		  fprintf( file_out, "    optimized++;\n", body_num);
		  fprintf( file_out, "  }\n" );
		}

	  body_num++;
	  ebl = get_next (ebl);

	}  /* end of loop on rule bodies */

      /* produce cleanup code */
      fprintf (file_out, "  freel(affectation_list);\n");
      fprintf (file_out, "  if (multi) freel(multi);\n");
      
      /* close rule */
      fprintf (file_out, "  return new_insn;\n");
      fprintf (file_out, " }\n\n");

      e = get_next (e);

    }  /* end of loop on rules */

  fprintf (file_out, " /* else ... */\n");
  fprintf (file_out, " if (option_debug) {\n");
  fprintf (file_out, "  char s[128];\n");
  fprintf (file_out,
	   "  keen_error(\"kburg: in genInsn(), error on %%s\",get_node_name_and_mode(node,s));\n");
  fprintf (file_out, " }\n");
  fprintf (file_out, " abort();\n");
  fprintf (file_out, "} /* end of genInsn() */\n");

}

char *
basename (char *s)
{
  char *p = s;
  while (*p && *p != '.')
    p++;
  *p = 0;
  return s;
}

int
main (int argc, char **argv)
{

  file_in = NULL;
  file_out = stdout;

  argc--;
  argv++;
  while (argc)
    {
      if (strcmp (argv[0], "-o") == 0)
	{
	  file_out = fopen (argv[1], "w");
	  if (file_out == NULL)
	    keen_error ("kburg: cannot open output file '%s'", argv[1]);
	  argc--;
	  argv++;
	  continue;
	}
      file_in = fopen (argv[0], "r");
      if (file_in == NULL)
	keen_error ("kburg: cannot open input file '%s'", argv[0]);
      base = basename (argv[0]);
      argc--;
      argv++;
    }
  if (file_in == NULL)
    keen_error ("usage: kburg [-o outfile ] infile");

  RULES = newList ();

  printf ("kburg: generating rules for %s\n", base);

  yyparse ();

  printf ("kburg: succesfully parsed %d rules\n", size (RULES));

  output ();

  fclose (file_out);

}
