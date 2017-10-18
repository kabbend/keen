/*
 * Keen compiler 
 *
 * translate.c
 *
 * translate an abstract-syntax tree (n_unit), issued by grammar,
 * into an IR tree. The result is not exactly one IR tree, but a list
 * of fragments, each one containing one IR tree and some additional 
 * information. The glue between fragments will be added later on 
 * in further pass.
 *
 * Translate module is also responsible, along with the translation 
 * into IR trees, for type-checking and semantic analysis.
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
#include <assert.h>
#include "xmalloc.h"
#include "translate.h"
#include "bind.h"
#include "symbol.h"
#include "error.h"
#include "temp.h"
#include "frame.h"
#include "mode.h"
#include "reg.h"
#include "list.h"
#include "grammar.h"		/* for position structure */
#include "type.h"
#include "string.h"

// extern or global variables
extern hashtable *env;                /* symbol table */
extern hashtable *env_goto_labels;    /* symbol table for goto labels */
List goto_labels;                     /* string list of goto labels within current function */
extern char *keen_version;

// forward declarations
enum int_or_ext_enum { ext_decl, int_decl } ;

irnode *translate_logical_or (n_logical_or * lor);
irnode *translate_compound_statement ();
irnode *translate_expression ();
irnode *translate_assignment ();
irnode *translate_unary ();
irnode *translate_cast ();
void print_tree( irnode * , int );
type * get_base_type (int ext_or_int, n_declarator_specifiers * bds);
irnode * translate_direct_declarator( int ext_or_int, n_direct_declarator *dd, type *basetype, 
                             int storage, n_initializer *in, int function_main_block,
                             int in_struct_or_union, type **member_type, char **member_name );

enum sptype { PREFIX_SIDE_EFFECT, POSTFIX_SIDE_EFFECT };
n_assignment * create_assign ( void *o, position pos, enum sptype t);

/*
 * global fragmentList variable, that will hold translation result
 */
fragmentList FRAGMENTS;

irnode *
getStmt (irnode * e)
{
  label *l;
  switch (GET_CLASS (e))
    {
    case n_CLASS_STMT:
      return e;
    case n_CLASS_EXPR:
      switch (GET_SUBCLASS (e))
	{
	case n_SUBCLASS_COND:
	  l = new_label ();
	  // patch
	  return nSEQ (getStmt (GET_IRCHILD (e, 0)), nLABEL (l));
	default:
	  return nEXPR (e);
	}
    }
}

/*
 * global pointer to the current function, when applicable
 */
symbol *in_function = NULL;

/*
 * allocate new fragment GLOBAL_VAR structure 
 */
fragment *
new_fragment_gvar (accessor * a, irnode * body)
{
  fragment *new = (fragment *) xmalloc (sizeof (fragment));
  new->kind = frag_var;
  new->u.var.access = a;
  new->u.var.body = body;
  return new;
}

/*
 * allocate new fragment DATA structure 
 */
fragment *
new_fragment_data (accessor * a, irnode * body)
{
  fragment *new = (fragment *) xmalloc (sizeof (fragment));
  new->kind = frag_data;
  new->u.data.access = a;
  new->u.data.body = body;
  return new;
}

/*
 * allocate new fragment FUNC structure 
 */
fragment *
new_fragment_func (irnode * body, frame * f)
{
  fragment *new = (fragment *) xmalloc (sizeof (fragment));
  new->kind = frag_func;
  new->u.func.f = f;
  new->u.func.body = body;
  return new;
}

/*
 * allocate new fragment INFO structure 
 */
fragment *
new_fragment_info (irnode * body)
{
  fragment *new = (fragment *) xmalloc (sizeof (fragment));
  new->kind = frag_info;
  new->u.info.body = body;
  return new;
}

// given expression E, return -E
// by performing '0 - E'
irnode *
getNEG (irnode * e)
{
  return nSUB (nCONSTm (m32, 0), e);
}

/*
 * return 1 if function prototypes are the same, 0 otherwise
 * return type and parameters are compared one by one
 * exception: if <exact_match> is _not_ required, and if <g> is declared 
 * with _no_ parameters, <f> is considered as superseeding <g>, and 
 * no error is returned. If <exact_match> is required, no such rule
 * is applied and the comparison is done parameter by parameter.
 */
int
are_functions_prototypes_compatible (bind * f, bind * g, int exact_match)
{

  // compare return types
  if (!are_compatible_type (f->u.f.return_t, g->u.f.return_t)) return 0;

  // compare parameters if needed
  if (exact_match && (f->u.f.arg_list != NULL && size(f->u.f.arg_list) > 0) 
	&& (g->u.f.arg_list == NULL || size(g->u.f.arg_list) == 0)) return 0;

  if ((f->u.f.arg_list == NULL || size(f->u.f.arg_list) == 0) && (g->u.f.arg_list != NULL && size(g->u.f.arg_list) > 0)) return 0;

  // check ellipsis
  if ((f->u.f.ellipsis != g->u.f.ellipsis)) return 0;

  if ((f->u.f.arg_list != NULL && size(f->u.f.arg_list) > 0) && (g->u.f.arg_list != NULL && size(g->u.f.arg_list) > 0))
    {

      parameterList a, b;
      a = f->u.f.arg_list;
      b = g->u.f.arg_list;

      /* compare numbers of parameters, as they must match. Only exception is if 
         one is empty and the other is void */
      if (size (a) == 0 && size(b) == 1)
        {
          parameter *p = (parameter *) object(get_first(b));
          if (!are_compatible_type ( get_parameter_type(p), Void)) return 0;
	  return 1;
        } 
      else if (size (b) == 0 && size(a) == 1)
        {
          parameter *p = (parameter *) object(get_first(a));
          if (!are_compatible_type ( get_parameter_type(p), Void)) return 0;
	  return 1;
        }
      else if (size (a) != size (b)) return 0;

      /* compare parameters one by one */
      ListElem e = get_first (a);
      ListElem f = get_first (b);
      while (e)
	{
	  if (!are_compatible_type
	      (get_parameter_type( ((parameter *) object (e)) ),
	       get_parameter_type( ((parameter *) object (f)) )))
	    return 0;
	  e = get_next (e);
	  f = get_next (f);
	}
    }

  return 1;
}


int 
_parse_type_qualifier_list ( n_type_qualifier_list *tql , int qualifier )
{
  if (tql == NULL) return qualifier;

  n_type_qualifier *tq = tql->tq;
  switch (tq->kind)
    {
	case type_qual_const: 
		if (qualifier == undefined) { qualifier = t_const; break; }
		if (qualifier & t_const) keen_error ("duplicate type qualifier const");
		qualifier = qualifier | t_const;
		break;

	case type_qual_restrict: 	
		/* FIXME not supported today (but not C89 anyway) */ 
		break;

	case type_qual_volatile: 
		if (qualifier == undefined) { qualifier = t_volatile; break; }
		if (qualifier & t_volatile) keen_error ("duplicate type qualifier volatile");
		qualifier = qualifier | t_volatile;
		break;

	default: assert(0);
    }

  return _parse_type_qualifier_list (tql->tql, qualifier);
}

int 
parse_type_qualifier_list ( n_type_qualifier_list *tql )
{
 return _parse_type_qualifier_list ( tql , undefined );
}

/*
 * Complete the base type <basetype> according to declarator <d>, if needed
 * Typically, in the declaration "int *a, b;", the base type is hold by specifiers, here 
 * "int", and the first declarator is "* f". Consequently, the actual type for "f" is 
 * not "int" (the basetype) but another type depending on the declarator itself (here,
 * "int *")
 * in case of abstract declarator, d is NULL and the type is unchanged
 */
type *
complete_base_type (n_declarator * d, type * basetype)
{
  if ( d == NULL) return basetype;
  n_pointer *pt;
  type *t = basetype;
  for (pt = d->p; pt != NULL; pt = pt->p)
    {
      t = type_pointer (t); 
      t->qualifier = parse_type_qualifier_list (pt->tql);
    }
  return t;
}

type *
complete_base_abstract_type (n_abstract_declarator * d, type * basetype)
{
  if ( d == NULL) return basetype;
  n_pointer *pt;
  type *t = basetype;
  for (pt = d->p; pt != NULL; pt = pt->p)
    {
      t = type_pointer (t); 
      t->qualifier = parse_type_qualifier_list (pt->tql);
    }
  return t;
}


/*
 * return the storage class as declared in the declaration <ds> 
 * returns "undefined" value if not declared
 */
int
get_storage_class (n_declarator_specifiers * ds)
{

  if (ds == NULL)
    return undefined;

  SET_POS (ds->pos);

  int storage = undefined;
  if (ds->kind == decl_spec_storage)
    {
      if ((storage != undefined) && (storage != ds->u.storage->kind))
	{
	  keen_error ("multiple storage classes in declaration");
	}
      if (storage == ds->u.storage->kind)
	{
	  keen_warning ("storage class is duplicated in declaration");
	}
      storage = ds->u.storage->kind;
    }
  return storage;

}

memberList
parse_struct_declaration( int int_or_ext, n_struct_declaration *decl )
{
  assert(decl);

  SET_POS(decl->pos);

  if (decl->sdl == NULL)
    {
      // get last type_specifier
      n_declarator_specifiers *ds, *prev_last = NULL, *last = NULL;
      ds = decl->sds; 

      SET_POS(ds->pos);

      if (ds->kind == decl_spec_type ) last = ds;
      while (ds)  
	{ 
 	  if (!ds->decl) last = ds;
	  if (ds->decl) prev_last = ds;
	  ds = ds->decl;
	}
      assert(last->kind == decl_spec_type); // FIXME

      if (!last)
	{
	  // no type specifier and no declarator, this is obviously an error
	  keen_error ("invalid member declaration");
	}

      n_type_specifier *ts = last->u.type_spec;

      // if no declarator but enum or struct/union, this is not an error and
      // we continue parsing
      if (ts->kind == type_spe_enum || ts->kind == type_spe_su)
	goto continue_parsing;

      // if no declarator and last id is a typename, this is not an error in this
      // context
      symbol *s = lookup_symbol (env, ts->name);
      if (s && s->binding->kind == b_type) keen_warning("member '%s' has same name as an existing type", ts->name);

      // otherwise we transform this type_spe into a declarator
      n_id n;
      n.v = string(ts->name);
      n.pos.line = ts->pos.line;
      n.pos.column = ts->pos.column;
      n.pos.filename = ts->pos.filename;

      n_direct_declarator *id = new_direct_declarator_id(n);
      n_declarator *nd = new_declarator ( NULL, id );
      n_struct_declarator *nsd = new_struct_declarator (nd);

      if (prev_last) prev_last->decl = NULL;
      decl->sdl = new_struct_declarator_list ( NULL, nsd );

    }

continue_parsing:

  ;
  type *basetype = get_base_type (int_or_ext,decl->sds);

  memberList ret = newList();
  n_struct_declarator_list *sdl;
  for(sdl=decl->sdl;sdl!=NULL;sdl=sdl->sdl)
    {
      n_declarator *d = sdl->sd->d;
      type *ty = complete_base_type(d,basetype);
      n_direct_declarator *dd = d->decl;
      SET_POS (dd->pos);
      char *member_name;
      type *t;
      translate_direct_declarator ( int_or_ext, dd , ty, storage_auto, NULL, 0, 1, &t, &member_name );
      if (!is_complete(t))
        {
          keen_error ("cannot create a member %s with incomplete type", member_name);
        }
      add( ret , new_member(member_name,t) ); 
    }

  // return list in reverse order, due to grammar parsing method
  memberList reverse = newList();
  member *m;
  FOR_EACH_REWIND(m,ret,e)
  { 
    add( reverse, m); 
    keen_debug("creating member %s\n", (m->name));
    print_type( m->t );
    keen_debug("\n");
  }
  freel(ret);
  return reverse;
}

memberList 
parse_struct_declaration_list( int int_or_ext, n_struct_declaration_list *sdl )
{
  if (!sdl) return newList();
  SET_POS(sdl->pos);
  memberList a = parse_struct_declaration_list(int_or_ext, sdl->sdl);
  memberList b = parse_struct_declaration(int_or_ext, sdl->decl);
  // check that no duplicate within a and b
  member *ma, *mb;
  FOR_EACH(ma,a,e)
    {
      FOR_EACH(mb,b,f)
        {
          if (strcmp(mb->name,ma->name)==0)
            {
              keen_error("member %s is redeclared", mb->name);
            }
        }
    }
  // else
  return join( a , b );
}

/*
 * return the base type from declaration specifiers <bds> (bds stands
 * for base declarator specifiers)
 */
