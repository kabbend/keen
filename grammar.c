/*
 * Keen compiler 
 *
 * grammar.c
 *
 * implement all constructors used in grammar
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
#include "xmalloc.h"
#include "grammar.h"

#define store_pos(x,y) if (y != NULL) { (x)->pos = (y)->pos; }

n_keyword
setOp (n_keyword src, int dest)
{
  n_keyword k;
  k.pos.line = src.pos.line;
  k.pos.column = src.pos.column;
  k.pos.filename = string (src.pos.filename);
  k.v = dest;
  return k;
}

n_unit *
new_unit (n_external_declaration * d, n_unit * u)
{
  n_unit *n = (n_unit *) xmalloc (sizeof (n_unit));
  store_pos (n, d);
  n->ext = d;
  n->u = u;
  return n;
}

n_external_declaration *
new_external_declaration_function (n_function_definition * f)
{
  n_external_declaration *n =
    (n_external_declaration *) xmalloc (sizeof (n_external_declaration));
  store_pos (n, f);
  n->kind = ext_decl_func;
  n->u.def = f;
  return n;
}

n_external_declaration *
new_external_declaration_declaration (n_declaration * d)
{
  n_external_declaration *n =
    (n_external_declaration *) xmalloc (sizeof (n_external_declaration));
  store_pos (n, d);
  n->kind = ext_decl_decl;
  n->u.decl = d;
  return n;
}

n_function_definition *
new_function_definition (n_declarator_specifiers * spec, n_declarator * d,
			 n_compound_statement * stmt)
{
  n_function_definition *n =
    (n_function_definition *) xmalloc (sizeof (n_function_definition));
  store_pos (n, spec);
  n->spec = spec;
  n->decl = d;
  n->block = stmt;
  return n;
}

n_compound_statement *
new_compound_statement (n_block_item_list * l)
{
  n_compound_statement *n =
    (n_compound_statement *) xmalloc (sizeof (n_compound_statement));
  store_pos (n, l);
  n->list = l;
  return n;
}

n_block_item_list *
new_block_item_list (n_block_item * i, n_block_item_list * l)
{
  n_block_item_list *n =
    (n_block_item_list *) xmalloc (sizeof (n_block_item_list));
  store_pos (n, i);
  n->item = i;
  n->list = l;
  return n;
}

n_block_item *
new_block_item_statement (n_statement * stmt)
{
  n_block_item *n = (n_block_item *) xmalloc (sizeof (n_block_item));
  store_pos (n, stmt);
  n->kind = bi_stmt;
  n->u.stmt = stmt;
  return n;
}

n_block_item *
new_block_item_declaration (n_declaration * decl)
{
  n_block_item *n = (n_block_item *) xmalloc (sizeof (n_block_item));
  store_pos (n, decl);
  n->kind = bi_decl;
  n->u.decl = decl;
  return n;
}

n_statement *
new_statement_return (n_expression * p)
{
  n_statement *n = (n_statement *) xmalloc (sizeof (n_statement));
  store_pos (n, p);
  n->u.ret = p;
  n->kind = stmt_return;
  return n;
}

n_statement *
new_statement_break ()
{
  n_statement *n = (n_statement *) xmalloc (sizeof (n_statement));
  n->kind = stmt_break;
  return n;
}

n_statement *
new_statement_continue ()
{
  n_statement *n = (n_statement *) xmalloc (sizeof (n_statement));
  n->kind = stmt_continue;
  return n;
}

n_statement *
new_statement_goto ( n_id label )
{
  n_statement *n = (n_statement *) xmalloc (sizeof (n_statement));
  n->kind = stmt_goto;
  n->pos.line = label.pos.line;
  n->pos.column = label.pos.column;
  n->pos.filename = label.pos.filename;
  n->u.label = string(label.v);
  return n;
}

n_statement *
new_statement_label ( n_id label )
{
  n_statement *n = (n_statement *) xmalloc (sizeof (n_statement));
  n->kind = stmt_label;
  n->pos.line = label.pos.line;
  n->pos.column = label.pos.column;
  n->pos.filename = label.pos.filename;
  n->u.label = string(label.v);
  return n;
}

n_statement *
new_statement_compound (n_compound_statement * p)
{
  n_statement *n = (n_statement *) xmalloc (sizeof (n_statement));
  store_pos (n, p);
  n->u.compound = p;
  n->kind = stmt_compound;
  return n;
}

n_statement *
new_statement_expression (n_expression * p)
{
  n_statement *n = (n_statement *) xmalloc (sizeof (n_statement));
  store_pos (n, p);
  n->u.expr = p;
  n->kind = stmt_expression;
  return n;
}

n_statement *
new_statement_if (n_expression * expr, n_statement * True,
		  n_statement * False)
{
  n_statement *n = (n_statement *) xmalloc (sizeof (n_statement));
  store_pos (n, expr);
  n->kind = stmt_if;
  n->u.If.expr = expr;
  n->u.If.True = True;
  n->u.If.False = False;
  return n;
}

n_statement *
new_statement_while (n_expression * expr, n_statement * stmt)
{
  n_statement *n = (n_statement *) xmalloc (sizeof (n_statement));
  store_pos (n, expr);
  n->kind = stmt_while;
  n->u.While.expr = expr;
  n->u.While.stmt = stmt;
  return n;
}

n_statement *
new_statement_switch (n_expression * expr, n_statement * stmt)
{
  n_statement *n = (n_statement *) xmalloc (sizeof (n_statement));
  store_pos (n, expr);
  n->kind = stmt_switch;
  n->u.Switch.expr = expr;
  n->u.Switch.stmt = stmt;
  return n;
}

n_statement *
new_statement_case (int kind, n_expression * expr)
{
  n_statement *n = (n_statement *) xmalloc (sizeof (n_statement));
  store_pos (n, expr);
  n->kind = stmt_case;
  n->u.Case.kind = kind;
  n->u.Case.expr = expr;
  return n;
}

n_statement *
new_statement_for (n_expression * start, n_expression * cond, n_expression * end, n_statement * stmt)
{
  n_statement *n = (n_statement *) xmalloc (sizeof (n_statement));
  store_pos (n, start);
  n->kind = stmt_for;
  n->u.For.start = start;
  n->u.For.cond = cond;
  n->u.For.end = end;
  n->u.For.stmt = stmt;
  return n;
}

n_statement *
new_statement_do (n_statement * stmt, n_expression * cond)
{
  n_statement *n = (n_statement *) xmalloc (sizeof (n_statement));
  store_pos (n, stmt);
  n->kind = stmt_do;
  n->u.Do.expr = cond;
  n->u.Do.stmt = stmt;
  return n;
}

n_declaration *
new_declaration (n_declarator_specifiers * decl,
		 n_init_declarator_list * init)
{
  n_declaration *n = (n_declaration *) xmalloc (sizeof (n_declaration));
  store_pos (n, decl);
  n->decl = decl;
  n->init = init;
  return n;
}

n_declarator_specifiers *
new_declarator_specifiers_storage (n_storage_class_specifier * storage,
				   n_declarator_specifiers * decl)
{
  n_declarator_specifiers *n =
    (n_declarator_specifiers *) xmalloc (sizeof (n_declarator_specifiers));
  store_pos (n, storage);
  n->kind = decl_spec_storage;
  n->u.storage = storage;
  n->decl = decl;
  return n;
}

n_declarator_specifiers *
new_declarator_specifiers_type (n_type_specifier * spec,
				n_declarator_specifiers * decl)
{
  n_declarator_specifiers *n =
    (n_declarator_specifiers *) xmalloc (sizeof (n_declarator_specifiers));
  store_pos (n, spec);
  n->kind = decl_spec_type;
  n->u.type_spec = spec;
  n->decl = decl;
  return n;
}

n_declarator_specifiers *
new_declarator_specifiers_qualifier (n_type_qualifier * qual,
				     n_declarator_specifiers * decl)
{
  n_declarator_specifiers *n =
    (n_declarator_specifiers *) xmalloc (sizeof (n_declarator_specifiers));
  store_pos (n, qual);
  n->kind = decl_spec_qualifier;
  n->u.qualifier = qual;
  n->decl = decl;
  return n;
}

n_init_declarator_list *
new_init_declarator_list (n_init_declarator * init,
			  n_init_declarator_list * list)
{
  n_init_declarator_list *n =
    (n_init_declarator_list *) xmalloc (sizeof (n_init_declarator_list));
  store_pos (n, init);
  n->init = init;
  n->list = list;
  return n;
}

n_init_declarator *
new_init_declarator (n_declarator * decl, n_initializer * init)
{
  n_init_declarator *n =
    (n_init_declarator *) xmalloc (sizeof (n_init_declarator));
  store_pos (n, decl);
  n->decl = decl;
  n->init = init;
  return n;
}

n_storage_class_specifier *
new_storage_class_specifier (int kind, int line, int column, char *filename)
{
  n_storage_class_specifier *n =
    (n_storage_class_specifier *)
    xmalloc (sizeof (n_storage_class_specifier));
  n->kind = kind;
  n->pos.line = line;
  n->pos.column = column;
  n->pos.filename = filename;
  return n;
}

n_type_specifier *
new_type_specifier (n_id id)
{
  n_type_specifier *n =
    (n_type_specifier *) xmalloc (sizeof (n_type_specifier));
  n->kind = type_spe;
  n->pos.line = id.pos.line;
  n->pos.column = id.pos.column;
  n->pos.filename = id.pos.filename;
  n->name = string (id.v);
  return n;
}

n_type_specifier *
new_type_specifier_su (n_struct_or_union_specifier *su)
{
  n_type_specifier *n =
    (n_type_specifier *) xmalloc (sizeof (n_type_specifier));
  store_pos(n,su);
  n->kind = type_spe_su;
  n->u.su = su;
  return n;
}

n_type_specifier *
new_type_specifier_enum (n_enumeration_specifier *es)
{
  n_type_specifier *n =
    (n_type_specifier *) xmalloc (sizeof (n_type_specifier));
  store_pos(n,es);
  n->kind = type_spe_enum;
  n->u.en = es;
  return n;
}

n_enumerator *
new_enumerator( n_id id, n_logical_or *lor )
{
  n_enumerator *n =
    (n_enumerator *) xmalloc (sizeof (n_enumerator));
  n->pos.line = id.pos.line;
  n->pos.column = id.pos.column;
  n->pos.filename = id.pos.filename;
  n->id = string(id.v);
  n->lor= lor;
  return n;
}

n_enumerator_list *
new_enumerator_list( n_enumerator_list *el, n_enumerator *en )
{
  n_enumerator_list *n =
    (n_enumerator_list *) xmalloc (sizeof (n_enumerator_list));
  store_pos(n,en);
  n->el = el;
  n->en = en;
  return n;
}

n_enumeration_specifier *
new_named_enumeration_specifier (n_id id, n_enumerator_list *enuml)
{
  n_enumeration_specifier *n =
    (n_enumeration_specifier *) xmalloc (sizeof (n_enumeration_specifier));
  n->pos.line = id.pos.line;
  n->pos.column = id.pos.column;
  n->pos.filename = id.pos.filename;
  n->id = string(id.v);
  n->el = enuml;
  return n;
}

n_enumeration_specifier *
new_anonymous_enumeration_specifier (n_enumerator_list *enuml)
{
  n_enumeration_specifier *n =
    (n_enumeration_specifier *) xmalloc (sizeof (n_enumeration_specifier));
  store_pos(n,enuml);
  n->id = NULL;
  n->el = enuml;
  return n;
}

n_type_qualifier *
new_type_qualifier (int kind, int line, int column, char *filename)
{
  n_type_qualifier *n =
    (n_type_qualifier *) xmalloc (sizeof (n_type_qualifier));
  n->kind = kind;
  n->pos.line = line;
  n->pos.column = column;
  n->pos.filename = filename;
  return n;
}

n_struct_declarator *
new_struct_declarator (n_declarator *d )
{
  n_struct_declarator *n = (n_struct_declarator *) xmalloc (sizeof (n_struct_declarator));
  store_pos (n, d);
  n->d = d;
  return n;
}

n_struct_declarator_list *
new_struct_declarator_list (n_struct_declarator_list *sdl, n_struct_declarator *sd )
{
  n_struct_declarator_list *n = (n_struct_declarator_list *) xmalloc (sizeof (n_struct_declarator_list));
  store_pos (n, sd);
  n->sdl = sdl;
  n->sd = sd;
  return n;
}

n_struct_declaration *
new_struct_declaration (n_declarator_specifiers *sds, n_struct_declarator_list *sdl ) 
{
  n_struct_declaration *n = (n_struct_declaration *) xmalloc (sizeof (n_struct_declaration));
  store_pos (n, sdl);
  n->sds = sds;
  n->sdl = sdl;
  return n;
}

n_struct_declaration_list *
new_struct_declaration_list (n_struct_declaration_list *l, n_struct_declaration *d ) 
{
  n_struct_declaration_list *n = (n_struct_declaration_list *) xmalloc (sizeof (n_struct_declaration_list));
  store_pos (n, d);
  n->sdl = l;
  n->decl = d;
  return n;
}

n_struct_or_union_specifier *
new_struct_or_union_specifier (int kind, n_id id, n_struct_declaration_list *sdl ) 
{
  n_struct_or_union_specifier *n = (n_struct_or_union_specifier *) xmalloc (sizeof (n_struct_or_union_specifier));
  n->pos.line = id.pos.line;
  n->pos.column = id.pos.column;
  n->pos.filename = id.pos.filename;
  n->kind = kind;
  n->id = id;
  n->sdl = sdl;
  n->is_anonymous = 0;
  return n;
}

n_struct_or_union_specifier *
new_struct_or_union_specifier_anonymous (int kind, n_struct_declaration_list *sdl )
{
  n_struct_or_union_specifier *n = (n_struct_or_union_specifier *) xmalloc (sizeof (n_struct_or_union_specifier));
  store_pos (n, sdl);
  n->kind = kind;
  n->sdl = sdl;
  n->is_anonymous = 1;
  return n;
}

n_declarator *
new_declarator (n_pointer * p, n_direct_declarator * direct)
{
  n_declarator *n = (n_declarator *) xmalloc (sizeof (n_declarator));
  store_pos (n, direct);
  n->p = p;
  n->decl = direct;
  return n;
}

n_abstract_declarator *
new_abstract_declarator (n_pointer * p, n_direct_abstract_declarator * direct)
{
  n_abstract_declarator *n = (n_abstract_declarator *) xmalloc (sizeof (n_abstract_declarator));
  if (p) store_pos (n,p);
  if (direct) store_pos (n, direct);
  n->p = p;
  n->d = direct;
  return n;
}

n_type_qualifier_list *
new_type_qualifier_list ( n_type_qualifier_list *tql, n_type_qualifier *tq )
{
  n_type_qualifier_list *n = (n_type_qualifier_list *) xmalloc (sizeof (n_type_qualifier_list));
  store_pos (n, tq);
  n->tql = tql;
  n->tq = tq;
  return n;
}

n_pointer *
new_pointer (n_type_qualifier_list *tql, n_pointer * p)
{
  n_pointer *n = (n_pointer *) xmalloc (sizeof (n_pointer));
  store_pos (n, p);
  n->tql = tql;
  n->p = p;
  return n;
}

n_direct_declarator *
new_direct_declarator_id (n_id id)
{
  n_direct_declarator *n =
    (n_direct_declarator *) xmalloc (sizeof (n_direct_declarator));
  n->kind = dd_id;
  n->u.name = string (id.v);
  n->pos.line = id.pos.line;
  n->pos.column = id.pos.column;
  n->pos.filename = id.pos.filename;
  return n;
}

n_direct_declarator *
new_direct_declarator_ptl (n_direct_declarator * dd,
			   n_parameter_type_list * ptl)
{
  n_direct_declarator *n =
    (n_direct_declarator *) xmalloc (sizeof (n_direct_declarator));
  store_pos (n, dd);
  n->kind = dd_ptl;
  n->u.d.dd = dd;
  n->u.d.ptl = ptl;
  return n;
}

n_direct_abstract_declarator *
new_direct_abstract_declarator_ptl (n_direct_abstract_declarator * dd,
			   n_parameter_type_list * ptl)
{
  n_direct_abstract_declarator *n =
    (n_direct_abstract_declarator *) xmalloc (sizeof (n_direct_abstract_declarator));
  if (dd) store_pos (n, dd);
  if (ptl) store_pos (n, ptl);
  n->kind = abstract_ptl;
  n->u.ptl.d = dd;
  n->u.ptl.l = ptl;
  return n;
}

n_direct_abstract_declarator *
new_direct_abstract_declarator_array (n_direct_abstract_declarator * dd,
			   n_expression * expr)
{
  n_direct_abstract_declarator *n =
    (n_direct_abstract_declarator *) xmalloc (sizeof (n_direct_abstract_declarator));
  if (dd) store_pos (n, dd);
  if (expr) store_pos (n, expr);
  n->kind = abstract_array;
  n->u.array.d = dd;
  n->u.array.expr = expr;
  return n;
}

n_direct_declarator *
new_direct_declarator_array (n_direct_declarator * dd,
			   n_expression * expr)
{
  n_direct_declarator *n =
    (n_direct_declarator *) xmalloc (sizeof (n_direct_declarator));
  store_pos (n, dd);
  n->kind = dd_array;
  n->u.a.dd = dd;
  n->u.a.expr = expr;
  return n;
}

n_parameter_type_list *
new_parameter_type_list (n_parameter_list * pl, int ellipsis)
{
  n_parameter_type_list *n =
    (n_parameter_type_list *) xmalloc (sizeof (n_parameter_type_list));
  store_pos (n, pl);
  n->ellipsis = ellipsis;
  n->pl = pl;
  return n;
}

n_parameter_list *
new_parameter_list (n_parameter_declaration * pd, n_parameter_list * pl)
{
  n_parameter_list *n =
    (n_parameter_list *) xmalloc (sizeof (n_parameter_list));
  store_pos (n, pd);
  n->pd = pd;
  n->pl = pl;
  return n;
}

n_parameter_declaration *
new_parameter_declaration (n_declarator_specifiers * ds, n_declarator * decl)
{
  n_parameter_declaration *n = (n_parameter_declaration *) xmalloc (sizeof (n_parameter_declaration));
  store_pos (n, decl);
  n->kind = pdecl;
  n->ds = ds;
  n->decl = decl;
  n->absdecl = NULL;
  return n;
}

n_parameter_declaration *
new_parameter_declaration_abstract (n_declarator_specifiers * ds, n_abstract_declarator * decl)
{
  n_parameter_declaration *n = (n_parameter_declaration *) xmalloc (sizeof (n_parameter_declaration));
  if (decl) store_pos (n, decl);
  if (ds) store_pos (n, ds);
  n->kind = pdecl_abstract;
  n->ds = ds;
  n->decl = NULL;
  n->absdecl = decl;
  return n;
}

n_initializer *
new_initializer (n_assignment * assign)
{
  n_initializer *n = (n_initializer *) xmalloc (sizeof (n_initializer));
  store_pos (n, assign);
  n->kind = init_simple;
  n->assign = assign;
  return n;
}

n_initializer *
new_initializer_bracket (n_initializer_list * init_list)
{
  n_initializer *n = (n_initializer *) xmalloc (sizeof (n_initializer));
  store_pos (n, init_list);
  n->kind = init_bracket;
  n->init_list = init_list;
  return n;
}

n_initializer_list *
new_initializer_list (n_initializer_list * init_list, n_initializer * init)
{
  n_initializer_list *n = (n_initializer_list *) xmalloc (sizeof (n_initializer_list));
  store_pos (n, init);
  n->init_list = init_list;
  n->init = init;
  return n;
}

n_expression *
new_expression (n_expression * expr, n_assignment * assign)
{
  n_expression *n = (n_expression *) xmalloc (sizeof (n_expression));
  n->expr = expr;
  n->assign = assign;
  if (expr)
    {
      store_pos (n, expr);
    }
  else
    {
      store_pos (n, assign);
    }
  return n;
}

n_assignment *
new_assignment_conditional (n_logical_or * lor, n_expression *expr, n_assignment *ass)
{
  n_assignment *n = (n_assignment *) xmalloc (sizeof (n_assignment));
  store_pos(n,lor);
  n->lor = lor;
  n->condassign = ass;
  n->expr = expr;
  n->assign = NULL;
  return n;
}

n_assignment *
new_assignment (n_assignment * assign, n_keyword op, n_logical_or * lor)
{
  n_assignment *n = (n_assignment *) xmalloc (sizeof (n_assignment));
  if (assign)
    {
      store_pos (n, assign);
    }
  else
    {
      store_pos (n, lor);
    }
  n->assign = assign;
  n->op = op.v;
  n->lor = lor;
  n->expr = NULL;
  n->condassign = NULL;
  n->compiler_internal = 0;	/* a priori */
  n->is_declaration = 0;	/* a priori */
  return n;
}