type *
get_base_type (int int_or_ext, n_declarator_specifiers * bds)
{

  symbol *s;
  int two_types_declared = 0;
  int is_unsigned = undefined;
  type *t = NULL;
  int qualifier = undefined;
  bind *b;
  enum { no_store, store_create, store_complete };

  n_declarator_specifiers *ds;
  for (ds = bds; ds != NULL; ds = ds->decl)
    {

      SET_POS (ds->pos);

      // 
      // get type qualifiers
      //
      
      if (ds->kind == decl_spec_qualifier)
	{

	  n_type_qualifier *tq = ds->u.qualifier;
	  switch (tq->kind)
	    {
		case type_qual_const: 
			if (qualifier == undefined) { qualifier = t_const; break; }
			if (qualifier & t_const) keen_error ("duplicate type qualifier const");
			qualifier = qualifier | t_const;
			break;

		case type_qual_restrict: 	
			/* FIXME not supported today (but not C89 anyway) */ 
			break;

		case type_qual_volatile: 
			if (qualifier == undefined) { qualifier = t_volatile; break; }
			if (qualifier & t_volatile) keen_error ("duplicate type qualifier volatile");
			qualifier = qualifier | t_volatile;
			break;

		default: assert(0);
	    }

	}

      // 
      // get type 
      //
      
      if (ds->kind == decl_spec_type)
	{

          n_type_specifier *ts = ds->u.type_spec;

          switch(ts->kind)
            {

              case type_spe: /* usual type specifier */

	      // treat special keywords signed and unsigned
	      if (strcmp ("signed", ts->name) == 0)
	        {
	          if (is_unsigned == 1)
		    {
		      keen_error ("both signed and unsigned in declaration");
		    }
	          if (is_unsigned == 0)
		    {
		      keen_error ("redundant 'signed' specifier in declaration");
		    }
	          is_unsigned = 0;
	          continue;
	        }
	      if (strcmp ("unsigned", ts->name) == 0)
	        {
	          if (is_unsigned == 0)
		    {
		      keen_error ("both signed and unsigned in declaration");
		    }
	          if (is_unsigned == 1)
		    {
		      keen_error ("redundant 'unsigned' specifier in declaration");
		    }
	          is_unsigned = 1;
	          continue;
	        }

	      // lookup for the symbol in symbol table
	      s = lookup_symbol (env, ts->name);
	      if (s == NULL)
	        {
	          keen_error ("unknown type '%s' in declaration", ts->name);
	        }
	      b = s->binding;
	      if (b->kind != b_type)
	        {
	          keen_error ("symbol '%s' in declaration is not a type", ts->name);
	        }

	      if (t == NULL)
	        {
	          // if no type was specified so far, make this type the base type
	          t = b->t;
	        }
	      else
	        {
	          // else, if several types are specified, check for validity (for instance, "long int")
	          if (two_types_declared == 1)
		    {
		      // we support only 2 types for the moment ('long int', but no 'long long int' for instance)
		      keen_error ("too many data types in declaration");
		    }
	          two_types_declared = 1;
	          if (((t == Int) && (b->t == Long)) || ((t == Long) && (b->t == Int)))
		    {
		      t = Long;		// store 'long'
		    }
	          else if (((t == Int) && (b->t == Short)) || ((t == Short) && (b->t == Int)))
		    {
		      t = Short;	// store 'short'
		    }
	          else if (((t == Long) && (b->t == Short)) || ((t == Short) && (b->t == Long)))
		    {
		      keen_error ("both long and short specifiers in declaration");
		    }
	          else
		    {
		      // otherwise, invalid combination
		      keen_error ("syntax error on symbol '%s' in declaration", ts->name);
		    }
	        }

              break;

            case type_spe_enum: /* enumeration */
              
              SET_POS (ts->u.en->pos);
              n_enumeration_specifier *ens = ts->u.en;
             
              int enum_store_required = no_store;
              symbol *enum_s = NULL;
              char *enum_name = stringf("enum %s",ens->id);
 
              if (ens->id)
                {
		  // NAMED ENUMERATION 
                  /* check if this named enumeration already exists */
                  enum_s = lookup_symbol(env, enum_name);
                  if (enum_s && enum_s->scope == env->current_scope) 
                    {
                      // struct or union found in current scope
                      if (ens->el && is_complete(enum_s->binding->t))
                        {
                          keen_error("%s is redefined",enum_name);
                        }
                      if (!ens->el)
                        {
                          // Nothing more to do
                          t = enum_s->binding->t;
                          break; 
                        }
                      // otherwise, enumeration found but is currently incomplete
                      // and there is some definition here. Just continue...
                      t = enum_s->binding->t;
                      enum_store_required = store_complete;
                    }
                  else
                    { 
                      // new type declaration. 
                      enum_store_required = store_create;
                    }
		}
	      else
		{
                  /* anonymous enumeration. This de facto creates a new type */
                  /* but there is no need to store it in symbol table as it cannot */
                  /* be referenced by name anywhere */
                  enum_store_required = no_store;
		}

	      // PARSE THE ENUMERATION
	      n_enumerator_list *enl = ens->el;
	      int enumerator_current_counter = 0; // by default the initial value is 0

              while (enl)
                {
		  assert(enl->en);

		  // check if this enumerator does not already exist in same scope
		  symbol *s = lookup_symbol (env, enl->en->id);
		  if (s && s->scope == env->current_scope)
		    {
		      keen_error ("enumerator %s conflicts with existing symbol", enl->en->id);
		    }

		  // get enumerator value if explicit
		  if (enl->en->lor) 
		    {
		      // treat this enumerator with expression
		      irnode *expr = translate_logical_or (enl->en->lor);
		      if (!is_arithmetic (GET_TYPE(expr)) || !(NODETYPE(expr) == n_CONST))
		  	{
			  keen_error("invalid enumeration value");
			}  
		      enumerator_current_counter = *(int *) GET_IRCHILD(expr, 0);
		      freenode (expr);
		    }

		  // insert this enumerator into symbol table, with right value
		  // an enumerator is declared as a constant of type (const int)
		  bind *b = new_bind_const (enumerator_current_counter);
		  insert_symbol (env, enl->en->id, b); 
		  // next enumerator
		  enumerator_current_counter++;
                  enl = enl->el;
                }

              // create the enumeration type finally
              type *final = type_dup( Int ) ;
	      final->is_enum = 1;
              if (!ens->el) 
                {
                  // this was an incomplete definition
                  final->is_complete = 0;
                }
              else
                {
                  keen_debug("enumeration %s is now complete\n",enum_name);
                  final->is_complete = 1;
                }

              // store symbol if needed
              switch(enum_store_required)
                {
                  case no_store: 
                    if (t == NULL) { t = final; } else { *t = *final; }
                    break;
                  case store_complete:
                    *t = *final;
                    b = new_bind_type (t);
                    assert(enum_name);
                    assert(enum_s);
                    assert(t->is_complete);
                    enum_s->binding = b; 
                    break;
                  case store_create:
                    if (t == NULL) { t = final; } else { *t = *final; }
                    b = new_bind_type (t);
                    assert(enum_name);
                    enum_s = insert_symbol ( env, enum_name, b ); 
	 	    break;
                }

              break;

            case type_spe_su: /* struct or union */
              
              SET_POS (ts->u.su->pos);
              n_struct_or_union_specifier *sus = ts->u.su;
             
              int store_required = no_store;
              char *name = NULL;
              symbol *s = NULL;
 
              if (!sus->is_anonymous)
                {
                  /* check if this named struct/union already exists */
                  name = (sus->kind==su_struct)?stringf("struct %s",sus->id.v):
                                                stringf("union %s",sus->id.v);
                  s = lookup_symbol(env, name);
                  if (s) 
                    {
                      // struct or union found 
                      if (sus->sdl && s->binding->t->is_complete)
                        {
                          keen_error("%s is redefined",name);
                        }
                      if (!sus->sdl)
                        {
                          // Nothing more to do
                          t = s->binding->t;
                          break; 
                        }
                      // otherwise, struct/union found but is currently incomplete
                      // and there is some definition here. Just continue...
                      t = s->binding->t;
                      store_required = store_complete;
                    }
                  else
                    { 
                      // new type declaration. 
                      store_required = store_create;
                    }
                }
              else
                {
                  /* anonymous struct/union. This de facto creates a new type */
                  /* but there is no need to store it in symbol table as it cannot */
                  /* be referenced by name anywhere */
                  store_required = no_store;
                }

              // this will hold actual members
              memberList tlist = newList();

              // we store struct/union into symbol table, even incomplete,
              // to allow recursive declarations with pointers
              if (store_required == store_create)
                {
                  if (sus->kind == su_struct) t = type_struct ( tlist );
                    else t = type_union ( tlist );
                  t->is_complete = 0;
                  b = new_bind_type (t);
                  assert(name);
                  s = insert_symbol ( env, name, b ); 
                  // we will complete it later
                  if (sus->sdl) store_required = store_complete;
                    else store_required = no_store;
                }

              // we now need to parse the struct/union type recursively
              if (sus->sdl)
               {
                 tlist = parse_struct_declaration_list(int_or_ext,sus->sdl);
                 // if struct, offset need to be set properly now
                 // if union, all offsets are null
                 int current_offset = 0;
                 member *m;
                 FOR_EACH(m,tlist,e)
                   {
                     m->offset = current_offset;
                     keen_debug("setting offset %d to member %s\n", current_offset, m->name);
                     if (sus->kind == su_struct)
                       {
                         current_offset += get_aligned_type_size( m->t );
                       }
                   }
               }

              // create the struct/union type finally
              type *replace;
              if (sus->kind == su_struct)
                replace = type_struct ( tlist );
              else
                replace = type_union ( tlist );

              if (!sus->sdl) 
                {
                  // this was an incomplete definition
                  replace->is_complete = 0;
                }
              else
                {
                  keen_debug("struct %s is now complete\n",name);
                  replace->is_complete = 1;
                }

              // store symbol if needed
              switch(store_required)
                {
                  case no_store: 
                    if (t == NULL) { t = replace; } else { *t = *replace; }
                    break;
                  case store_complete:
                    *t = *replace;
                    b = new_bind_type (t);
                    assert(name);
                    assert(s);
                    assert(t->is_complete);
                    s->binding = b; 
                    break;
                  case store_create:
                    assert(0);
                }

              break;

          } // end switch()

	} // end get type

    } // end for-loop on specifiers

  //
  // Step 2: type final check 
  //

  SET_POS (bds->pos);

  // if no type was specified at all, make it 'int' by default
  if (t == NULL)
    {
      t = Int;
    }

  // if type was not an arithmetic one, any specified sign is useless 
  if (!is_arithmetic(t))
    {
      if (is_unsigned != undefined) keen_warning ("useless sign specified in declaration");
      is_unsigned = undefined;
    }

  // reversely, if the type is a basic one, and no sign was specified, make it signed by default
  if (is_arithmetic(t) && is_unsigned == undefined)
    {
      is_unsigned = 0;
    }

  // create actual type by replication
  if (!is_struct_or_union(t) && !is_enum(t)) t = type_dup (t);
  t->is_unsigned = is_unsigned;
  t->qualifier = qualifier;
  return t;

}

/*
 * parse declaration for typedefs and update symbol table appropriately
 */
void
parse_declaration_for_typedefs (n_declaration * p)
{

  // keep current location

  int current_line = GET_LINE();
  int current_column = GET_COLUMN();
  char *current_file = GET_FILE();

  int storage = get_storage_class (p->decl);
  if (storage != storage_typedef)
    goto away;

  type *basetype = NULL;
  n_init_declarator_list *idl = p->init;
  for (; idl != NULL; idl = idl->list)
    {

      symbol *s;
      bind *b;
      n_init_declarator *id = idl->init;
      n_declarator *d = id->decl;
      n_direct_declarator *dd = d->decl;

      SET_POS (dd->pos);

      switch (dd->kind)
	{

	case dd_id:		/* IDENTIFIER DECLARATION */
	  // check that the symbol has not been declared already in same scope
	  s = lookup_symbol (env, dd->u.name);
	  if ((s != NULL) && (s->scope == env->current_scope))
	    {
	      keen_error ("symbol '%s' is already declared", dd->u.name);
	    }
	  // store the symbol
	  b = new_bind_type (NULL);	/* no need to store a type actually */
	  insert_symbol (env, dd->u.name, b);
	  break;

	}			/* end switch(dd->kind) */

    }				/* end for(idl) */

away:
  // restore location
  SET_LINE(current_line);
  SET_COLUMN(current_column);
  SET_FILE(current_file);

}


type *
get_final_type_n_name ( type *basetype, n_direct_declarator *dd, char **name )
{
 type *t; 
 int num_elem;
 assert(dd);
 switch (dd->kind)
   {
     case dd_id: *name = dd->u.name; return basetype; 
     case dd_ptl: assert(0); // FIXME not supported yet
     case dd_array: 
	if (is_array(basetype) && get_array_size(basetype) == undefined )
	  {
	    keen_error ("empty array size is not allowed here");
	  }
	num_elem = undefined;
 	if (dd->u.a.expr)
	  {
	    // size is specified
	    irnode *i = translate_expression (dd->u.a.expr);
	    if (!is_arithmetic(GET_TYPE(i)) || !(NODETYPE(i)==n_CONST))
		{
		  keen_error ("invalid array size");
		}
	    num_elem = *(int *) GET_IRCHILD (i, 0); 
	    if (num_elem <= 0)
		{
		  keen_error ("invalid array size");
		}
	  }
        t = type_array ( basetype, num_elem );
	return get_final_type_n_name( t, dd->u.a.dd, name );
     default: assert(0);
   }
}

type *
get_final_abstract_type ( type *basetype, n_direct_abstract_declarator *dad )
{
 type *t; 
 int num_elem;
 if (!dad) return basetype;
 switch (dad->kind)
   {
     case abstract_ptl: assert(0); // FIXME not supported yet
     case abstract_array: 
	if (is_array(basetype) && get_array_size(basetype) == undefined )
	  {
	    keen_error ("empty array size is not allowed here");
	  }
	num_elem = undefined;
 	if (dad->u.array.expr)
	  {
	    // size is specified
	    irnode *i = translate_expression (dad->u.array.expr);
	    if (!is_arithmetic(GET_TYPE(i)) || !(NODETYPE(i)==n_CONST))
		{
		  keen_error ("invalid array size");
		}
	    num_elem = *(int *) GET_IRCHILD (i, 0); 
	    if (num_elem <= 0)
		{
		  keen_error ("invalid array size");
		}
	  }
	t = type_array ( basetype, num_elem );
	return get_final_abstract_type ( t, dad->u.array.d ); 
     default: assert(0);
   }
}

type *
parse_type_name (n_type_name *tname)
{
 n_type_name_specifiers_list *sl;
 // FIXME ! today tname->l is a list but there is no need for this as a declarator_specifiers is a list also
 // grammar has been adapted to this but not the structures/creator functions. The loop below is useless as
 // there is one element only
 type *basetype = NULL;
 for (sl = tname->l; sl != NULL; sl = sl->l )
   {
      basetype = get_base_type (int_decl /* is internal */, sl->s);
   } 
 type *t = complete_base_abstract_type (tname->absdecl, basetype);
 // treat declarator if any
 if (tname->absdecl) t = get_final_abstract_type (t, tname->absdecl->d);
 return t; 
}

/*
 * parse parameter type list <ptl> and return a typelist chained-list
 * holding one (completed) type per parameter
 * modify <ellipsis> arg to hold 1 if the <ptl> contains an ellipsis, 0 otherwise
 */
parameterList
parse_ptl (n_parameter_type_list * ptl, int *ellipsis, int abstract_allowed)
{
  parameterList rt = newList ();
  if (ptl == NULL) return rt;
  *ellipsis = ptl->ellipsis;
  n_parameter_list *pl;
  int number_of_parameters = 0;

  for (pl = ptl->pl; pl != NULL; pl = pl->pl, number_of_parameters++)
    {
      n_parameter_declaration *pd = pl->pd;
      SET_POS (pd->pos);

      type *basetype = get_base_type (ext_decl /* cannot be internal */, pd->ds);

      if (!abstract_allowed && pd->kind == pdecl_abstract)
	{
	  // FIXME we should test direcly with != Void but some type_dup() before prevents this
	  // obviously too many type_dup() are used in get_base_type()...
	  if (!are_compatible_type(basetype, Void )) 
            {
	      // this is an error except if abstract is (void)
	      keen_error("missing declarator in function parameter");
            }
	  // So type is void. Check that there is only one parameter
	  if (number_of_parameters != 0 || pl->pl)
            {
	      keen_error("syntax error in function parameters");
            }
	  // otherwise, OK, fallthrough and return an parameter list with Void
	  //return newList();
	}

      // for each declarator, complete the base type if needed
      type *t;
      if (pd->kind == pdecl_abstract)
	{
          t = complete_base_abstract_type (pd->absdecl, basetype);
	}
      else
	{
          t = complete_base_type (pd->decl, basetype);
	}


      // check declarator validity if applicable
      parameter *nt = (parameter *) xmalloc (sizeof (parameter));
      if (pd->kind == pdecl)
        {
	  // declarator is present
          n_direct_declarator *dd = pd->decl->decl;
          SET_POS (dd->pos);
          char *name = NULL;
	  t = get_final_type_n_name (t, dd, &name);
	  if (!name)
 	    {
	      keen_error("syntax error in function parameter list");
	    }
          nt->name = string (name);
	}
      else
    	{
	  // abstract declarator
	  n_direct_abstract_declarator *dad;
   	  if (pd->absdecl)
	    dad = pd->absdecl->d;
	  else
	    dad = NULL;
	  t = get_final_abstract_type (t, dad);
          nt->name = NULL;
 	}

      // an array is converted to pointer when passed as argument
      if (is_array(t)) 
	{
	  t->is_array = 0;
	  t->is_complete = 1;
	   t->qualifier = undefined; 
	}

      nt->t = t;
      add (rt, nt);
    }

  return rt;
}

/* forward declaration */
irnode * parse_initializer( int int_or_ext, dataList dl, int just_count, irnode *sequence, n_id id, 
                            type *id_type, n_initializer *in, int indexes[], int current_index, type *current_type, 
			    memberList mlist, int parent_is_union );

irnode *
parse_initializer_list( int int_or_ext, dataList dl, int just_count, irnode *sequence, n_id id, 
                        type *id_type, n_initializer_list *in_list, int indexes[], int current_index, type *current_type, 
			memberList mlist, int parent_is_union )
{
  if (!in_list) return sequence; // end recursion
  irnode *new;

  type *t;
  if (is_array(current_type)) t = current_type->u.pointer_on;
  else if (is_struct_or_union(current_type)) t = ((member *) object( get_ith( mlist,indexes[current_index])))->t;
  else t = current_type;

  new = parse_initializer ( int_or_ext, dl, just_count, sequence, id, id_type, in_list->init, indexes, current_index, t , mlist, parent_is_union );

  if ( in_list->init_list )
    {
      int maxindex = 0; // a priori
      if (is_array(current_type)) 
        {
          maxindex = get_array_size (current_type);
          if (maxindex != undefined) maxindex--; // index starts at 0, not 1
        }
      else if (parent_is_union)
	{
	  maxindex = 0;
	}
      else if (mlist)
	{
	  maxindex = size( mlist ) - 1; // index starts at 0, not 1
	}

      if ( !just_count && maxindex != undefined && indexes[current_index]+1 > maxindex)
        { 
          keen_error ("too many elements in initialization of '%s'", id.v );
        }

      indexes[ current_index ]++;

      if (mlist)
	{
	  member *brother = object ( get_ith( mlist, indexes[ current_index ] ) );
	  current_type = brother->t;
	}

      new = parse_initializer_list (int_or_ext, dl, just_count, new, id, id_type, in_list->init_list, 
                                    indexes, current_index, current_type, mlist, parent_is_union );

      // reinit index of current dimension to previous value, because recursion can go backward
      // (to previous dimension) and in such case the following ones must be reset to 0
      // but do nothing if 1st dimension because
      // - not useful in this case (cannot go backward)
      // - this 1st index value will be used in case of uncomplete array to set the actual array size
      if (current_index != 0) indexes[ current_index ]--;
    }

  return new; 
}

n_postfix *
create_array_or_struct_recursively( n_id id, type *t, int *indexes, int current_index )
{

 assert(indexes);

 // we will create complete assignment sentence. this will help
 n_keyword null_keyword = { {0, 0, NULL} , 0 };

 // get actual type
 type *at = get_type_depth ( t, indexes, current_index - 1, 0 );

 int constant = indexes[current_index];

 if (is_array(at))
 {
   n_integer n_constant;
   n_constant.pos.line = id.pos.line;
   n_constant.pos.column = id.pos.column;
   n_constant.pos.filename = id.pos.filename;
   n_constant.v = constant;

   n_expression *nexp = new_expression( NULL, new_assignment( NULL, null_keyword, 
      new_logical_or (NULL, new_logical_and (NULL, new_inclusive_or 
      (NULL, new_exclusive_or (NULL, new_and (NULL, new_equality 
      (NULL, null_keyword, new_relational (NULL, null_keyword, new_shift 
      (NULL, null_keyword, new_additive (NULL, null_keyword, new_multiplicative
      (NULL, null_keyword, new_cast_unary (new_unary_postfix 
      (new_postfix_primary (new_primary_constant ( n_constant ))))))))))))))));

   if (current_index == 0) 
     return new_postfix_array( new_postfix_primary( new_primary_identifier( id )), nexp );
   else
     return new_postfix_array( create_array_or_struct_recursively( id, t, indexes, current_index-1 ), nexp );

 }
 else if (is_struct_or_union(at))
 {

   member *mb = (member *) object( get_ith( at->u.members, constant ) );
   n_id m;
   m.pos.line = id.pos.line;
   m.pos.column = id.pos.column;
   m.pos.filename = id.pos.filename;
   m.v = string( mb->name );

   if (current_index == 0) 
     return new_postfix_dot( new_postfix_primary( new_primary_identifier( id )), m );
   else
     return new_postfix_dot( create_array_or_struct_recursively( id, t, indexes, current_index-1 ), m );

 }

 // should not get here...
 abort();

}

n_logical_or *
create_initialization_lor( n_id id, type *id_type, int *indexes, int current_index )
{

  // we will create complete assignment sentence. this will help
  n_keyword null_keyword = { {0, 0, NULL} , 0 };
  n_keyword keyword_opASSIGN = { {0, 0, NULL} , opASSIGN };

  // create a complete assignement sentence from the variable declaration & assignment
  n_logical_or *lor ;

  if (indexes == NULL)
    {
      lor = new_logical_or (NULL, new_logical_and (NULL, new_inclusive_or 
                  (NULL, new_exclusive_or (NULL, new_and (NULL, new_equality 
                  (NULL, null_keyword, new_relational (NULL, null_keyword, new_shift 
                  (NULL, null_keyword, new_additive (NULL, null_keyword, new_multiplicative
		  (NULL, null_keyword, new_cast_unary (new_unary_postfix 
                  (new_postfix_primary (new_primary_identifier (id))))))))))))));
    }
  else 
    {
      n_postfix *a = create_array_or_struct_recursively(id,id_type,indexes,current_index);

      lor = new_logical_or (NULL, new_logical_and (NULL, new_inclusive_or 
                  (NULL, new_exclusive_or (NULL, new_and (NULL, new_equality 
                  (NULL, null_keyword, new_relational (NULL, null_keyword, new_shift 
                  (NULL, null_keyword, new_additive (NULL, null_keyword, new_multiplicative
		  (NULL, null_keyword, new_cast_unary (new_unary_postfix 
                  (a))))))))))));
    }

  return lor;
}

/* return the string content if assign represents a literal string constant
 * (as defined in source code), or NULL otherwise
 */
/*
char *
is_string( n_assignment *a )
{
  assert(a->condassign == 0);
  assert(a->expr == 0);
  irnode *i = translate_assignment (a);
  if (IS_LITERAL(i) && GET_MODE(i) == STR_mode) return GET_LITERAL(i);
  return NULL;
}
*/

n_initializer_list *
create_string_equiv( n_initializer *initial, char *remaining_string )
{
  
  n_keyword null_keyword = { {0, 0, NULL} , 0 };
  n_keyword keyword_opASSIGN = { {0, 0, NULL} , opASSIGN };

  // create a constant char based on received string
  n_char cchar;
  cchar.pos.line = initial->pos.line;
  cchar.pos.column = initial->pos.column;
  cchar.pos.filename = initial->pos.filename;
  cchar.v = *remaining_string;

  n_logical_or *lor = new_logical_or (NULL, new_logical_and (NULL, new_inclusive_or 
                  (NULL, new_exclusive_or (NULL, new_and (NULL, new_equality 
                  (NULL, null_keyword, new_relational (NULL, null_keyword, new_shift 
                  (NULL, null_keyword, new_additive (NULL, null_keyword, new_multiplicative
		  (NULL, null_keyword, new_cast_unary (new_unary_postfix 
                  (new_postfix_primary (new_primary_constant_char (cchar))))))))))))));

  n_assignment *internal = new_assignment (NULL, keyword_opASSIGN, lor);

  if (*remaining_string == 0) 
    {
      // end of string, means end of recursion
      // add trailing \0
      return new_initializer_list( NULL, new_initializer( internal ) );
    }
  
  // otherwise, continue recursion with next char
  return new_initializer_list( create_string_equiv( initial, remaining_string + 1), new_initializer( internal) );
}

/*
 * parse initializer element recursively and return sequence SEQ(...) of irnode statement
 * if just_count is set, do not create sequence actually, but update indexes table (if appropriate) in
 * order to get the number of elements in the initializer
 */
irnode *
parse_initializer( 
  int int_or_ext, 	/* is the declaration internal or external to a function ?
                     	   this changes the way we treat the initializer, either as a dataList of values
                     	   that will be used to define the variable statically (if external), or 
                     	   an irnode sequence to produce executable code (if internal) */

  dataList dl, 		/* if external declaration, hold the list of values for static initialization */

  int just_count, 	/* if flag is set, determine initializer size but does not produce actual code or
                           values. This is used to know array size when no explicit size is given but an
       			   initializer is present */

  irnode *sequence, 	/* if internal declaration, irnode that holds instructions for initialization */

  n_id id, 		/* ID of the variable to be initialized */

  type *id_type,	/* ID type (FIXME is it useful now that we have current_type instead ?) */
 
  n_initializer *in, 	/* initializer or pointer on current position of the initializer. This position
			   is modified during recursion. */

  int indexes[],	/* contains the current position when multidimensional variable (array and/or struct).
			   Indexes size is the number of dimensions. Each value is the position in this
			   dimension (for instance, the initialization of an array of size 2 "int x[2]" will
			   produce an indexes[] of size 1 (one dimension) and value 0 or 1.
			   Indexes array is NULL (and not relevant) for scalar types */ 

  int current_index,	/* current index in the indexes[] array. This index is modified during recursion. */

  type *current_type, 	/* current type of the variable to be initialized. This type is modified during
			   recursion */

  memberList mlist,	/* if current type corresponds to a struct member, is the parent member list */

  int parent_is_union

)
{
  if (!in) return sequence; // end recursion
  irnode *new;
  type *t;

  switch( in->kind )
    {
      case init_simple:

        t = current_type;

	/* we evaluate the assignment to determine if it is a string constant */
	/* we discard the resulting irnode as the actual assignment will be done in a different
 	   way. Nevertheless, by doing an evaluation, we cause a side-effect if string, 
	   because a new fragment will be added to data section. We thus need to avoid
	   several calls to translate_assignment() if is a string, and get the initial label 
	   and string-value once for all */
	int is_string;
	char *string_value;
	label *string_label;

	irnode *discard = translate_assignment (in->assign);

	is_string = (IS_LITERAL(discard) &&  GET_MODE(discard) == STR_mode);
	if (is_string) 
	  {
	    string_value = GET_LITERAL(discard);
	    string_label = (label *) GET_IRCHILD( GET_IRCHILD(discard,0), 0); /* we suppose MEM(NAME) */
	  }

        if ( is_array(t) && 
             ( are_compatible_type( t->u.pointer_on, Char ) ||
               are_compatible_type( t->u.pointer_on, Uchar)) && 
               is_string )
          {
            // when an array[] of char is initialized with a literal "string", 
            // we consider this string as equivalent to { 's', 't', 'r', 'i', 'n', 'g', '\0' }  
            // this is the only case where an array can be initialized with a single value
           
            // create equivalent { } list
            n_initializer_list *string_equiv = create_string_equiv( in, string_value );

            in = new_initializer_bracket( string_equiv );
            // fallthrough to case init_bracket !

          }
        else 
          {
            if (just_count) break;

            //if ( indexes != NULL && current_index == -1 ) 
            if ( is_array(t) && current_index == -1 ) 
              {
                keen_error("cannot initialize an array with a single value");
              }

	    if ( is_struct_or_union(t) && current_index == -1 )
	      {
		  /* this means that this is a simple struct assignment. No need to parse initializer */
		  free (indexes);
		  indexes = NULL;
	      }

            if (int_or_ext == int_decl )
              {
                // internal declaration
                // we create an actual assignment that will be translated to assembler later
                n_keyword null_keyword = { {0, 0, NULL} , 0 };
                n_keyword keyword_opASSIGN = { {0, 0, NULL} , opASSIGN };

                n_assignment *internal = 
			new_assignment (in->assign, keyword_opASSIGN, create_initialization_lor ( id, id_type, indexes, current_index ) );

		internal->is_declaration = 1;
			/* we flag this assignment as a 1st declaration */

                new = translate_assignment( internal );

              }
            else
              {
                // external declaration
                // we create a list (sequence) of constants values that will be used to 
                // initialize the variable statically
                new = discard;

		/* initializer must be constant, ie. 
 		   - numeric literal 
		   - string constant 
		   - address of a global variable
		   FIXME more cases to manage surely
		*/
                if (NODETYPE(new) != n_CONST && !is_string && 
		     !(NODETYPE(new) == n_MEM && NODETYPE(GET_IRCHILD(new,0)) == n_NAME))
                  {
                    keen_error("cannot assign a non-constant value");
                  } 

		 /* current type is a member of struct. Need to get corresponding padding */
		 int padding = 0;
		 if (mlist) 
 		      {
		        int rank = indexes[ current_index ];
		        member *m = object( get_ith( mlist, rank) );
		        padding = m->padding; 
		      }

		/* treat numeric constant */
 		if (NODETYPE(new) == n_CONST) 
		  {

		    int i = *(int *) GET_IRCHILD (new,0);
                    data *d = create_data( 
                            get_type_natural_mode(current_type),
                            padding,
                            stringf("%d",i)); 

                    add (dl, d);

		  }

		/* treat string constant */
	        if (is_string) 
		  {

                    data *d = create_data( 
                            get_type_natural_mode(current_type),
                            padding,
                            stringf("%s",get_label_name(string_label))); 

                    add (dl, d);

		  }

		/* treat address of a global variable */
	        if (!is_string && NODETYPE(new) == n_MEM && NODETYPE(GET_IRCHILD(new,0)) == n_NAME)
		  {

                    data *d = create_data( 
                            get_type_natural_mode(current_type),
                            padding,
                            stringf("%s",get_label_name((label *)GET_IRCHILD(GET_IRCHILD(new,0),0)))); 

                    add (dl, d);

		  }
 
		/* this does not create a return sequence. We just return nop operation */
                new = nNOP();
              }

            return nSEQ( sequence, getStmt(new) );
          }

      case init_bracket:

        current_index++; // we go deeper in the array or struct initialization
         
 	memberList ml = NULL; /* a priori not a member of struct. May change below */
 	int is_union = 0;
 
        /* check type and dimension boundary */
	if (is_array(current_type)) {
	    t = current_type;
	    ml = NULL; /* not a struct */
	  } 
 	else if (is_struct_or_union(current_type)) {
	    t = current_type;
	    ml = current_type->u.members; /* is a struct. Pass member list as well */
	    is_union = (current_type->kind == t_union);
	  } 
	else
          {
            // must not have more dimensions in the initializer than in the id itself,
            // except that a scalar can be initialized with a list { } of one element !
            if (!(is_scalar(current_type) && current_index == 0)) keen_error("too many levels in the { } initialization");
          }

        new = parse_initializer_list( int_or_ext, dl, just_count, sequence, id, id_type, 
                                      in->init_list, indexes, current_index, t, ml, is_union );
 
        return new;

        break;

      default: assert(0);
    }
}

/*
 * translate a direct declarator (within a function) recursively
 * return an irnode (statement) if initialization need to be performed (as in "int x = 1;")
 * if no initialization is required, return nNOP()
 * current_type is amended recursively (in particular when array)
 * actual declaration ( = insertion into symbol table ) in performed at end
 * of recursion
 *
 * if in_struct_or_union is set, means that we are parsing a struct/union member. Do not perform any insertion
 * into symbol table and return declarator actual type/name in member_type/name parameters
 */
irnode *
translate_direct_declarator( int int_or_ext, n_direct_declarator *dd, type *basetype, 
                             int storage, n_initializer *in, int function_main_block,
                             int in_struct_or_union, type **member_type, char **member_name )
{

  symbol *s = NULL;
  bind *b = NULL;
  type *t = basetype;
  parameterList tl = NULL;
  accessor *access = NULL;
  int ellipsis = 0;

  irnode *result = nNOP();
  irnode *size;
  int actual_size;
 
  int fragment_storage_required = 0;
  label *slabel = NULL;
  dataList dl = newList();

  SET_POS (dd->pos);

  switch (dd->kind)
	{

	case dd_ptl:		// FUNCTION DECLARATION 
	  // parse parameters
	  tl = parse_ptl (dd->u.d.ptl, &ellipsis, 1 /* abstract declarators are allowed */);
	  // create binding for future use
	  b = new_bind_func (t, tl, ellipsis, 
			   0, 0, NULL, dd->pos);
	  // get function declarator
	  dd = dd->u.d.dd;
	  // check for previous declarations and/or definitions
	  int store_required = 1;	// a priori, symbol storage in symbol table is required 
	  s = lookup_symbol (env, dd->u.name);
	  if (s != NULL)
	    {
	      bind *bi = s->binding;
	      // previously declared as something not a function...
	      if (bi->kind != b_func)
		{
		  keen_error
		    ("symbol '%s' is already declared as a different kind of symbol",
		     dd->u.name);
		}
	      if (bi->u.f.is_defined)
		{
		  // function is declared and already defined, check prototypes for exact match
		  if (!are_functions_prototypes_compatible (b, bi, 1))
		    {
		      keen_errorn ("conflicting types for '%s'", dd->u.name);
		      SET_POS (bi->u.f.first_decl_position);
		      keen_error ("function '%s' is declared here",
				  dd->u.name);
		    }
		  store_required = 0;	// no need to store since function was already defined 
		}
	      else
		{
		  // function is declared and not defined, check prototypes 
		  if (!bi->u.f.is_defined)
		    {
		      if (!are_functions_prototypes_compatible (b, bi, 0))
			{
			  keen_errorn ("conflicting types for '%s'",
				       dd->u.name);
			  SET_POS (bi->u.f.first_decl_position);
			  keen_error ("function '%s' is declared here",
				      dd->u.name);
			}
		    }
		}
	    }
	  if (store_required)
	    {
	      insert_symbol (env, dd->u.name, b);
	    }

	  break;

        case dd_array:

          if (dd->u.a.expr)
            {
            // FIXME today we only support constant
            size = translate_expression( dd->u.a.expr );
            if (!is_arithmetic( GET_TYPE(size) ) || !(NODETYPE(size)==n_CONST))
              {
                keen_error("invalid array size");
              }
  
            // create a new type based on previous one
            actual_size = *(int *) GET_IRCHILD( size, 0 ); 
            if (actual_size <= 0) 
              {
                keen_error("array size must be greater than 0");
              }

           }
          else
           {
             // uncomplete array type. we will check later (in recursion) if allowed
             actual_size = undefined;
           }

          // if array or array, check that previous one is not uncomplete
          if (is_array(t)) 
            {
              if (get_array_size(t) == undefined)
                keen_error("empty array size not allowed here");
            }

          // create the new type, based on previous one
          t = type_array( t , actual_size );

          return translate_direct_declarator( int_or_ext, dd->u.a.dd, t, storage, in, 
                                              function_main_block, in_struct_or_union, member_type, member_name );

	case dd_id:		/* IDENTIFIER DECLARATION, may be type or variable */

          // if in struct, just set member type and name
          if (in_struct_or_union)
            {
              *member_name = string(dd->u.name);
              *member_type = t; 
              break;
            }

	  // check that the symbol has not been declared already in same scope
	  s = lookup_symbol (env, dd->u.name);
	  if ((s != NULL) && (s->scope == env->current_scope))
	    {
	      if (function_main_block && s->binding->is_parameter)
		{
		  keen_warning
		    ("in function '%s', symbol '%s' shadows a parameter",
		     in_function->name, dd->u.name);
		}
	      else
		{
		  if (s->binding->kind != b_var || 
			(s->binding->kind == b_var && !s->binding->u.v.is_extern && storage != storage_extern)) 
				keen_error ("symbol '%s' is already declared", dd->u.name);
		}
	    }

          // we are about to store the symbol
          // but if type is uncomplete, need to get its actual size before
          n_id id;
          id.pos.line = dd->pos.line;
          id.pos.column = dd->pos.column;
          id.pos.filename = dd->pos.filename;
          id.v = string(dd->u.name);

          if (is_array(t) && !is_complete(t) && in )
            {
		  // First, check that element type itself is complete
		  if (!is_complete(t->u.pointer_on)) keen_error("cannot initialize array %s[] of incomplete elements", dd->u.name);

                  // get type depth
                  int maxdim = get_depth (t); 

		  // allocate index table
                  int *indexes = (int *) xcalloc( maxdim, sizeof(int)); // will hold index sequence during { } parsing

                  parse_initializer ( int_or_ext, dl, 1 /* just count */, nNOP(), id, t, in, 
                                      indexes , -1 /* not entered the { } initializer yet */, 
				      t, 0, 0 );

                  keen_debug("array initialization: setting size of %s[] to %d\n",id.v,indexes[0]+1);

                  t->array_size = indexes[0]+1;
                  t->is_complete = 1;
            }

	  // store the symbol
	  switch (storage)
	    {

	    case storage_typedef:
              if (is_struct_or_union(t)) 
                keen_debug("creating typedef based on struct type for '%s' (complete=%d, t pointer=%d)\n",
                            dd->u.name,t->is_complete,t);
	      b = new_bind_type (t);
	      break;

	    case storage_extern:
	    case storage_static:
	    case storage_auto:
	    case storage_register:
              if (!t->is_complete && storage != storage_extern)
                {
                  keen_error("cannot create variable %s with unknown size (type is incomplete)", dd->u.name);
                }
              if ( int_or_ext == ext_decl || (int_or_ext == int_decl && storage == storage_extern))
                {
                  // EXTERNAL DECLARATOR
                  slabel = new_named_label (dd->u.name);
                  // create new fragment for further data section
                  access = new_global_var ( get_type_natural_mode (t), slabel, get_type_size (t) );
                  if (storage != storage_extern) fragment_storage_required = 1;
                }
              else
                {
                  // INTERNAL DECLARATOR
	          access = new_var ( get_type_natural_mode (t),
			 in_function->binding->u.f.f_frame, 1 /* escapes */ ,
			 get_type_size (t) );
                }
	      b = new_bind_var (t, 0 /* not a parameter */ , access, (storage == storage_extern) );
	      break;
	    }

	  // update symbol table
	  s = insert_symbol (env, dd->u.name, b);
          keen_debug("creating variable %s, mode %d\n",dd->u.name, get_type_natural_mode(t));
          print_type( t );

          // if no initializer and external declaration, set variable to zero
          if (!in && fragment_storage_required)
            {
              if (is_scalar(t))
                {
                  data *d = create_data ( get_type_natural_mode(t), 0 /* offset */, "0" /* value */);
                  add( dl, d);
                }
              else
                {
                  // a non-scalar un-initialized has no datalist
                }
            }

	  // treat initializer, if any
	  if (in)
	    {

	      if (storage == storage_typedef)
		{
		  keen_error ("initialization is not allowed here");
		}

              if (is_array(t) || is_struct_or_union(t))
                { 
                  // get type depth
                  int maxdim = get_depth (t); 

		  // allocate index table
                  int *indexes = (int *) xcalloc( maxdim, sizeof(int)); // will hold index sequence during { } parsing

                  result = parse_initializer ( int_or_ext, dl, 0 /* don't count */, 
                                               nNOP(), id, t, in, indexes , 
                                               -1 /* not entered the { } initializer yet */,
					       t, 0, 0 );
                }
	      else
                {
                  // this is a scalar 
                  result = parse_initializer ( int_or_ext, dl, 0 /* don't count */, 
                                               nNOP(), id, t, in, NULL , -1,
					       t, 0, 0 );
                } 

              keen_debug("INITIALIZER STMT=\n"); 
              if (debug_level()) print_tree( result,0);
              keen_debug("END INITIALIZER STMT\n"); 

	    }

            if (fragment_storage_required)
              {
                  keen_debug("creating a fragment with label '%s', scalar = %d\n",
                             get_label_name(slabel), is_scalar(t)); 
                  add (FRAGMENTS, new_fragment_gvar (access,
                                     nDATA ( is_scalar(t),      /* scalar */ 
                                             (in != NULL),      /* initialized */
                                             get_type_size(t),  /* total size */
                                             slabel,            /* data label */
                                             dl                 /* data list */ 
                                           )));

                  result = nNOP();
              }

	  break;

  }	/* end switch(dd->kind) */

  return result;

}