n_logical_or *
new_logical_or (n_logical_or * lor, n_logical_and * land)
{
  n_logical_or *n = (n_logical_or *) xmalloc (sizeof (n_logical_or));
  if (lor)
    {
      store_pos (n, lor);
    }
  else
    {
      store_pos (n, land);
    }
  n->lor = lor;
  n->land = land;
  return n;
}

n_logical_and *
new_logical_and (n_logical_and * land, n_inclusive_or * inclor)
{
  n_logical_and *n = (n_logical_and *) xmalloc (sizeof (n_logical_and));
  if (land)
    {
      store_pos (n, land);
    }
  else
    {
      store_pos (n, inclor);
    }
  n->land = land;
  n->inclor = inclor;
  return n;
}

n_inclusive_or *
new_inclusive_or (n_inclusive_or * inclor, n_exclusive_or * exclor)
{
  n_inclusive_or *n = (n_inclusive_or *) xmalloc (sizeof (n_inclusive_or));
  if (inclor)
    {
      store_pos (n, inclor);
    }
  else
    {
      store_pos (n, exclor);
    }
  n->inclor = inclor;
  n->exclor = exclor;
  return n;
}

n_exclusive_or *
new_exclusive_or (n_exclusive_or * exclor, n_and * and)
{
  n_exclusive_or *n = (n_exclusive_or *) xmalloc (sizeof (n_exclusive_or));
  if (exclor)
    {
      store_pos (n, exclor);
    }
  else
    {
      store_pos (n, and);
    }
  n->exclor = exclor;
  n->and = and;
  return n;
}