/*
 * translate a declaration within a function
 * if <function_main_block> is set to 1, the declarations are made just inside a 
 * function main-block. This implies some additional checks on parameter shadowing.
 * The current function is pointed to by the global variable <in_function> 
 */

irnode *
translate_int_ext_declaration( int int_or_ext, n_declaration *p, int function_main_block )
{

  int storage = get_storage_class (p->decl);

  // by default an internal decl is automatic. An external decl is static
  if (storage == undefined && int_or_ext == ext_decl) storage = storage_static;
  if (storage == undefined && int_or_ext == int_decl) storage = storage_auto;

  irnode *result = nNOP();

  // a declaration can hold 0..N declarators. 
  // If no declarator: 
  // - must be union, struct, enum, or
  // - last identifier was not a typename, but an actual identifier
  n_init_declarator_list *idl = p->init;
  if (idl == NULL)
    {
      // get last type_specifier
      n_declarator_specifiers *ds, *prev_last = NULL, *last = NULL;
      ds = p->decl; 

      SET_POS(ds->pos);

      if (ds->kind == decl_spec_type ) last = ds;
      while (ds)  
	{ 
 	  if (!ds->decl) last = ds;
	  if (ds->decl) prev_last = ds;
	  ds = ds->decl;
	}

      assert(last->kind == decl_spec_type); // FIXME

      if (!last)
	{
	  // no type specifier and no declarator, this is obviously an error
	  keen_error ("invalid declaration");
	}

      n_type_specifier *ts = last->u.type_spec;

      // if no declarator but enum or struct/union, this is not an error and
      // we continue parsing
      if (!(ts->kind == type_spe_enum) && !(ts->kind == type_spe_su))
	{
      
	  // if no declarator and last id is a typename, this is an error in this
      	  // context (and can be allowed only in struct/union declaration)
      	  symbol *s = lookup_symbol (env, ts->name);
      	  if (s && s->binding->kind == b_type) keen_error("invalid declaration");

      	  // otherwise we transform this type_spe into a declarator
      	  n_id n;
      	  n.v = string(ts->name);
      	  n.pos.line = ts->pos.line;
      	  n.pos.column = ts->pos.column;
      	  n.pos.filename = ts->pos.filename;

      	  n_direct_declarator *id = new_direct_declarator_id(n);
      	  n_declarator *nd = new_declarator ( NULL, id );
      	  n_init_declarator *nid = new_init_declarator ( nd, NULL );

      	  if (prev_last) prev_last->decl = NULL;
      	  idl = new_init_declarator_list ( nid, NULL );
	
	}

    }

  // parse specifiers and get base type
  type *basetype = get_base_type (int_or_ext, p->decl);

  // a declaration can hold several declarators. Treat them sequentially
  for (; idl != NULL; idl = idl->list)
    {

      n_init_declarator *id = idl->init;
      n_declarator *d = id->decl;
      n_initializer *in = id->init;

      // for each declarator, complete the base type if needed
      type *ty = complete_base_type (d, basetype);

      // treat declarator itself
      n_direct_declarator *dd = d->decl;

      result = nSEQ( result , 
                     translate_direct_declarator ( int_or_ext, dd , ty, storage, in, 
                                                   function_main_block, 0, NULL, NULL ));

      keen_debug("FINAL DECLARATOR IS:\n");
      if (debug_level()) print_tree( result, 0 );
      keen_debug("END FINAL DECLARATOR\n");
    }	

  return result;

}

// 
// TRANSLATE PRIMARY
//
irnode *
translate_primary (n_primary * p)
{
  irnode *t;
  symbol *s;
  bind *b;
  type *ty;
  label *constant_label;
  accessor *access;

  SET_POS (p->pos);

  switch (p->kind)
    {
    case primary_constant:
      ty = Int;
      t = nCONSTm (m32, p->u.constant);
      SET_TYPE (t, ty);
      SET_LITERAL (t);
      int *vconst = (int *) xmalloc(sizeof(int));
      *vconst = p->u.constant;
      SET_LITERAL_VALUE(t,vconst);
      return t;

    case primary_constant_string:

     if (!p->label_name)
      {
       // create new fragment for further data section
       constant_label = new_label ();
       access = new_data (STR_mode , constant_label, 0 /* size not significant */ );
       add (FRAGMENTS, new_fragment_data (access, nDATA ( 1, /* is scalar */
                                                         1, /* is initialized */
                                                         strlen(p->u.constant_string)+1, /* total size */
                                                         constant_label, /* label */
                                                         add( newList(), 
                                                              create_data( STR_mode, 
                                                                           0, /* offset */ 
                                                                           p->u.constant_string /* value */ )))));
       // store the newly created label name to avoid fragment duplicates
       p->label_name = get_label_name(constant_label);
      }
     else
      {
       // data fragment has already been created for this constant string. Get existing label
       constant_label = new_named_label (p->label_name);
      }

      // create and return current irnode
      ty = type_pointer( Char );
      ty->qualifier = t_const;
      t = nMEMm (STR_mode, nNAME (constant_label));
      SET_TYPE (t, ty);
      SET_LITERAL (t);
      char *vstr = string(p->u.constant_string);
      SET_LITERAL_VALUE(t,vstr);
      return t;

    case primary_constant_char:
      ty = Char /* signed FIXME */;
      t = nCONSTm (m8, p->u.constant);
      SET_TYPE (t, ty);
      SET_LITERAL (t);
      char *vchar = (char *) xmalloc(sizeof(char));
      *vchar = p->u.constant;
      SET_LITERAL_VALUE(t,vchar);
      return t;
    case primary_expr:
      t = translate_expression (p->u.expr);
      return t;
    case primary_identifier:
      /* get identifier in symbol table */
      s = lookup_symbol (env, p->u.id);
      if (s == NULL)
	{
	  keen_error ("identifier '%s' undeclared", p->u.id);
	}
      /* get type */
      b = s->binding;
      switch (b->kind)
	{
	case b_type:
	  keen_error ("unexpected type '%s' here", p->u.id);	/* should not happen */
	case b_var:
          if (is_array(b->t) || is_struct_or_union(b->t)) 
            {
	      t = getAddress (b->u.v.access);
              if (NODETYPE(t) == n_NAME) t = nMEMm ( get_type_natural_mode(b->t), t );
            }
          else
            {
	      t = getMemoryExp (b->u.v.access);
            }
	  SET_TYPE (t, b->t);
	  SET_LVALUE (t, is_modifiable_lvalue (b->t));
	  SET_SYMBOL (t, s);
	  return t;

	case b_func:
	  return nNOP ();

	case b_const:
      	  t = nCONSTm (m32, b->u.c.value);
          SET_TYPE (t, b->t);
          SET_LITERAL (t);
	  int *value = (int *) xmalloc (sizeof(int));
 	  *value = b->u.c.value;
	  SET_LITERAL_VALUE(t,value);
	  return t;

	}
      assert (0);
    }
}

//
// TRANSLATE POSTFIX
//
irnode *
translate_postfix (n_postfix * p)
{
  irnode *a, *b, *c;
  symbol *f;
  irnode *ecall;
  irnodeList elist;
  irnode *se;
  irnode *e;
  irnode *x;
  n_argument_expr_list *arglist = NULL;
  type *return_type = NULL;
  temp *r;
  type *t;
  char *member_name;
  member *found_member;
  memberList mlist;
  ListElem el;

  SET_POS (p->pos);

  switch (p->kind)
    {

    case postfix_primary:

      return translate_primary (p->u.pri->pri);

    case postfix_pointer:

      a = translate_postfix (p->u.pointer->postfix); 

      t = GET_TYPE(a);

      if (!is_pointer(t) || !is_struct_or_union( t->u.pointer_on ))
        {
          keen_error("not a valid pointer to struct or union");
        }
      if (!t->u.pointer_on->is_complete)
        {
          keen_debug("cannot take member of an incomplete struct/union type (t pointer=%d)",t);
          keen_error("cannot take member of an incomplete struct/union type");
        }

      // find member in type t
      member_name = p->u.pointer->id;
      mlist = t->u.pointer_on->u.members;
      found_member = NULL;
      member *m;
      FOR_EACH(m,mlist,e1)
        {
          if (strcmp(m->name, member_name)==0) { found_member = m; break; }
        }

      if (!found_member)
        {
          keen_error("struct/union has no member %s", member_name );
        }

      // return a memory location a priori, except if array or struct
      if ( is_array(found_member->t) || is_struct_or_union(found_member->t) )
        {
          c = nADD( a , nCONST( found_member->offset) );
        }
      else
        {
          c = nMEMm( get_type_natural_mode ( found_member->t ),
                 nADD( a , nCONST( found_member->offset) ) );
        }
      SET_TYPE(c , found_member->t );
      SET_LVALUE(c , is_modifiable_lvalue(found_member->t));
      SET_SYMBOL(c, GET_SYMBOL (a) );

      return c;

    case postfix_dot:

      a = translate_postfix (p->u.dot->postfix); 

      t = GET_TYPE(a);
      if (!is_struct_or_union( t ))
        {
          keen_error("not a struct or union");
        }
      if (!t->is_complete)
        {
          keen_debug("cannot take member of an incomplete struct/union type (t pointer=%d)",t);
          keen_error("cannot take member of an incomplete struct/union type");
        }
      // find member in type t
      member_name = p->u.dot->id;
      mlist = t->u.members;
      found_member = NULL;
      FOR_EACH(m,mlist,e2)
        {
          if (strcmp(m->name, member_name)==0) { found_member = m; break; }
        }

      if (!found_member)
        {
          keen_error("struct/union has no member %s", member_name );
        }

      // return a memory location a priori, except if array or struct
      if ( is_array(found_member->t) || is_struct_or_union(found_member->t) )
        {
          c = nADD( a , nCONST( found_member->offset) );
        }
      else
        {
          c = nMEMm( get_type_natural_mode ( found_member->t ),
                 nADD( a , nCONST( found_member->offset) ) );
        }
      SET_TYPE(c , found_member->t );
      SET_LVALUE(c , is_modifiable_lvalue(found_member->t));
      SET_SYMBOL(c, GET_SYMBOL (a) );

      return c;

    case postfix_array:

      a = translate_postfix (p->u.array->postfix); 

      // check validity
      if (!is_pointer(GET_TYPE(a)))
        {
          keen_error("not an array or pointer");
        }

      // evaluate expression
      b = translate_expression( p->u.array->expr );

      irnode *ir_add, *ir_mul;
      // perform a basic constant folding right now, to avoid useless add/mul
      if (NODETYPE(b) == n_CONST)
            {
              ir_mul = nCONST( (*(int *)GET_IRCHILD( b, 0)) * get_type_size (GET_TYPE (a)->u.pointer_on) );
            }
      else
            {
              ir_mul =  nMUL ( b , nCONST ( get_type_size (GET_TYPE (a)->u.pointer_on)));
            }

      if (NODETYPE(a) == n_CONST && NODETYPE(ir_mul) == n_CONST)
            {
              ir_add = nCONST(  *(int *)GET_IRCHILD( a, 0) +  *(int *)GET_IRCHILD( ir_mul, 0) );
            }
      else
            {
              ir_add = nADD( a , ir_mul );
            }
      // return an offset a priori, except if last indexation level or struct/union
      if (is_array(GET_TYPE (a)->u.pointer_on) || is_struct_or_union( GET_TYPE(a)->u.pointer_on))
        {
          c = ir_add;
          SET_TYPE( c, (GET_TYPE (a)->u.pointer_on) );
          SET_LVALUE( c , is_modifiable_lvalue(GET_TYPE(a)->u.pointer_on)); 
          SET_SYMBOL( c, GET_SYMBOL (a) );
        }
      else
        {
          // return a memory location
          c = nMEMm( get_type_natural_mode (GET_TYPE (a)->u.pointer_on), 
                     ir_add );
          SET_TYPE( c, (GET_TYPE (a)->u.pointer_on) );
          SET_LVALUE( c , is_modifiable_lvalue(GET_TYPE(a)->u.pointer_on)); 
          SET_SYMBOL( c, GET_SYMBOL (a) );
        } 

      return c;

    case postfix_call:
      /* check designator kind */
      if (!(p->u.call->designator->kind == postfix_primary &&
	    p->u.call->designator->u.pri->pri->kind == primary_identifier))
	{
	  keen_error ("invalid function designator");
	}
      /* check if designator is declared */
      f = lookup_symbol (env, p->u.call->designator->u.pri->pri->u.id);
      if (f == NULL)
	{
	  // implicit function declaration
	  // Store this symbol with return type int
	  return_type = Int;
	  bind *b = new_bind_func (return_type,
				   NULL /* no param */ ,
				   0 /* no ellipsis */ ,
				   0 /* function is not defined */ ,
				   1 /* function is implicit */ ,
				   NULL /* no frame */ ,
				   p->u.call->designator->u.pri->pri->pos);
	  // Warning: the symbol table current scope is not upmost one, so use particular insert function 
	  f =
	    insert_symbol_uppermost_scope (env,
					   p->u.call->designator->u.pri->pri->
					   u.id, b);
	  // warn user
	  keen_warning ("implicit declaration of function '%s' in '%s'",
			f->name, in_function->name);
	}
      else
	{
	  if (f->binding->kind != b_func)
	    {
	      keen_error ("'%s' is not a function", f->name);
	    }
	  return_type = f->binding->u.f.return_t;
	}

      /* produce call irnode */
      ecall = nNAME (new_named_label (stringf ("%s%s", ASM_FUNCBASE, f->name)));

      /* get arguments */
      elist = newList();

      // if return type is struct or union, create a fake 1st argument
      // that will be copied when function returns
      irnode *addr = NULL;
      if (is_struct_or_union(return_type))
        {
          // create a local variable in container function to hold result
          accessor *a = new_var( m32, in_function->binding->u.f.f_frame, 1, get_type_size(return_type) );
          // add its address as 1st parameter to called function
          addr = getAddress( a );
        }

      for (arglist = p->u.call->list; arglist != NULL;
	   arglist = arglist->list)
	{
	  x = translate_assignment (arglist->assign);
	  add (elist, x);
	}

      if (addr) add (elist, addr); // 1st parameter included last, as list is in reverse order !!
                                   // a bit clumsy, should be simplified
                                   
      se = nNOP (); // FIXME: we can remove this useless se part from CALL IRnode

      /* produce IR tree */
      e = nCALL (ecall, elist, se);
      SET_TYPE (e, return_type);

      return e;

    case postfix_post:

      /* treat expression without side effect */
      a = translate_postfix (p->u.post->postfix);

      /* check that ++ and -- operators is applied on lvalue */
      if (GET_LVALUE (a) != LVALUE)
	{
	  keen_error ("invalid lvalue in increment/decrement (*)");
	}

      /* now treat side effect. Create an (internal) assignment ( p += 1) */
      n_assignment *internal = create_assign( p, p->pos, POSTFIX_SIDE_EFFECT );

      r = new_temp();
      b = nESEQ( nMOVE( nTEMP(r), a),
                    nESEQ( getStmt( translate_assignment( internal) ),
                           nTEMP(r) ) );

      /* the result is not an lvalue */
      SET_LVALUE (b, NOT_LVALUE);
      SET_TYPE (b, GET_TYPE(a));
      SET_SYMBOL (b, GET_SYMBOL(a));

      return b;

    }
  assert (0);
}

//
// TRANSLATE UNARY CAST
//
irnode *
translate_unary_cast (n_unary_cast * u)
{
  irnode *c = translate_cast (u->cast);
  irnode *s;
  int lvalue;

  SET_POS (u->pos);

  switch (u->op)
    {
    case opNOT:
      if (is_arithmetic (GET_TYPE (c)) || is_pointer (GET_TYPE (c)))
	{
		if (NODETYPE(c) == n_CONST)
		  {
		    *(int *)GET_IRCHILD( c , 0) = !((int *)GET_IRCHILD( c , 0));
			return c;
		  }
		  
          label *t = new_label();
          label *f = new_label();
          temp *r = new_temp();
	  s = nESEQ( nCJUMP (nEQ (c, nCONST(0)), t, f),
                     nESEQ( nLABEL(t),
                            nESEQ( nMOVE( nTEMP(r), nCONST(1)),
                                   nESEQ( nLABEL(f),
                                          nESEQ( nMOVE( nTEMP(r), nCONST(0)),
                                                 nTEMP(r))))));
	  SET_TYPE (s, Int);
	  return s;
	}
      else
	{
	  keen_error ("invalid operand in negation expression");
	}

    case opUPLUS:
      if (is_arithmetic (GET_TYPE (c)))
	{
	  return c;
	}
      else
	{
	  keen_error ("invalid operand in unary '+' expression");
	}

    case opUMINUS:
      if (is_arithmetic (GET_TYPE (c)))
	{
          if (NODETYPE(c) == n_CONST)
            {
              *(int *)GET_IRCHILD( c , 0) *= -1; 
            }
          else
            {
              c = getNEG(c);
	      SET_TYPE(c,Int); // FIXME
            }
          return c;
	}
      else
	{
	  keen_error ("invalid operand in unary '-' expression");
	}

    case opIND:
      if (!is_pointer (GET_TYPE (c)))
	{
	  keen_error ("invalid operand in unary '*' expression");
	}
      //lvalue = ((is_arithmetic (GET_TYPE (c)->u.pointer_on)) || (is_pointer (GET_TYPE (c)->u.pointer_on)));
      if (is_struct_or_union(GET_TYPE(c)->u.pointer_on))
        {
          s =  c;
        }
      else
        {
          s = nMEMm ( get_type_natural_mode( GET_TYPE (c)->u.pointer_on ), c );
        }
      lvalue = is_modifiable_lvalue (GET_TYPE(c)->u.pointer_on);
      SET_LVALUE (s, lvalue);
      SET_TYPE (s, GET_TYPE (c)->u.pointer_on);
      SET_SYMBOL (s, c->s);
      return s;

    case opADDR:
 
      // check that we take the address of an lvalue (an object or a derivative
      // type of an object).
      if (is_lvalue(c) != LVALUE) 
	{
	  keen_error ("address operator on something not an lvalue");
	}

      s = getAddress (c->s->binding->u.v.access);
      if (NODETYPE(s)==n_NAME) s = nMEM ( s );
      SET_TYPE (s, type_pointer (GET_TYPE (c)));
      SET_LVALUE (s, NOT_LVALUE);
      SET_SYMBOL (s, GET_SYMBOL (c));
      return s;

    case opTILDE:		/* one's complement FIXME */
      if (is_arithmetic (GET_TYPE (c)))
	{
          if (NODETYPE(c) == n_CONST)
            {
              *(int *)GET_IRCHILD( c , 0) = ~(*(int *)GET_IRCHILD( c , 0)); 
            }
          else
            {
	      c = nCOMP ( c );
	      SET_TYPE(c,Int); // FIXME
            }
          return c;
	}
      else
	{
	  keen_error ("invalid operand in unary '~' expression");
	}

    }
  assert (0);
}

//
// TRANSLATE UNARY PREFIX
//
irnode *
translate_unary_prefix (n_unary_prefix * u)
{

  /* translate prefix without side effect */
  irnode *a = translate_unary (u->unary);

  SET_POS (u->pos);

  /* check that ++ and -- operators is applied on lvalue */
  if (GET_LVALUE (a) != LVALUE)
    {
      keen_error ("invalid lvalue in pre-increment/decrement");
    }

  /* now treat side effect. Create an (internal) assignment ( p += 1) */
  n_assignment *internal = create_assign( u, u->pos, PREFIX_SIDE_EFFECT );

  temp *r = new_temp();
  irnode *b = nESEQ( getStmt( translate_assignment( internal) ),
                     a );

  /* the result is not an lvalue */
  SET_LVALUE (b, NOT_LVALUE);
  SET_TYPE(b, GET_TYPE(a));
  SET_SYMBOL(b, GET_SYMBOL(a));
  
  return b;
}


//
// TRANSLATE UNARY SIZEOF
//
irnode *
translate_unary_sizeof (n_unary_sizeof * u)
{
  type *t;
  irnode *i;
  switch (u->kind)
    {
      case sizeof_expr:  
	i = translate_unary (u->u.un);
	t = GET_TYPE (i);
	if (!is_complete(t))
	  {
	    keen_error ("cannot apply sizeof to operand with incomplete type");
	  }
	i = nCONST ( get_type_size (t) );
	SET_TYPE (i, Int );
	return i;

      case sizeof_typename:
 	t = parse_type_name (u->u.tn);	
	if (!is_complete(t))
	  {
	    keen_error ("cannot apply sizeof to incomplete type");
	  }
	i = nCONST ( get_type_size (t) );
	SET_TYPE (i, Int );
	return i;
    }
  assert(0);
}


//
// TRANSLATE CAST UNARY
//
irnode *
translate_unary (n_unary * u)
{
  switch (u->kind)
    {
    case unary_postfix:
      return translate_postfix (u->u.postfix->postfix);
    case unary_prefix:
      return translate_unary_prefix (u->u.prefix);
    case unary_cast:
      return translate_unary_cast (u->u.cast);
    case unary_sizeof:
      return translate_unary_sizeof (u->u.sof);
    }
  assert (0);
}

// 
// TRANSLATE CAST
//
irnode *
translate_cast (n_cast * cast)
{
  switch (cast->kind)
    {
    case cast_cast:

      assert(cast->u.cast->cast);

      irnode *ret = translate_cast (cast->u.cast->cast);
      type *t = parse_type_name (cast->u.cast->tname); 

      keen_debug("Cast to type: ");
      print_type(t);

      // perform some sanity checks 
      if ( !is_scalar(t) || !is_scalar( GET_TYPE(ret) ))
        {
	  keen_error("cannot cast from/to non-scalar types");
        }

      // result of cast is not an lvalue
      SET_LVALUE (ret, NOT_LVALUE);
      SET_TYPE (ret, t);
      return ret;	

    case cast_unary:
      return translate_unary (cast->u.unary->unary);
    }
  assert (0);
}


//
// TRANSLATE MULTIPLICATIVE
//
irnode *
translate_multiplicative (n_multiplicative * mul)
{
  irnode *cast = translate_cast (mul->cast);
  
  if (!mul->mul) return cast;
  
  irnode *s;
  irnode *m = translate_multiplicative (mul->mul);
  
  switch (mul->op)
	{
	case opMUL:
	
	  if ( NODETYPE(cast) == n_CONST && NODETYPE(m) == n_CONST)
		{
		  int operand1 = *(int *) GET_IRCHILD(m,0);
		  int operand2 = *(int *) GET_IRCHILD(cast,0);
		  s = nCONST (operand1 * operand2 );
		}
	  else
	    {
		  s = nMUL (m , cast);
		}
	  break;
	  
	case opDIV:
	  if ( NODETYPE(cast) == n_CONST && NODETYPE(m) == n_CONST)
		{
		  int operand1 = *(int *) GET_IRCHILD(m,0);
		  int operand2 = *(int *) GET_IRCHILD(cast,0);
		  s = nCONST (operand1 / operand2 );
		}
	  else
	    {
		  s = nDIV (m , cast);
		}
	  break;
	  
	case opMOD:
	  if ( NODETYPE(cast) == n_CONST && NODETYPE(m) == n_CONST)
		{
		  int operand1 = *(int *) GET_IRCHILD(m,0);
		  int operand2 = *(int *) GET_IRCHILD(cast,0);
		  s = nCONST (operand1 % operand2 );
		}
	  else
	    {
		  s = nMOD (m , cast);
		}
	  break;
	  
	default: assert(0);
	}
	
  SET_TYPE (s, Int);	/* FIXME */
  return s;
	  
}


//
// TRANSLATE ADDITIVE
//
irnode *
translate_additive (n_additive * add)
{

  irnode *mul = translate_multiplicative (add->mul);

  if (!add->add) return mul;
   
  irnode *ad = translate_additive (add->add);
  type *return_type;
  irnode *result;

  // standard addition
  if (is_arithmetic (GET_TYPE (mul)) && is_arithmetic (GET_TYPE (ad)))
	{
	  return_type = Int;	/* FIXME */
	  switch (add->op)
	    {
	    case opPLUS:
		  if (NODETYPE(ad) == n_CONST && NODETYPE(mul) == n_CONST)
		    {
			  int operand1 = *(int *) GET_IRCHILD( ad, 0 );
			  int operand2 = *(int *) GET_IRCHILD(mul, 0 );
			  result = nCONST ( operand1 + operand2 );
			}
		  else
		    {
			  result = nADD (ad, mul);
			}
	      
	      SET_TYPE (result, return_type);
	      return result;
		  
	    case opMINUS:
	      if (NODETYPE(ad) == n_CONST && NODETYPE(mul) == n_CONST)
		    {
			  int operand1 = *(int *) GET_IRCHILD( ad, 0 );
			  int operand2 = *(int *) GET_IRCHILD(mul, 0 );
			  result = nCONST ( operand1 - operand2 );
			}
		  else
		    {
			  result = nSUB (ad, mul);
			}
			
	      SET_TYPE (result, return_type);
	      return result;
	    }
	}

  // pointer substraction 
  if (add->op == opMINUS && is_pointer (GET_TYPE (mul))
	  && is_pointer (GET_TYPE (ad))
	  && are_compatible_type (GET_TYPE (mul)->u.pointer_on,
			    GET_TYPE (ad)->u.pointer_on))
	{
	  result = nDIV (nSUB (ad, mul),
			 nCONST (get_type_size (GET_TYPE (ad)->u.pointer_on)));
	  SET_TYPE (result, Int);
	  return result;
	}

  // otherwise, check if arithmetic on pointers 

  // addition
  if (add->op == opPLUS)
	{
	  if (is_arithmetic (GET_TYPE (mul)) && is_pointer (GET_TYPE (ad)))
	    {
	      // addition on pointers
	      result =
		nADD (ad,
		      nMUL (mul,
			    nCONST (get_type_size (GET_TYPE (ad)->u.pointer_on))));
	      SET_TYPE (result, GET_TYPE (ad));
	      return result;
	    }
	  else if (is_arithmetic (GET_TYPE (ad))
		   && is_pointer (GET_TYPE (mul)))
	    {
	      // addition on pointers
	      result =
		nADD (mul,
		      nMUL (ad,
			    nCONST (get_type_size (GET_TYPE (mul)->u.pointer_on))));
	      SET_TYPE (result, GET_TYPE (mul));
	      return result;

	    }
	}

  // substraction
  if (add->op == opMINUS)
	{
	  if (is_arithmetic (GET_TYPE (mul)) && is_pointer (GET_TYPE (ad)))
	    {
	      // substraction on pointers
	      result =
		nSUB (ad,
		      nMUL (mul,
			    nCONST (get_type_natural_mode (GET_TYPE (ad)->u.pointer_on))));
	      SET_TYPE (result, GET_TYPE (ad));
	      return result;
	    }
	}

  // otherwise, invalid operands
  SET_POS (add->pos);
  keen_error ("invalid operands in additive expression");
   
}


//
// TRANSLATE SHIFT
//
irnode *
translate_shift (n_shift * s)
{
  irnode *add = translate_additive (s->add);
  
  if (!s->shift) return add;
  
  irnode *result;
  irnode *m = translate_shift (s->shift);
  
  switch (s->op)
	{
	case opSHIFTL:
	
	  if (NODETYPE(m) == n_CONST && NODETYPE(add) == n_CONST)
	    {
		  int operand1 = *(int *) GET_IRCHILD(  m, 0 );
		  int operand2 = *(int *) GET_IRCHILD(add, 0 );
		  result = nCONST ( operand1 << operand2 );
		}
	  else
	    {
		  result = nLSHIFT (m, add);
		}
	  
	  SET_TYPE (result, Int);	/* FIXME */
	  return result;
	
	case opSHIFTR:
	  if (NODETYPE(m) == n_CONST && NODETYPE(add) == n_CONST)
	    {
		  int operand1 = *(int *) GET_IRCHILD(  m, 0 );
		  int operand2 = *(int *) GET_IRCHILD(add, 0 );
		  result = nCONST ( operand1 >> operand2 );
		}
	  else
	    {
		  result = nRSHIFT (m, add);
		}
		
	  SET_TYPE (result, Int);	/* FIXME */
	  return result;
	  
	default: assert(0);
	}
 
}


//
// TRANSLATE RELATIONAL
//
irnode *
translate_relational (n_relational * rel)
{
  irnode *s = translate_shift (rel->shift);
  if (!rel->rel) return s;
  
  irnode *result;
  irnode *m = translate_relational (rel->rel);
  switch (rel->op)
	{
	case opGE:
	  if (NODETYPE(m) == n_CONST && NODETYPE(s) == n_CONST)
	    {
		  int operand1 = *(int *) GET_IRCHILD( m, 0 );
		  int operand2 = *(int *) GET_IRCHILD( s, 0 );
		  result = nCONST ( operand1 >= operand2 ); /* FIXME get mode for Int */
		}
	  else
	    {
		  result = nGE (m, s);
		}
	  SET_TYPE (result, Int);	/* int by construction */
	  return result;
	  
	case opGT:
	  if (NODETYPE(m) == n_CONST && NODETYPE(s) == n_CONST)
	    {
		  int operand1 = *(int *) GET_IRCHILD( m, 0 );
		  int operand2 = *(int *) GET_IRCHILD( s, 0 );
		  result = nCONST ( operand1 > operand2 ); /* FIXME get mode for Int */
		}
	  else
	    {
		  result = nGT (m, s);
		}
	  SET_TYPE (result, Int);	/* int by construction */
	  return result;
	  
	case opLE:
	  if (NODETYPE(m) == n_CONST && NODETYPE(s) == n_CONST)
	    {
		  int operand1 = *(int *) GET_IRCHILD( m, 0 );
		  int operand2 = *(int *) GET_IRCHILD( s, 0 );
		  result = nCONST ( operand1 <= operand2 ); /* FIXME get mode for Int */
		}
	  else
	    {
		  result = nLE (m, s);
		}
	  SET_TYPE (result, Int);	/* int by construction */
	  return result;
	  
	case opLT:
	  if (NODETYPE(m) == n_CONST && NODETYPE(s) == n_CONST)
	    {
		  int operand1 = *(int *) GET_IRCHILD( m, 0 );
		  int operand2 = *(int *) GET_IRCHILD( s, 0 );
		  result = nCONST ( operand1 < operand2 ); /* FIXME get mode for Int */
		}
	  else
	    {
		  result = nLT (m, s);
		}
	  SET_TYPE (result, Int);	/* int by construction */
	  return result;
	  
	default:
      assert (0);
    }
 
}

// 
// TRANSLATE EQUALITY
//
irnode *
translate_equality (n_equality * eq)
{
  irnode *rel = translate_relational (eq->rel);
  if (!eq->eq) return rel;
  
  irnode *result;
  irnode *m = translate_equality (eq->eq);
      
  switch (eq->op)
	{
	case opEQ:
	  if (NODETYPE(m) == n_CONST && NODETYPE(rel) == n_CONST)
	    {
		  int operand1 = *(int *) GET_IRCHILD( m, 0 );
		  int operand2 = *(int *) GET_IRCHILD(rel, 0 );
		  result = nCONST ( operand1 == operand2 ); /* FIXME get mode for Int */
		}
	  else
	    {
		  result = nEQ (m, rel);
		}
	  SET_TYPE (result, Int);	/* int by construction */
	  return result;

	case opNE:
	  if (NODETYPE(m) == n_CONST && NODETYPE(rel) == n_CONST)
	    {
		  int operand1 = *(int *) GET_IRCHILD( m, 0 );
		  int operand2 = *(int *) GET_IRCHILD(rel, 0 );
		  result = nCONST ( operand1 != operand2 ); /* FIXME get mode for Int */
		}
	  else
	    {
		  result = nNE (m, rel);
		}
	  SET_TYPE (result, Int);	/* int by construction */
	  return result;

	default:
      assert (0);
    }

}


//
// TRANSLATE BITWISE AND
//
irnode *
translate_and (n_and * and)
{
  irnode *eq = translate_equality (and->eq);
  if (!and->and) return eq;
  
  irnode *s;
  irnode *m = translate_and (and->and);
      
  if (NODETYPE(m) == n_CONST && NODETYPE(eq) == n_CONST)
	    {
		  int operand1 = *(int *) GET_IRCHILD(  m, 0 );
		  int operand2 = *(int *) GET_IRCHILD( eq, 0 );
		  s = nCONST ( operand1 & operand2 );
		}
  else
	    {
		  s = nBAND (m, eq);
		}
  
  SET_TYPE (s, Int);	/* FIXME */
  return s;
  
}


//
// TRANSLATE BITWISE XOR
//
irnode *
translate_exclusive_or (n_exclusive_or * exclor)
{
  irnode *and = translate_and (exclor->and);
  if (!exclor->exclor) return and;
  
  irnode *s;
  irnode *m = translate_exclusive_or (exclor->exclor);
  
  if (NODETYPE(m) == n_CONST && NODETYPE(and) == n_CONST)
	    {
		  int operand1 = *(int *) GET_IRCHILD(   m, 0 );
		  int operand2 = *(int *) GET_IRCHILD( and, 0 );
		  s = nCONST ( operand1 ^ operand2 );
		}
  else
	    {
		  s = nBXOR (m, and);
		}
  
  SET_TYPE (s, Int);	/* FIXME */
  return s;

}