n_and *
new_and (n_and * and, n_equality * eq)
{
  n_and *n = (n_and *) xmalloc (sizeof (n_and));
  if (and)
    {
      store_pos (n, and);
    }
  else
    {
      store_pos (n, eq);
    }
  n->and = and;
  n->eq = eq;
  return n;
}

n_equality *
new_equality (n_equality * eq, n_keyword op, n_relational * rel)
{
  n_equality *n = (n_equality *) xmalloc (sizeof (n_equality));
  if (eq)
    {
      store_pos (n, eq);
    }
  else
    {
      store_pos (n, rel);
    }
  n->eq = eq;
  n->rel = rel;
  n->op = op.v;
  return n;
}

n_relational *
new_relational (n_relational * rel, n_keyword op, n_shift * shift)
{
  n_relational *n = (n_relational *) xmalloc (sizeof (n_relational));
  if (rel)
    {
      store_pos (n, rel);
    }
  else
    {
      store_pos (n, shift);
    }
  n->rel = rel;
  n->op = op.v;
  n->shift = shift;
  return n;
}

n_shift *
new_shift (n_shift * shift, n_keyword op, n_additive * add)
{
  n_shift *n = (n_shift *) xmalloc (sizeof (n_shift));
  if (shift)
    {
      store_pos (n, shift);
    }
  else
    {
      store_pos (n, add);
    }
  n->shift = shift;
  n->op = op.v;
  n->add = add;
  return n;
}

n_additive *
new_additive (n_additive * add, n_keyword op, n_multiplicative * mul)
{
  n_additive *n = (n_additive *) xmalloc (sizeof (n_additive));
  if (add)
    {
      store_pos (n, add);
    }
  else
    {
      store_pos (n, mul);
    }
  n->add = add;
  n->op = op.v;
  n->mul = mul;
  return n;
}

n_multiplicative *
new_multiplicative (n_multiplicative * mul, n_keyword op, n_cast * cast)
{
  n_multiplicative *n =
    (n_multiplicative *) xmalloc (sizeof (n_multiplicative));
  if (mul)
    {
      store_pos (n, mul);
    }
  else
    {
      store_pos (n, cast);
    }
  n->mul = mul;
  n->op = op.v;
  n->cast = cast;
  return n;
}

n_cast *
new_cast_unary (n_unary * unary)
{
  n_cast *n = (n_cast *) xmalloc (sizeof (n_cast));
  store_pos (n, unary);
  n_cast_unary *n_u = (n_cast_unary *) xmalloc (sizeof (n_cast_unary));
  n_u->unary = unary;
  store_pos (n_u, unary);
  n->kind = cast_unary;
  n->u.unary = n_u;
  return n;
}

n_cast *
new_cast_cast (n_type_name *tn, n_cast * cast)
{
  n_cast *n = (n_cast *) xmalloc (sizeof (n_cast));
  store_pos (n, cast);
  n_cast_cast *n_c = (n_cast_cast *) xmalloc (sizeof (n_cast_cast));
  n_c->cast = cast;
  store_pos (n_c, cast);
  n_c->tname = tn;
  n->kind = cast_cast;
  n->u.cast = n_c;
  return n;
}