//
// TRANSLATE BITWISE OR
//
irnode *
translate_inclusive_or (n_inclusive_or * inclor)
{
  irnode *exclor = translate_exclusive_or (inclor->exclor);
  if (!inclor->inclor) return exclor;
  
  irnode *s;
  irnode *m = translate_inclusive_or (inclor->inclor);
  
  if (NODETYPE(m) == n_CONST && NODETYPE(exclor) == n_CONST)
	    {
		  int operand1 = *(int *) GET_IRCHILD(      m, 0 );
		  int operand2 = *(int *) GET_IRCHILD( exclor, 0 );
		  s = nCONST ( operand1 | operand2 );
		}
  else
	    {
		  s = nBOR (m, exclor);
		}
  
  SET_TYPE (s, Int);	/* FIXME */
  return s;
  
}


//
// TRANSLATE LOGICAL AND
//
irnode *
translate_logical_and (n_logical_and * land)
{
  irnode *inclor = translate_inclusive_or (land->inclor);
  if (!land->land) return inclor;
  
  irnode *s;
  
  irnode *and = translate_logical_and (land->land);
  
  if (NODETYPE(inclor) == n_CONST && NODETYPE(and) == n_CONST)
   {
     int operand1 = *(int *) GET_IRCHILD( inclor, 0 );
	 int operand2 = *(int *) GET_IRCHILD(    and, 0 );
	 s = nCONST ( operand1 && operand2 );
   }
  else
   {
    label *f = new_label();
    label *t1 = new_label();
    label *t2 = new_label();
    label *o = new_label();
    temp *result = new_temp();
    s = nESEQ( nCJUMP( nNE( and, nCONST(0)), t1, f ),
                 nESEQ( nLABEL( t1 ),
                        nESEQ( nCJUMP( nNE( inclor, nCONST(0)), t2, f ),
                               nESEQ( nLABEL( t2 ),
                                      nESEQ( nMOVE( nTEMP(result), nCONST(1)),
                                             nESEQ( nJUMP( nNAME(o), o ),
                                                    nESEQ( nLABEL( f ),
                                                           nESEQ( nMOVE( nTEMP(result), nCONST(0)),
                                                                  nESEQ( nLABEL( o ),
                                                                         nTEMP(result)))))))))); 
   }
   
  SET_TYPE (s, Int);
  SET_LVALUE (s, NOT_LVALUE);
  return s;
    
}

//
// TRANSLATE LOGICAL OR
//
irnode *
translate_logical_or (n_logical_or * lor)
{
  irnode *land = translate_logical_and (lor->land);
  if (!lor->lor) return land;
  
  irnode *s;
  irnode *or = translate_logical_or (lor->lor);
  
  if (NODETYPE(or) == n_CONST && NODETYPE(land) == n_CONST)
    {
     int operand1 = *(int *) GET_IRCHILD( or, 0 );
	 int operand2 = *(int *) GET_IRCHILD( land, 0 );
	 s = nCONST ( operand1 || operand2 );
	}
  else
	{
	  label *f = new_label();
      label *t1 = new_label();
      label *t2 = new_label();
      label *o = new_label();
      temp *result = new_temp();
      s = nESEQ( nCJUMP( nNE( or, nCONST(0)), f, t1 ),
                 nESEQ( nLABEL( t1 ),
                        nESEQ( nCJUMP( nNE( land, nCONST(0)), f, t2 ),
                               nESEQ( nLABEL( t2 ),
                                      nESEQ( nMOVE( nTEMP(result), nCONST(0)),
                                             nESEQ( nJUMP( nNAME(o), o ),
                                                    nESEQ( nLABEL( f ),
                                                           nESEQ( nMOVE( nTEMP(result), nCONST(1)),
                                                                  nESEQ( nLABEL( o ),
                                                                         nTEMP(result)))))))))); 
     }

  SET_TYPE (s, Int);
  SET_LVALUE (s, NOT_LVALUE);
  return s;
}

// forward declaration
irnode * create_struct_assign ( irnode *sequence, temp *temp_lor, temp *temp_ass, member *m, int base_offset );

irnode *
create_array_copy ( irnode *sequence, temp *temp_lor, temp *temp_ass, type *array_type, int base_offset )
{

  irnode *result = sequence;
  // determine array derived type
  type *derived_type = array_type;
  int num_elem = 1 ;
  while(is_array(derived_type)) 
    {  
      num_elem *= get_array_size(derived_type); 
      derived_type = derived_type->u.pointer_on; 
    }
  int size = get_type_size(derived_type); 
  irnode *affect = nCOPY(
                          nADD( nTEMP( temp_lor ), nCONST(base_offset) ), 
                          nADD( nTEMP( temp_ass ), nCONST(base_offset) ),
                          ((num_elem * size) % 4 == 0)?((num_elem * size) / 4):(((num_elem * size)+4) / 4) // FIXME
                        ); 
  result =  nSEQ( result, affect );
  return result;
}

 
irnode *
create_struct_assign ( irnode *sequence, temp *temp_lor, temp *temp_ass, member *m, int base_offset )
{
  irnode *result;

  if (!m) return sequence;

  int offset = m->offset;
  if ( is_array(m->t) )
        {
          // ARRAY
          return create_array_copy( sequence, temp_lor, temp_ass, m->t, base_offset  + offset);
        }
  else if ( is_struct_or_union(m->t) )
        {
          result = sequence; 
          member *im;
          FOR_EACH(im,m->t->u.members,e)
            { 
              result = nSEQ( result, create_struct_assign( nNOP(), temp_lor, temp_ass, im, 
                             base_offset + offset + im->offset ) );
            }
          return result;
        }
  else
        {
          // SCALAR
          int mode = get_type_natural_mode(m->t);
          irnode *affect = nMOVE( nMEMm( mode, nADD( nTEMP( temp_lor ), nCONST(base_offset + offset) ) ), 
                                  nMEMm( mode, nADD( nTEMP( temp_ass ), nCONST(base_offset + offset) ) )
                            );
          return nSEQ( sequence, affect );
        }

}

//
// TRANSLATE CONDITIONAL
//
irnode *
translate_conditional (n_assignment * a)
{

  assert(a->condassign);
  assert(a->expr);
  assert(a->lor);

  SET_POS(a->pos);

  /* an actual assignment is not allowed in condassign */
  if ( a->condassign->assign)
    {
      keen_error("assignment is not allowed here"); 
    }

  irnode *lor = translate_logical_or (a->lor);
  irnode *exp = translate_expression (a->expr);
  irnode *ass = translate_assignment (a->condassign);

  /* check that both parts of conditional can be converted to same type*/
  if (!is_scalar(GET_TYPE(lor))) keen_error("invalid operand in conditional");

  if (is_arithmetic(GET_TYPE(exp)) && is_arithmetic(GET_TYPE(ass))) goto operands_ok;

  if (is_struct_or_union(GET_TYPE(exp)) || is_struct_or_union(GET_TYPE(ass)) && are_compatible_type(GET_TYPE(exp),GET_TYPE(ass))) goto operands_ok;

  if (GET_TYPE(exp)->kind == t_void && GET_TYPE(ass)->kind == t_void) goto operands_ok;
 
  if (is_pointer(GET_TYPE(exp)) && is_pointer(GET_TYPE(ass)) && 
      are_compatible_type_wo_qualifiers( GET_TYPE(exp)->u.pointer_on, GET_TYPE(ass)->u.pointer_on)) goto operands_ok;

  if (is_pointer(GET_TYPE(exp)) && IS_LITERAL(ass) && is_arithmetic(GET_TYPE(ass)) && *(int *)GET_LITERAL(ass) == 0) goto operands_ok;

  if (is_pointer(GET_TYPE(ass)) && IS_LITERAL(exp) && is_arithmetic(GET_TYPE(exp)) && *(int *)GET_LITERAL(exp) == 0) goto operands_ok;

  if (is_pointer(GET_TYPE(ass)) && is_pointer(GET_TYPE(exp))
      && GET_TYPE(exp)->u.pointer_on->kind == t_void) goto operands_ok;

  if (is_pointer(GET_TYPE(exp)) && is_pointer(GET_TYPE(ass))
      && GET_TYPE(ass)->u.pointer_on->kind == t_void) goto operands_ok;

  keen_error("invalid operands in conditional operator ?:");

operands_ok: ;

  /* FIXME this type will become result type */
  type *common_type = GET_TYPE(exp);
  
  irnode *s;
  if (NODETYPE(lor) == n_CONST)
    {
	  int operand = *(int *) GET_IRCHILD(lor, 0);
	  if (operand)
	    s = exp;
	  else
	    s = ass;
	}
  else
    {
      label *f = new_label();
      label *t = new_label();
      label *o = new_label();
      temp *result = new_temp();
      s = nESEQ( nCJUMP( nEQ( lor, nCONST(0)), f, t ),
                 nESEQ( nLABEL( t ),
                        nESEQ( nMOVE( nTEMP(result), exp ),
                               nESEQ( nJUMP( nNAME(o), o ),
                                      nESEQ( nLABEL( f ),
                                             nESEQ( nMOVE( nTEMP(result), ass),
                                                    nESEQ( nLABEL( o ),
                                                           nTEMP(result)))))))); 
  
	}
	
  SET_TYPE (s, common_type );
  SET_LVALUE (s, NOT_LVALUE);

  return s;   
}

//
// TRANSLATE ASSIGNEMENT
//
irnode *
translate_assignment (n_assignment * a)
{
  temp *r;
  irnode *result;

  if (a->condassign) 
    {
	/* conditional is treated separately */
	return translate_conditional (a);
    }

  if (a->assign)
    {

      assert (a->lor);

      irnode *ass = translate_assignment (a->assign);
      irnode *lor = translate_logical_or (a->lor);

      SET_POS (a->pos);

      /* check that assignment is made on a valid l-value */
      /* The only exception is for 1st assignment. In such case even const-qualified
 	 variables can be assigned */
      if (GET_LVALUE (lor) != LVALUE)
	{
	  if (a->is_declaration)
	    {
	      // We check that the unqualified type is an lvalue, to be sure we do not
	      // authorize too much
	      type *unqualified = type_dup (GET_TYPE(lor));
	      unqualified->qualifier = undefined;
	      int i = is_modifiable_lvalue(unqualified);
	      if (i == NOT_LVALUE) keen_error ("assignment on something not an lvalue");
	      if (i == NOT_LVALUE_READ_ONLY) keen_error ("invalid assignment on read-only location");
	      // else, ok, we can authorize this assignment 
	    }
	  else if (a->compiler_internal)
	    {
	      // assignment was created by compiler during side-effects pass
	      // on ++ and -- operators
	      keen_error ("invalid lvalue in increment/decrement");
	    }
	  else
	    {
	      if (GET_LVALUE(lor) == NOT_LVALUE_READ_ONLY) keen_error ("invalid assignment on read-only location");
	      keen_error ("assignment on something not an lvalue");
	    }
	}

      /* check types & apply conversions */
      if (are_compatible_type(GET_TYPE(ass),GET_TYPE(lor))) 
        {
          if (is_struct_or_union(GET_TYPE(ass)) && a->op != opASSIGN)
            {
              keen_error("invalid assignment operator on struct/union");
            } 
          goto ok;
        }

      // both are arithmetic, convert right member to left member
      if (is_arithmetic (GET_TYPE (ass)) && is_arithmetic (GET_TYPE (lor)))
	{
	  SET_TYPE (ass, GET_TYPE (lor));
	  goto ok;
	}
      // or, one is pointer on void, and the other is pointer
      if (((is_pointer (GET_TYPE (ass)) && points_on (GET_TYPE (ass), t_void))
	   && is_pointer (GET_TYPE (lor))) ||
	  ((is_pointer (GET_TYPE (lor)) && points_on (GET_TYPE (lor), t_void))
	   && is_pointer (GET_TYPE (ass))))
	{
	  goto ok;
	}
      // or, pointer from integral or integral from pointer
      if (a->op == opASSIGN && 
          is_pointer (GET_TYPE (lor)) && is_arithmetic (GET_TYPE (ass)))
	{
	  keen_warning ("assignment makes pointer from integer without a cast");
	  goto ok;
	}
      if (a->op == opASSIGN && 
           is_pointer (GET_TYPE (ass)) && is_arithmetic (GET_TYPE (lor)))
	{
	  keen_warning ("assignment makes integer from pointer without a cast");
	  goto ok;
	}
      if (a->op == opASSIGNPLUS && is_pointer (GET_TYPE (lor))
	  && is_arithmetic (GET_TYPE (ass)))
	goto ok;
      if (a->op == opASSIGNMINUS && is_pointer (GET_TYPE (lor))
	  && is_arithmetic (GET_TYPE (ass)))
	goto ok;

      // or, pointers on different types
      if (is_pointer (GET_TYPE (ass)) && is_pointer (GET_TYPE (lor)))
	{
	  if (!are_compatible_type_wo_qualifiers
	      (GET_TYPE (ass)->u.pointer_on, GET_TYPE (lor)->u.pointer_on))
	    {
	      keen_warning ("assignment from incompatible pointer type");
	    }
	  goto ok;
	}
      keen_error ("invalid assignment");

    ok:

      if (is_struct_or_union(GET_TYPE(ass)))
        {
          // struct assignment
          // operator and type has already been checked
          temp *temp_lor = new_temp();
          temp *temp_ass = new_temp();
          result = nNOP();
          member *m;
          FOR_EACH(m,GET_TYPE(ass)->u.members,e)
            {
              result = create_struct_assign( result, temp_lor, temp_ass, m, 0 ); 
            }

          return nESEQ( nSEQ( nMOVE( nTEMP( temp_lor ), lor ),
                           nSEQ( nMOVE( nTEMP( temp_ass ), ass ),
                                 result ) ),
                        nCONST(1) ) ;
        }

      irnode *intermediate = NULL;
      temp *r1 = new_temp(), *r2 = new_temp(), *r3 = new_temp();
      switch (a->op)
	{

	case opASSIGN:
	  ; temp *r = new_temp();
	  result = nESEQ (nMOVE ( nTEMP(r), ass),
                          nESEQ ( nMOVE( lor, nTEMP(r)),
                                  nTEMP(r)));
	  SET_TYPE (result, GET_TYPE (lor));
	  SET_LVALUE (result, LVALUE);
	  SET_SYMBOL (result, GET_SYMBOL (lor));
	  return result;
	  
	case opASSIGNMINUS:
          if (NODETYPE(ass) == n_CONST)
			{
				int operand = (*(int *)GET_IRCHILD(ass, 0));
				ass = nCONST ( -1 * operand );
			}
		  else
			{
				ass = getNEG(ass);
			}
          // and goes fallthrough now...
		  
	case opASSIGNPLUS:

 	  // if arithmetic on pointer, change to type size
 	  if (is_pointer(GET_TYPE(lor)))
            {
		ass = nMUL( ass , nCONST( get_type_size( GET_TYPE(lor)->u.pointer_on )));
            }

	  intermediate = nADD( nMEM( nTEMP(r2) ), nTEMP(r1) );
	  break;
	
	case opASSIGNMUL:
 
	  intermediate = nMUL( nMEM( nTEMP(r2) ), nTEMP(r1) );
          break;
 
	case opASSIGNMOD:
 
	  intermediate = nMOD( nMEM( nTEMP(r2) ), nTEMP(r1) );
          break;
 
	case opASSIGNDIV:
 
	  intermediate = nDIV( nMEM( nTEMP(r2) ) , nTEMP(r1) );
          break;
 
	case opASSIGNBOR:
 
	  intermediate = nBOR( nMEM( nTEMP(r2) ), nTEMP(r1) );
          break;
 
	case opASSIGNBAND:
 
	  intermediate = nBAND( nMEM( nTEMP(r2) ), nTEMP(r1) );
          break;
 
	case opASSIGNBXOR:
 
	  intermediate = nBXOR( nMEM( nTEMP(r2) ), nTEMP(r1) );
          break;
 
	default:
	  assert(0);

	}

 	switch( NODETYPE( lor ) )
            {
              case n_TEMP:
                assert(0); // FIXME ? I suppose we have something to do at least in case storage class is register 
		break;

              case n_MEM:
	        result = nESEQ (nMOVE ( nTEMP(r1), ass),
                          nESEQ( nMOVE( nTEMP(r2), (irnode *) GET_IRCHILD( lor, 0) /* address */),
                           nESEQ( nMOVE( nTEMP(r3), intermediate ),
                            nESEQ ( nMOVE( nMEM( nTEMP(r2) ) , nTEMP(r3)),
                                  nTEMP(r3)))));
                break;

              default:     // we suppose that dest MEM is TEMP or MEM only
                assert(0);
            }

	SET_TYPE (result, GET_TYPE (lor));
	SET_LVALUE (result, LVALUE);
	SET_SYMBOL (result, GET_SYMBOL (lor));
	return result;

    }
  else
    {
      return translate_logical_or (a->lor);
    }
}


//
// TRANSLATE EXPRESSION
//
irnode *
translate_expression (n_expression * e)
{
  irnode *ass = translate_assignment (e->assign);
  irnode *result;
  if (e->expr)
    {
	  result = nESEQ (getStmt (translate_expression (e->expr)), ass);
	  SET_TYPE (result, GET_TYPE (ass));
	  SET_LVALUE (result, GET_LVALUE (ass));
	  SET_SYMBOL (result, GET_SYMBOL (ass));
	  return result;
    }
  else
    {
      return ass;
    }
}


//
// TRANSLATE STATEMENT
//

enum { upper_none, upper_loop, upper_switch, upper_both };

typedef struct xcase_item
{
  irnode *expr;           // case IR expression (NULL when default)
  label  *l;              // corresponding (internal) label
} case_item;

case_item *
new_case_item( irnode *expr, label *l )
{
  case_item *new = (case_item *) xmalloc( sizeof(case_item));
  new->expr = expr;
  new->l = l;
  return new;
}

typedef List caseList;

irnode *
build_recursive_switch_sequence( temp * r , ListElem case_entry , label * f, irnode * body )
{
  // end recursion
  if ( case_entry == NULL )
    {
      return nESEQ( getStmt( body ), 
                    nESEQ( nLABEL( f ) ,nCONST(0) ));
    }

  case_item *ci = object ( case_entry );
  if (ci->expr == NULL) 
    {
      // default (and final) case clause
      assert (ci->l);
      return nESEQ ( nJUMP( nNAME( ci->l ), ci->l ),
                            build_recursive_switch_sequence( r , NULL, f, body ));
    }
  else 
    {
      label * local_f = new_label();
      if ( get_next( case_entry ) != NULL ) 
        { 
          return nESEQ( nCJUMP (nEQ ( ci->expr , nTEMP (r)) , ci->l , local_f ),
                    nESEQ ( nLABEL( local_f ),
                            build_recursive_switch_sequence( r , get_next(case_entry) , f , body )));
        }
      else
        {
          return nESEQ( nCJUMP (nEQ ( ci->expr , nTEMP (r)) , ci->l , local_f ),
                    nESEQ ( nLABEL( local_f ),
                            nESEQ( nJUMP( nNAME( f ) , f ),
                                   build_recursive_switch_sequence( r , get_next(case_entry) , f , body ))));
        }
    } 
}

irnode *
translate_statement (n_statement * s, int upper_structure, label *upper_break, label *upper_continue, caseList cl )
{
  irnode *a = NULL, *b = NULL, *c = NULL;
  irnode *tree;
  irnode *stmt;
  irnode *result;
  temp *r;
  label *t, *f, *o, *p;
  labelList llist = NULL;
  int structure;
  symbol *sym;
  bind *bi;

  switch (s->kind)
    {

    case stmt_label:

      sym = lookup_symbol( env_goto_labels, s->u.label );
      if (sym)
        {
         // this goto label is known already
         if (sym->binding->u.l.is_defined) 
          {
            SET_POS( s->pos );
            keen_error("the label '%s' is already defined within that scope", s->u.label );
          }
         else
          {
            o = sym->binding->u.l.internal_label;
            sym->binding->u.l.is_defined = 1; /* it is now defined */
          }
        }
      else
        {
          // new definition
          o = new_label();
          bi = new_bind_goto( o , 1 /* is defined */ );
          bi->u.l.first_use_line = 0; /* not used yet */
          insert_symbol( env_goto_labels, s->u.label, bi );
          // insert in goto_labels list as well, to ease final checking
          add( goto_labels, s->u.label );
        }
 
      return nLABEL( o );

    case stmt_goto:

      sym = lookup_symbol( env_goto_labels, s->u.label );
      if (sym)
        {
          // this goto label has already been defined or used before
          // use the same internal label
          bi = sym->binding;
          o = bi->u.l.internal_label;
          if (bi->u.l.first_use_line==0) bi->u.l.first_use_line = s->pos.line;
          assert(o);
        }
      else
        {
          // need to declare a new label. hope it will be declared later
          // this will be checked when exiting function body 
          o = new_label();
          bi = new_bind_goto( o , 0 /* not defined so far */ );
          bi->u.l.first_use_line = s->pos.line;
          insert_symbol( env_goto_labels, s->u.label, bi );
          // insert in goto_labels list as well, to ease final checking
          add( goto_labels, s->u.label );
        }

      return nJUMP( nNAME( o ), o );

    case stmt_compound:
      return translate_compound_statement (s->u.compound,
					   /* not a function main-block */ 0,
                                           upper_structure, upper_break, upper_continue, cl );

    case stmt_expression:
      // check for empty expression
      if (s->u.expr == NULL)
	return nNOP ();

      a = translate_expression (s->u.expr);
      return a;

    case stmt_if:
      a = translate_expression (s->u.If.expr);
      // check that expression is arithmetic or pointer
      if (!is_arithmetic (GET_TYPE (a)) && !is_pointer (GET_TYPE (a)))
	{
	  SET_POS (s->u.If.expr->pos);
	  keen_error ("invalid condition in if-statement");
	}
      // construct the whole IR tree
      if (s->u.If.False == NULL)
	{
	  // IF THEN statement
	  t = new_label ();
	  f = new_label ();
	  tree =
	    nESEQ (nCJUMP (nNE (a, nCONST(0)), t, f),
		   nESEQ (nLABEL (t),
			  nESEQ (getStmt (translate_statement (s->u.If.True, upper_structure, upper_break, upper_continue, cl)),
				 nESEQ (nLABEL (f),
					nCONST(0)))));
	  return tree;
	}
      else
	{
	  // IF THEN ELSE statement
	  t = new_label ();
	  f = new_label ();
	  o = new_label ();
	  irnode *if_true = translate_statement (s->u.If.True, upper_structure, upper_break, upper_continue, cl );
	  irnode *if_false = translate_statement (s->u.If.False, upper_structure, upper_break, upper_continue, cl );
	  tree =
	    nESEQ (nCJUMP (nNE (a, nCONST(0)), t, f),
		   nESEQ (nLABEL (t),
			  nESEQ (getStmt (if_true),
				 nESEQ (nJUMP (nNAME (o), o ),
					nESEQ (nLABEL (f),
					       nESEQ (getStmt (if_false),
						      nESEQ (nLABEL (o),
							     nCONST(0))))))));
	  return tree;
	}
      break;

    case stmt_return:
      // check that we are inside some function...
      SET_POS (s->pos);
      if (!in_function)
	{
	  keen_error ("unexpected return statement");
	}

      if (s->u.ret)
	{
	  a = translate_expression (s->u.ret);
	  /* check types & apply conversions */
          if (are_compatible_type( GET_TYPE(a) , in_function->binding->u.f.return_t)) goto ok;

	  // both are arithmetic, convert right member to left member
	  if (is_arithmetic (GET_TYPE (a))
	      && is_arithmetic (in_function->binding->u.f.return_t))
	    {
	      SET_TYPE (a, in_function->binding->u.f.return_t);
	      goto ok;
	    }
	  // or, one is pointer on void, and the other is pointer
	  if (((is_pointer (GET_TYPE (a)) && points_on (GET_TYPE (a), t_void))
	       && is_pointer (in_function->binding->u.f.return_t)) ||
	      ((is_pointer (in_function->binding->u.f.return_t)
		&& points_on (in_function->binding->u.f.return_t, t_void))
	       && is_pointer (GET_TYPE (a))))
	    {
	      goto ok;
	    }
	  // or, pointer from integral 
	  if (is_pointer (in_function->binding->u.f.return_t)
	      && is_arithmetic (GET_TYPE (a)))
	    {
	      keen_warning
		("return expression makes pointer from integer without a cast");
	      goto ok;
	    }
	  if (is_arithmetic (in_function->binding->u.f.return_t)
	      && is_pointer (GET_TYPE (a)))
	    {
	      keen_warning
		("return expression makes integer from pointer without a cast");
	      goto ok;
	    }
	  // or, pointers on different types
	  if (is_pointer (GET_TYPE (a))
	      && is_pointer (in_function->binding->u.f.return_t))
	    {
	      if (!are_compatible_type
		  (GET_TYPE (a)->u.pointer_on,
		   in_function->binding->u.f.return_t->u.pointer_on))
		{
		  keen_warning
		    ("return expression makes assignment from incompatible pointer type");
		}
	      goto ok;
	    }
	  keen_error ("invalid expression in return statement");
	}

    ok:
      o = get_frame_label_epilogue (in_function->binding->u.f.f_frame);
      if (a)
	{
          if (is_struct_or_union(GET_TYPE(a)))
            {
              // create struct assignment to 1st parameter of the called function
              // (this supposes that every called function that return struct has 1st
              // parameter with address to appropriate object, to hold result)
              result = nNOP();
              temp *temp_lor = new_temp();
              temp *temp_ass = new_temp();
              member *m;
              FOR_EACH( m, GET_TYPE(a)->u.members, e )
                {
                  result = create_struct_assign( result, temp_lor, temp_ass, m, 0 ); 
                }
              // get address of 1st parameter
              ListElem first_param_e = get_first ( get_formals (in_function->binding->u.f.f_frame));
              assert(first_param_e);
              irnode *addr = getAddress( object(first_param_e) );
              tree = nSEQ ( nMOVE( nTEMP(temp_lor), nMEM( addr ) ), 
                      nSEQ ( nMOVE( nTEMP(temp_ass), a ), 
                       nSEQ ( result,
	                nSEQ ( nMOVE (nTEMP (RV ()), nTEMP(temp_lor)), nJUMP (nNAME (o), o ) ) ) ) );
            }
          else
            {
	      tree = nSEQ (nMOVE (nTEMP (RV ()), a), nJUMP (nNAME (o), o ));
            }
	}
      else
	{
	  tree = nJUMP (nNAME (o), o );
	}
      return tree;

    case stmt_for:

      if (s->u.For.start)
        {
          a = translate_expression (s->u.For.start);
          // check that expression is arithmetic or pointer
          if (!is_arithmetic (GET_TYPE (a)) && !is_pointer (GET_TYPE (a)))
	    {
	      SET_POS (s->u.For.start->pos);
	      keen_error ("invalid expression in for-statement");
	    }
        }
      else
        {
          a = nCONST( 1 );
        }

      if (s->u.For.cond)
        {
          b = translate_expression (s->u.For.cond);
          // check that expression is arithmetic or pointer
          if (!is_arithmetic (GET_TYPE (b)) && !is_pointer (GET_TYPE (b)))
	    {
	      SET_POS (s->u.For.cond->pos);
	      keen_error ("invalid expression in for-statement");
	    }
        }
      else
        {
          b = nCONST( 1 );
        }

      if (s->u.For.end)
        {
          c = translate_expression (s->u.For.end);
          // check that expression is arithmetic or pointer
          if (!is_arithmetic (GET_TYPE (c)) && !is_pointer (GET_TYPE (c)))
	    {
	      SET_POS (s->u.For.end->pos);
	      keen_error ("invalid expression in for-statement");
	    }
        }
      else
        {
          c = nCONST( 1 );
        }

      // determine new upper structure
      switch (upper_structure)
        {
          case upper_loop: 
          case upper_none: structure = upper_loop; break;
          case upper_both:
          case upper_switch: structure = upper_both; break;
          default: assert(0);
        }

      // construct the whole IR tree
      t = new_label ();
      f = new_label ();
      o = new_label ();
      p = new_label ();
      tree = nESEQ ( getStmt(a),
                     nESEQ( nLABEL (o), 
		            nESEQ (nCJUMP (nNE (b, nCONST(0)), t, f),
                                   nESEQ( nLABEL(t),
                                          nESEQ( getStmt( translate_statement (s->u.For.stmt, structure, f, p , cl ) ),
                                                 nESEQ( nLABEL(p),
                                                        nESEQ( getStmt( c ),
                                                               nESEQ( nJUMP( nNAME(o), o ),  
			   	                                      nESEQ (nLABEL (f),
						                             nCONST(0))))))))));
      return tree;

    case stmt_while:
      a = translate_expression (s->u.While.expr);
      // check that expression is arithmetic or pointer
      if (!is_arithmetic (GET_TYPE (a)) && !is_pointer (GET_TYPE (a)))
	{
	  SET_POS (s->u.While.expr->pos);
	  keen_error ("invalid condition in while-statement");
	}
      // construct the whole IR tree
      t = new_label ();
      f = new_label ();
      o = new_label ();
      tree = nESEQ (nLABEL (o),
		    nESEQ (nCJUMP
			   (nNE (a, nCONST(0)), t, f),
			   nESEQ (nLABEL (t),
				  nESEQ (getStmt
					 (translate_statement (s->u.While.stmt, /* in a new loop */ upper_loop, f, o , cl )),
					 nESEQ (nJUMP (nNAME (o), o ),
						nESEQ (nLABEL (f),
						       nCONST(0)))))));
      return tree;

    case stmt_do:
      
      // determine new upper structure
      switch (upper_structure)
        {
          case upper_loop: 
          case upper_none: structure = upper_loop; break;
          case upper_both:
          case upper_switch: structure = upper_both; break;
          default: assert(0);
        }

      // create a new exit/continue label for this loop
      f = new_label();
      t = new_label ();

      // evaluate loop statement
      b = translate_statement (s->u.Do.stmt, structure, f, t, cl );

      // evaluate cond expression
      a = translate_expression (s->u.Do.expr);
      
      // check that expression is arithmetic or pointer
      if (!is_arithmetic (GET_TYPE (a)) && !is_pointer (GET_TYPE (a)))
	{
	  SET_POS (s->u.Do.expr->pos);
	  keen_error ("invalid condition in while-statement");
	}

      // construct the whole IR tree
      tree = nESEQ (nLABEL (t),
                    nESEQ( getStmt( b ),
                           nESEQ( nCJUMP(nNE (a, nCONST(0)), t, f),
                                  nESEQ( nLABEL(f),
                                         nCONST(0) ) ) ) );

	return tree;

    case stmt_break:
      if (upper_structure == upper_none)
        {
          keen_error("break statement is not allowed here");
        }
      tree = nJUMP( nNAME(upper_break) , upper_break ) ;
      return tree;

    case stmt_continue:
      if (upper_structure != upper_loop && upper_structure != upper_both )
        {
          keen_error("continue statement is not allowed here");
        }
      tree = nJUMP( nNAME(upper_continue) , upper_continue ) ;
      return tree;

    case stmt_switch:

      a = translate_expression (s->u.Switch.expr);
      // check that expression is arithmetic or pointer
      if (!is_arithmetic (GET_TYPE (a)) && !is_pointer (GET_TYPE (a)))
	{
	  SET_POS (s->u.Switch.expr->pos);
	  keen_error ("invalid condition in switch-statement");
	}

      // in any case, put expression result in some temp
      r = new_temp ();
      tree = nESEQ (nMOVE (nTEMP (r), a), nTEMP (r));
      SET_TYPE (tree, GET_TYPE (a));
      SET_LVALUE (tree, GET_LVALUE (a));
      SET_SYMBOL (tree, GET_SYMBOL (a));
      a = tree;

      // create a false label for future inner break statements
      f = new_label();

      // create a label list to store case labels 
      caseList case_entries = newList();

      // determine new upper structure
      switch (upper_structure)
        {
          case upper_switch: 
          case upper_none: structure = upper_switch; break;
          case upper_both:
          case upper_loop: structure = upper_both; break;
          default: assert(0);
        }

      // parse switch stmt
      b = getStmt( 
           translate_statement(s->u.Switch.stmt, structure, f, upper_continue /* continue exit point is left as is */ , case_entries ) 
          );

      // Check that there is at most one default entry
      case_item *def = NULL;
      case_item *c;
      FOR_EACH( c, case_entries, e )
        {
          if (c->expr == NULL) 
            {
              if (def) keen_error("there should be only one default clause in switch statement");
              def = c;
            }
        }
      // put it last if exists
      if (def)
        {
          find_n_drop( case_entries, def );
          add( case_entries , def );
        }
       
      tree = nESEQ( getStmt( a ) , 
                    build_recursive_switch_sequence( r , get_first(case_entries) , f , b ) ) ;

      return tree;

    case stmt_case:

      if (!upper_structure == upper_switch && !upper_structure == upper_both )
        {
          keen_error("case statement not within a switch");
        }

      if (s->u.Case.expr == NULL)
        {
          // default :
          a = NULL;
        }
      else
        { 
          a = translate_expression (s->u.Case.expr);
          // check that expression is arithmetic or pointer
          if (!is_arithmetic (GET_TYPE (a)) && !is_pointer (GET_TYPE (a)) && !IS_LITERAL(a))
	    {
	      SET_POS (s->u.Case.expr->pos);
	      keen_error ("invalid expression in case statement");
	    }
        }

      // construct the whole IR tree and store label for further use
      // when we construct the actual switch construction
      o = new_label();
      case_item *ci = new_case_item( a /* is NULL if default case */ , o );
      assert( cl );
      add ( cl , ci );

      tree = nLABEL( o ) ; 
      return tree;

    }				/* end switch */
  assert (0);
}



//
// TRANSLATE COMPOUND STATEMENT
//
irnode *
translate_compound_statement (n_compound_statement * c,
			      int function_main_block,
                              int upper_structure, 
                              label *upper_break, label *upper_continue, 
                              caseList cl )
{

  // simple return condition
  if (c == NULL)
    return nNOP ();

  // if the block is not a function-block, open a new scope
  if (!function_main_block)
    begin_scope (env);

  // treat all stmts within the block
  irnode *s = nNOP ();
  n_block_item *i;
  n_block_item_list *b = c->list;
  if (b)
    {
      i = b->item;
      switch (i->kind)
	{
	case bi_decl:
	  s = translate_int_ext_declaration (int_decl, i->u.decl, function_main_block);
	  break;
	case bi_stmt:
	  s = translate_statement (i->u.stmt, upper_structure, upper_break, upper_continue, cl );
	  break;
	}
      b = b->list;
    }
  for (; b != NULL; b = b->list)
    {
      i = b->item;
      switch (i->kind)
	{
	case bi_decl:
	  s =
	    nSEQ (getStmt (s),
		  getStmt (translate_int_ext_declaration
			   (int_decl, i->u.decl, function_main_block)));
	  break;
	case bi_stmt:
	  s = nSEQ (getStmt (s), getStmt (translate_statement (i->u.stmt, upper_structure, upper_break, upper_continue, cl )));
	  break;
	}
    }

  // check for side-effect at beginning of block, if 1st instruction has such side-effects
  irnode *result;
  result = s;

  // close scope if needed
  if (!function_main_block)
    end_scope ( env );

  return result;
}