n_type_name *
new_type_name (n_type_name_specifiers_list *l, n_abstract_declarator *abs)
{
  n_type_name *n = (n_type_name *) xmalloc (sizeof (n_type_name));
  store_pos (n, l);
  n->l = l;
  n->absdecl = abs;
  return n;
}

n_type_name_specifiers_list *
new_type_name_specifiers_list (n_declarator_specifiers *s, n_type_name_specifiers_list *l)
{
  n_type_name_specifiers_list *n = (n_type_name_specifiers_list *) xmalloc (sizeof (n_type_name_specifiers_list));
  if (l) store_pos (n, l);
  if (s) store_pos (n, s);
  n->l = l;
  n->s = s;
  return n;
}

n_unary *
new_unary_sizeof_type (n_type_name * tn)
{
  n_unary *n = (n_unary *) xmalloc (sizeof (n_unary));
  store_pos (n, tn);
  n_unary_sizeof *n_p = (n_unary_sizeof *) xmalloc (sizeof (n_unary_sizeof));
  store_pos (n_p, tn);
  n_p->u.tn = tn;
  n->kind = unary_sizeof;
  n_p->kind = sizeof_typename;
  n->u.sof = n_p;
  return n;
}

n_unary *
new_unary_sizeof_unary (n_unary * unary)
{
  n_unary *n = (n_unary *) xmalloc (sizeof (n_unary));
  store_pos (n, unary);
  n_unary_sizeof *n_p = (n_unary_sizeof *) xmalloc (sizeof (n_unary_sizeof));
  store_pos (n_p, unary);
  n_p->u.un = unary;
  n->kind = unary_sizeof;
  n_p->kind = sizeof_expr;
  n->u.sof = n_p;
  return n;
}

n_unary *
new_unary_postfix (n_postfix * postfix)
{
  n_unary *n = (n_unary *) xmalloc (sizeof (n_unary));
  store_pos (n, postfix);
  n_unary_postfix *n_p =
    (n_unary_postfix *) xmalloc (sizeof (n_unary_postfix));
  store_pos (n_p, postfix);
  n_p->postfix = postfix;
  n->kind = unary_postfix;
  n->u.postfix = n_p;
  return n;
}

n_unary *
new_unary_prefix (n_keyword op, n_unary * unary)
{
  n_unary *n = (n_unary *) xmalloc (sizeof (n_unary));
  store_pos (n, unary);
  n_unary_prefix *n_p = (n_unary_prefix *) xmalloc (sizeof (n_unary_prefix));
  store_pos (n_p, unary);
  n_p->op = op.v;
  n_p->unary = unary;
  n->kind = unary_prefix;
  n->u.prefix = n_p;
  return n;
}

n_unary *
new_unary_cast (n_keyword op, n_cast * cast)
{
  n_unary *n = (n_unary *) xmalloc (sizeof (n_unary));
  store_pos (n, cast);
  n_unary_cast *n_c = (n_unary_cast *) xmalloc (sizeof (n_unary_cast));
  store_pos (n_c, cast);
  n_c->op = op.v;
  n_c->cast = cast;
  n->kind = unary_cast;
  n->u.cast = n_c;
  return n;
}