//
// TRANSLATE FUNCTION DEFINITION
// each function can create several fragments:
// - at least the one representing the body function (of type frag_func)
// - and potentially any number of frag_data or frag_var if global variables (static)
//   or static data are declared within function body 
//
void
translate_function_definition (n_function_definition * p)
{

  // 
  // parse function prototype
  //
  
  SET_POS (p->pos);

  n_declarator *d = p->decl;

  if (d == NULL) keen_error ("no declarator specified");
  if (d->decl->kind != dd_ptl) keen_error ("invalid function declarator"); 

  n_direct_declarator *dd = d->decl->u.d.dd;

  symbol *s = lookup_symbol ( env, dd->u.name );
  if (s && s->binding->kind == b_type) keen_error("symbol '%s' redeclared as a function", dd->u.name );

  // parse specifiers and get base type
  type *basetype = get_base_type (int_decl, p->spec);
  int storage = get_storage_class (p->spec);

  type *t = NULL;
  int ellipsis = 0;
  parameterList tl;

  // complete the base type if needed
  t = complete_base_type (d, basetype);

  // parse function parameters
  tl = parse_ptl (d->decl->u.d.ptl, &ellipsis, 0 /* abstract declarators are NOT allowed */);

  SET_POS (dd->pos);

  // compute binding for the function
  //frame *f = new_frame (new_named_label (dd->u.name), new_label ());
  frame *f = new_frame (new_named_label (stringf("%s%s",ASM_FUNCBASE,dd->u.name)), new_label ());
  bind *b = new_bind_func (t, tl, ellipsis, /* function is defined from now */ 1,
		   /* not implicit */ 0, f, dd->pos);

  // check for previous declarations and/or definitions
  if (s)
    {
      bind *bi = s->binding;
      // we consider a previous symbol declaration only if:
      // - in current scope
      // - in different scope, but provided it is a (undeclared) implicit function
      if (s->scope == env->current_scope)
	{
	  // previously declared as something not a function...
	  if (bi->kind != b_func)
	    {
	      keen_error
		("symbol '%s' is already declared as a different kind of symbol",
		 dd->u.name);
	    }
	  // declared as function, but already defined...
	  if (bi->u.f.is_defined)
	    {
	      keen_error ("function '%s' is already defined", dd->u.name);
	    }
	  // function is declared but not defined, check prototypes
	  if (!are_functions_prototypes_compatible (b, bi, 0))
	    {
	      keen_errorn ("conflicting types for '%s'", dd->u.name);
	      SET_POS (bi->u.f.first_decl_position);
	      keen_error ("function '%s' is declared here", dd->u.name);
	    }
	}
      if (s->scope != env->current_scope && bi->kind == b_func
	  && bi->u.f.is_implicit)
	{
	  if (!are_functions_prototypes_compatible (b, bi, 0))
	    {
	      keen_errorn ("conflicting types for '%s'", dd->u.name);
	      SET_POS (bi->u.f.first_decl_position);
	      keen_error ("function '%s' is declared here", dd->u.name);
	    }
	}
    }

  // store the function declaration (this may shadow an existing one)
  // from that point, in_function points on the function symbol in symbol table
  in_function = insert_symbol (env, dd->u.name, b);

  //
  // translate function body
  // 

  // open new scope and declare parameters within that scope
  begin_scope ( env );
  begin_scope ( env_goto_labels );
  goto_labels = newList();

  // if function return struct, add a "shadow" parameter that will hold
  // result
  if (is_struct_or_union(b->u.f.return_t))
    {
      new_param (DEFAULT_MODE, f, 4 /* FIXME should not be fixed, but mode/size of pointer to struct */);
    }

  parameter *n;
  FOR_EACH_REWIND(n,b->u.f.arg_list,e)
    {

      // normally a parameter in a function definition is a named parameter. Only exception is 
      // when the function is declared with parameter list "(void)", in such case an unnamed 
      // parameter is authorized, but we have nothing to treat here and can exit immediatly
      if (!n->name && are_compatible_type(n->t,Void)) break;

      assert(n->name);
      s = lookup_symbol (env, n->name);
      if ((s != NULL) && (s->scope == env->current_scope))
	{
	  keen_error ("symbol '%s' is already declared", n->name);
	}

      if (is_struct_or_union(n->t))
        {
          // a struct is copied, so size is not default one
          insert_symbol (env, n->name,
		     new_bind_var (n->t, /* parameter */ 1,
				   new_param (DEFAULT_MODE , f, get_type_size(n->t)), 0));
        }
      else
        {
          insert_symbol (env, n->name,
		     new_bind_var (n->t, /* parameter */ 1,
				   new_param (DEFAULT_MODE , f, 4 /* FIXME default size */), 0));
        }
    }

  // get expression body and create corresponding fragment
  irnode *body =
    translate_compound_statement (p->block, /* function main block */ 1,
                                  upper_none /* not in loop nor switch */, 
                                  NULL, NULL /* no break/continue upper structure labels */, 
                                  NULL /* no case list */);

  // ensure that body is a stmt, not an expr (this is a prerequisite to produce canonical form later )
  if (GET_CLASS (body) != n_CLASS_STMT)
    body = nEXPR (body);

  keen_debug  ("FUNCTION FRAME:\n"); 
  print_frame ( in_function->binding->u.f.f_frame );
  keen_debug  ("FUNCTION BODY:\n"); 
  if (debug_level()) print_tree  ( body , 0 );

  add (FRAGMENTS,
       new_fragment_func (nSEQ
			  (generate_prologue (in_function->binding->u.f.f_frame, body),
			   generate_epilogue (in_function->binding->u.f.f_frame)),
			  in_function->binding->u.f.f_frame));

  // close goto_labels symbol table
  // just before, check that all goto labels used within function body were actually defined
  symbol *sym;
  char *str;
  FOR_EACH(str,goto_labels,e2)
    {
      sym = lookup_symbol ( env_goto_labels, str );
      assert(sym);
      if (!sym->binding->u.l.is_defined)
        {
          SET_LINE( sym->binding->u.l.first_use_line );
          SET_COLUMN(1);
          keen_error("label '%s' was used here but not defined", str );
        }
    }
  freel (goto_labels);
  end_scope ( env_goto_labels );

  // close scope
  end_scope ( env );

  // flag we are outside the function
  in_function = NULL;

}



//
// TRANSLATE UNIT
//
void
translate_unit (n_unit * u)
{

  while (u)
    {
      n_external_declaration *e = u->ext;
      switch (e->kind)
	{
	case ext_decl_decl:
	  translate_int_ext_declaration (ext_decl, e->u.decl, 0 /* not in function main block */);
	  break;
	case ext_decl_func:
	  translate_function_definition (e->u.def);
	  break;
	default:
	  abort ();		// should not get here...
	}
      u = u->u;
    }

}

//
// TRANSLATE THE WHOLE PROGRAM
//
fragmentList
translate (n_unit * u)
{
  FRAGMENTS = newList ();
  SET_LOC (location_line);
  translate_unit (u);
  // add miscellaneous information in assembler file
  ListElem e = get_first(FRAGMENTS);
  add_before( FRAGMENTS, e , new_fragment_info(nINFO( string(GET_FILE()), string(keen_version) ))) ;
  return FRAGMENTS;
}


//
// DEBUG PRINT METHODS
//
void
indent (int n)
{
  for (; n > 0; n--)
    keen_debug (" ");
}

void
print_tree (irnode * e, int level)
{
  char name[128];
  get_node_name_and_mode (e, name);
  irnodeList l;
  ListElem z;
  if (e == NULL)
    return;
  switch (NODETYPE (e))
    {
    case n_ADD:
    case n_SUB:
    case n_MUL:
    case n_DIV:
    case n_MOD:
    case n_AND:
    case n_OR:
    case n_LSHIFT:
    case n_RSHIFT:
    case n_BOR:
    case n_BAND:
    case n_BXOR:
    case n_EQ:
    case n_NE:
    case n_GT:
    case n_LT:
    case n_GE:
    case n_LE:
      indent (level);
      keen_debug ("%s(\n", name);
      print_tree (GET_IRCHILD (e, 0), level + 6);
      print_tree (GET_IRCHILD (e, 1), level + 6);
      indent (level);
      keen_debug (")%s\n", name);
      return;
    case n_TEMP:
      indent (level);
      keen_debug ("%s( %s )\n", name,
		  get_temp_name ((temp *) GET_IRCHILD (e, 0)));
      return;
    case n_ESEQ:
      indent (level);
      keen_debug ("%s(\n", name);
      print_tree (GET_IRCHILD (e, 0), level + 5);
      print_tree (GET_IRCHILD (e, 1), level + 5);
      indent (level);
      keen_debug (")%s\n", name);
      return;
    case n_CONST:
      indent (level);
      keen_debug ("%s( %d )\n", name, *((int *) GET_IRCHILD (e, 0)));
      return;
    case n_MEM:
      indent (level);
      keen_debug ("%s(\n", name);
      print_tree (GET_IRCHILD (e, 0), level + 4);
      indent (level);
      keen_debug (")%s\n", name);
      return;
    case n_NAME:
      indent (level);
      keen_debug ("%s( %s )\n", name,
		  get_label_name ((label *) GET_IRCHILD (e, 0)));
      return;
    case n_CALL:
      indent (level);
      keen_debug ("%s(\n", name);
      print_tree (GET_IRCHILD (e, 0), level + 5);
      l = (irnodeList) GET_IRCHILD (e, 1);
      z = get_first (l);
      while(z)
	{
	  print_tree (object(z), level + 5);
          z = get_next (z);
	}
      if (NODETYPE (GET_IRCHILD (e, 2)) != n_NOP)
	{
	  indent (level);
	  keen_debug ("     Side-Effects:\n");
	  print_tree (GET_IRCHILD (e, 2), level + 5);
	  indent (level);
	  keen_debug (")%s\n", name);
	}
      return;
    case n_SEQ:
      indent (level);
      keen_debug ("%s(\n", name);
      print_tree (GET_IRCHILD (e, 0), level + 4);
      print_tree (GET_IRCHILD (e, 1), level + 4);
      indent (level);
      keen_debug (")%s\n", name);
      return;
    case n_LABEL:
      indent (level);
      keen_debug ("%s( %s )\n", name,
		  get_label_name ((label *) GET_IRCHILD (e, 0)));
      return;
    case n_NOP:
      indent (level);
      keen_debug ("%s()\n", name);
      return;
    case n_EXPR:
      indent (level);
      keen_debug ("%s(\n", name);
      print_tree (GET_IRCHILD (e, 0), level + 5);
      indent (level);
      keen_debug (")%s\n", name);
      return;
    case n_MOVE:
      indent (level);
      keen_debug ("%s(\n", name);
      print_tree (GET_IRCHILD (e, 0), level + 5);
      print_tree (GET_IRCHILD (e, 1), level + 5);
      indent (level);
      keen_debug (")%s\n", name);
      return;
    case n_CJUMP:
      indent (level);
      keen_debug ("%s(\n", name);
      print_tree (GET_IRCHILD (e, 0), level + 5);
      indent (level);
      keen_debug ("      jump to label %s if true\n",
		  get_label_name ((label *) GET_IRCHILD (e, 1)));
      indent (level);
      keen_debug ("      jump to label %s if false\n",
		  get_label_name ((label *) GET_IRCHILD (e, 2)));
      indent (level);
      keen_debug (")%s\n", name);
      return;
    case n_JUMP:
      indent (level);
      keen_debug ("%s\n", name);
      print_tree (GET_IRCHILD (e, 0), level + 5);
      indent (level);
      keen_debug (")%s\n", name);
      return;
    case n_PROLOGUE:
      indent (level);
      keen_debug ("PROLOGUE\n");
      return;
    case n_EPILOGUE:
      indent (level);
      keen_debug ("EPILOGUE\n");
      return;
    case n_GLOBAL:
      indent (level);
      keen_debug ("GLOBAL\n");
      return;
    case n_DATA:
      indent (level);
      keen_debug ("%s(%s)\n", name,
		  get_label_name ((label *) GET_IRCHILD (e, 1)));
      return;
    case n_INFO:
      return;
    case n_COPY:
      // FIXME
      return;
    case n_COMP:
      // FIXME
      return;
    }
  assert (0);
}

void
print_fragmentList (fragmentList fraglist)
{
  fragment *f;
  FOR_EACH(f,fraglist,e)
    {
      switch (f->kind)
	{
	case frag_func:
	  keen_debug ("FRAGMENT function:\n");
	  keen_debug ("Label '%s'\n", get_label_name (get_frame_label (f->u.func.f)));
	  if (debug_level())
	  {
           print_frame(f->u.func.f);
	   print_tree (f->u.func.body, 0);
	  }
	  break;
	case frag_var:
	  keen_debug ("FRAGMENT global declaration:\n");
	  keen_debug ("Label '%s'\n", get_label_name (get_access_label (f->u.var.access)));
	}
    }
}


n_assignment * 
create_assign ( void *o, position pos, enum sptype t)
{

  assert (o);

  n_keyword null_keyword = { {0, 0, NULL}, 0 };
  n_keyword keyword_opASSIGNPLUS;
  n_keyword keyword_opASSIGNMINUS;
  n_integer one;

  // create a keyword += (that will replace ++ operators) 
  // with same position as the sequence point itself
  keyword_opASSIGNPLUS.pos.line = pos.line;
  keyword_opASSIGNPLUS.pos.column = pos.column;
  keyword_opASSIGNPLUS.pos.filename = pos.filename;
  keyword_opASSIGNPLUS.v = opASSIGNPLUS;

  // create a keyword -= (that will replace -- operators) 
  // with same position as the sequence point itself
  keyword_opASSIGNMINUS.pos.line = pos.line;
  keyword_opASSIGNMINUS.pos.column = pos.column;
  keyword_opASSIGNMINUS.pos.filename = pos.filename;
  keyword_opASSIGNMINUS.v = opASSIGNMINUS;

  // create a constant "1"
  one.pos.line = pos.line;
  one.pos.column = pos.column;
  one.pos.filename = pos.filename;
  one.v = 1;

  // if "v++" is the original postfix expression, where v is an identifier, create a new expression of the form "v += 1"
  // To do so, create the assignment "1", the logical-or "v", and put on top a new assignment with operator "+="
  n_assignment *right = 
    new_assignment (NULL, null_keyword, new_logical_or (NULL, new_logical_and
    (NULL, new_inclusive_or (NULL, new_exclusive_or (NULL, new_and (NULL, new_equality
    (NULL, null_keyword, new_relational (NULL, null_keyword, new_shift 
    (NULL, null_keyword, new_additive (NULL, null_keyword, new_multiplicative 
    (NULL, null_keyword, new_cast_unary (new_unary_postfix (new_postfix_primary 
    (new_primary_constant (one)))))))))))))));

  // left part of assignment
  // the way to get the actual node depends of pre- or post- kind of stack object
  n_logical_or *left;
  if (t == POSTFIX_SIDE_EFFECT)
    {
      left = new_logical_or (NULL, new_logical_and (NULL, new_inclusive_or (NULL, new_exclusive_or
                (NULL, new_and (NULL, new_equality (NULL, null_keyword, new_relational
                (NULL, null_keyword, new_shift (NULL, null_keyword, new_additive 
                (NULL, null_keyword, new_multiplicative (NULL, null_keyword, new_cast_unary
                (new_unary_postfix (((n_postfix *) o)->u.post->postfix))))))))))));
     }
  else
     {
       left = new_logical_or (NULL, new_logical_and (NULL, new_inclusive_or (NULL, new_exclusive_or
                (NULL, new_and (NULL, new_equality (NULL, null_keyword, new_relational 
                (NULL, null_keyword, new_shift (NULL, null_keyword, new_additive 
                (NULL, null_keyword, new_multiplicative (NULL, null_keyword,
                 new_cast_unary ( ((n_unary_prefix *) o)->unary)))))))))));
     }

  // create += or -= top assignment
  n_assignment *assign;
  switch (t)
    {
	case POSTFIX_SIDE_EFFECT:
	  switch (((n_postfix *) o)->u.post->op)
	    {
	    case opPPPLUS:
	      assign = new_assignment (right, keyword_opASSIGNPLUS, left);
	      break;
	    case opPMMINUS:
	      assign = new_assignment (right, keyword_opASSIGNMINUS, left);
	      break;
	    default:
	      assert (0);
	      break;
	    }
	  break;
	case PREFIX_SIDE_EFFECT:
	  switch (((n_unary_prefix *) o)->op)
	    {
	    case opPPPLUS:
	      assign = new_assignment (right, keyword_opASSIGNPLUS, left);
	      break;
	    case opPMMINUS:
	      assign = new_assignment (right, keyword_opASSIGNMINUS, left);
	      break;
	    default:
	      assert (0);
	      break;
	    }
	  break;
	default:
	  assert (0);
  }

  // indicate that the expression was added by the compiler, for use in further passes
  assign->compiler_internal = 1;

  return assign;

}