n_postfix *
new_postfix_primary (n_primary * pri)
{
  n_postfix *n = (n_postfix *) xmalloc (sizeof (n_postfix));
  store_pos (n, pri);
  n_postfix_primary *n_p =
    (n_postfix_primary *) xmalloc (sizeof (n_postfix_primary));
  store_pos (n_p, pri);
  n_p->pri = pri;
  n->kind = postfix_primary;
  n->u.pri = n_p;
  return n;
}

n_postfix *
new_postfix_pointer (n_postfix * postfix, n_id id)
{
  n_postfix *n = (n_postfix *) xmalloc (sizeof (n_postfix));
  store_pos (n, postfix);
  n_postfix_pointer *n_p =
    (n_postfix_pointer *) xmalloc (sizeof (n_postfix_pointer));
  store_pos (n_p, postfix);
  n_p->id = id.v;
  n_p->postfix = postfix;
  n->kind = postfix_pointer;
  n->u.pointer = n_p;
  return n;
}

n_postfix *
new_postfix_dot (n_postfix * postfix, n_id id)
{
  n_postfix *n = (n_postfix *) xmalloc (sizeof (n_postfix));
  store_pos (n, postfix);
  n_postfix_dot *n_d = (n_postfix_dot *) xmalloc (sizeof (n_postfix_dot));
  store_pos (n_d, postfix);
  n_d->id = id.v;
  n_d->postfix = postfix;
  n->kind = postfix_dot;
  n->u.dot = n_d;
  return n;
}

n_postfix *
new_postfix_post (n_postfix * postfix, n_keyword op)
{
  n_postfix *n = (n_postfix *) xmalloc (sizeof (n_postfix));
  store_pos (n, postfix);
  n_postfix_post *n_p = (n_postfix_post *) xmalloc (sizeof (n_postfix_post));
  store_pos (n_p, postfix);
  n_p->op = op.v;
  n_p->postfix = postfix;
  n->kind = postfix_post;
  n->u.post = n_p;
  return n;
}

n_postfix *
new_postfix_array (n_postfix * postfix, n_expression * expr)
{
  n_postfix *n = (n_postfix *) xmalloc (sizeof (n_postfix));
  store_pos (n, postfix);
  n_postfix_array *n_e =
    (n_postfix_array *) xmalloc (sizeof (n_postfix_array));
  store_pos (n_e, postfix);
  n_e->expr = expr;
  n_e->postfix = postfix;
  n->kind = postfix_array;
  n->u.array = n_e;
  return n;
}

n_postfix *
new_postfix_call (n_postfix * designator, n_argument_expr_list * list)
{
  n_postfix *n = (n_postfix *) xmalloc (sizeof (n_postfix));
  store_pos (n, designator);
  n_postfix_call *n_c = (n_postfix_call *) xmalloc (sizeof (n_postfix_call));
  if (list) store_pos (n_c, list)
    else store_pos (n_c, designator);
  n_c->list = list;
  n_c->designator = designator;
  n->kind = postfix_call;
  n->u.call = n_c;
  return n;
}

n_primary *
new_primary_constant (n_integer constant)
{
  n_primary *n = (n_primary *) xmalloc (sizeof (n_primary));
  n->pos.line = constant.pos.line;
  n->pos.column = constant.pos.column;
  n->pos.filename = constant.pos.filename;
  n->kind = primary_constant;
  n->u.constant = constant.v;
  n->label_name = 0;
  return n;
}

n_primary *
new_primary_constant_string (n_string constant)
{
  n_primary *n = (n_primary *) xmalloc (sizeof (n_primary));
  n->pos.line = constant.pos.line;
  n->pos.column = constant.pos.column;
  n->pos.filename = constant.pos.filename;
  n->kind = primary_constant_string;
  n->u.constant_string = constant.v;
  n->label_name = 0;
  return n;
}

n_primary *
new_primary_constant_string_from_list (n_primary *l, n_string constant)
{
  l->kind = primary_constant_string;
  l->u.constant_string = stringf ("%s%s", l->u.constant_string, constant.v );
  l->label_name = 0;
  return l;
}

n_primary *
new_primary_constant_char (n_char constant)
{
  n_primary *n = (n_primary *) xmalloc (sizeof (n_primary));
  n->pos.line = constant.pos.line;
  n->pos.column = constant.pos.column;
  n->pos.filename = constant.pos.filename;
  n->kind = primary_constant_char;
  n->u.constant_char = constant.v;
  n->label_name = 0;
  return n;
}

n_primary *
new_primary_identifier (n_id id)
{
  n_primary *n = (n_primary *) xmalloc (sizeof (n_primary));
  n->pos.line = id.pos.line;
  n->pos.column = id.pos.column;
  n->pos.filename = id.pos.filename;
  n->kind = primary_identifier;
  n->u.id = (char *) xmalloc (strlen (id.v) + 1);
  strcpy (n->u.id, id.v);
  return n;
}

n_primary *
new_primary_expr (n_expression * expr)
{
  n_primary *n = (n_primary *) xmalloc (sizeof (n_primary));
  store_pos (n, expr);
  n->kind = primary_expr;
  n->u.expr = expr;
  return n;
}

n_argument_expr_list *
new_argument_expr_list (n_argument_expr_list * list, n_assignment * assign)
{
  n_argument_expr_list *n =
    (n_argument_expr_list *) xmalloc (sizeof (n_argument_expr_list));
  if (list)
    {
      store_pos (n, list);
    }
  else
    {
      store_pos (n, assign);
    }
  n->list = list;
  n->assign = assign;
  return n;
}
