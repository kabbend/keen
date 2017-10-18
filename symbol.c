/*
 * Keen compiler 
 *
 * symbol.c
 *
 * implement symbol table routines
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
#include "symbol.h"
#include "bind.h"
#include "type.h"

/* Symbol tables

   Symbol tables are hashtables where symbol name is the key and that store a pointer to
   a binding (bind *) which stores all necessary information for that symbol. 

   Symbol tables also manage 
   - scopes, ie. all symbols are associated to a particular scope, and all symbols in a given
   scope can be removed at the same time (for instance, when the compiler has finished to translate
   a function and can release all variables local to that function-scope). The routines to
   manipulate scopes are begin_scope() and end_scope(). It is also possible to retrieve the
   particular scope of a symbol by using the field symbol->scope.
   - shadowing, ie. the same symbol name can be inserted several times (possibly with different
   meanings and bindings). Only the last one is returned by a request on the table, the others
   being shadowed.
 
   There are 2 separate symbol tables: env for functions, struct, unions, enumerations and variables
   in general, and env_goto for goto labels.

   Within env symbol-table, struct, unions and enumerations do not share same namespace simply by
   adding an heading 'struct', 'union' or 'enum' string before actual name. This prevents that 
   variable 'x' is confused with 'struct x'. This of course requires that any request within symbol
   table adds the appropriate keyword prior to the call.

   The separation between env and env_goto_labels is required because labels are visible outside
   classical scope boundaries, as in:

      int f() 
        {
          goto inside;
          {
            // this is a new scope
            inside: ...
          } 
        }
     
.  If they were managed in same table, a call to close_scope() at the end of inner compound statement
   would remove wrongly the declaration for 'inside' label. Hence goto are managed in separate table
   with separate scope (at function level, to be precise). */

#define HASHSIZE 		109
#define SPECIAL_SCOPE_SYMBOL 	"*begin*scope*"

hashtable *env = NULL;
hashtable *env_goto_labels = NULL;

void
init_env (void)
{

  if (env) free_hashtable (env);
  if (env_goto_labels) free_hashtable (env_goto_labels);

  env = create_hashtable ();
  env_goto_labels = create_hashtable ();

  /* base types */

  /* 'signed' and 'unsigned' tokens are considered as types here only for parsing/grammar 
   * reasons, and are not used as 'true' types in the rest of the compiler (they are replaced
   * by their corresponding 'int' with appropriate sign attributes ). 
   * The reason is that the grammar must distinguish between TYPENAME and IDENTIFIER
   * to avoid ambiguities, and to do so all keywords related to type specification must be
   * considered as TYPENAME, not IDENTIFIERs. We could of course use another token as 
   * KEYWORD for this 2 particular specifiers, but this would make the grammar more complex 
   */

  insert_symbol (env, "void", new_bind_type ( Void ));
  insert_symbol (env, "char", new_bind_type ( Char ));
  insert_symbol (env, "int", new_bind_type ( Int ));
  insert_symbol (env, "short", new_bind_type ( Short ));
  insert_symbol (env, "long", new_bind_type ( Long ));
  insert_symbol (env, "signed", new_bind_type ( Int ));
  insert_symbol (env, "unsigned", new_bind_type ( Uint ));

}

void
open_scope()
{
 begin_scope(env);
}

void
close_scope()
{
 end_scope(env);
}

/*
 * wrapper function to create a new symbol entry 
 */
symbol *
new_symbol (char *name, bind * binding, int scope, symbol * next)
{
  symbol *s = (symbol *) xmalloc (sizeof (symbol));
  s->name = (char *) xmalloc (strlen (name) + 1);
  strcpy (s->name, name);
  s->binding = binding;
  s->scope = scope;
  s->next = next;
  return s;
}

/*
 * wrapper function to create a new undo 
 */
undo *
new_undo (symbol * s, undo * next)
{
  undo *u = (undo *) xmalloc (sizeof (undo));
  u->s = s;
  u->next = next;
  return u;
}

/*
 * basic hash function
 */
unsigned int
hash (char *s)
{
  unsigned int h = 0;
  char *p;
  for (p = s; *p; p++)
    h = (h * 65599) + (*p);
  return h;
}

/* 
 * creates an empty symbol table  
 */
hashtable *
create_hashtable (void)
{
  hashtable *t = (hashtable *) xmalloc (sizeof (hashtable));
  t->table = (symbol **) xcalloc (HASHSIZE, sizeof (symbol *));
  t->undo_list = NULL;
  t->current_scope = 0;
  return t;
}

/*
 * free the given hashtable
 */
void
free_hashtable (hashtable * h)
{
  if (h == NULL)
    return;
  if (h->table)
    free (h->table);
  free (h);
}

/*
 * insert a new symbol in the given symbol table. Previous identical symbols, if any, 
 * are shadowed but not replaced. The newly inserted symbol is automatically placed in the
 * undo_list, to allow future undo (by a call to undo_last() or end_scope() function) 
 */
symbol *
insert_symbol (hashtable * t, char *name, bind * binding)
{
  int index = hash (name) % HASHSIZE;
  symbol *s = new_symbol (name, binding, t->current_scope, t->table[index]);
  t->table[index] = s;
  t->undo_list = new_undo (s, t->undo_list);
  return s;
}

/*
 * insert a new symbol in the given symbol table, at the uppermost scope. Previous identical symbols, 
 * if any, are shadowed but not replaced. The newly inserted symbol is _not_ undo-able 
 */
symbol *
insert_symbol_uppermost_scope (hashtable * t, char *name, bind * binding)
{
  int index = hash (name) % HASHSIZE;
  symbol *s = new_symbol (name, binding, t->current_scope, t->table[index]);
  t->table[index] = s;
  return s;
}

/*
 * returns the binding (void *) associated to the given symbol name, or NULL is no such symbol
 * exists in the symbol table
 */
symbol *
lookup_symbol (hashtable * t, char *name)
{
  symbol *s;
  int index = hash (name) % HASHSIZE;
  for (s = t->table[index]; s != NULL; s = s->next)
    {
      if (0 == strcmp (name, s->name))
	return s;
    }
  return NULL;
}

/*
 * remove the last insertion in hashtable t, and return this last symbol. 
 * If a symbol with same name was shadowed, it becomes visible now. 
 */
symbol *
undo_last (hashtable * t)
{

  symbol *s;

  // simple exit condition
  undo *u = t->undo_list;
  if (u == NULL)
    return NULL;

  // remove undo object from undo_list
  t->undo_list = u->next;

  // remove the symbol in the hashtable itself (this symbol is mandatorily in first position
  // in the chained list)
  s = u->s;
  int index = hash (s->name) % HASHSIZE;
  t->table[index] = s->next;

  // free useless undo object
  free (u);

  // return symbol
  return s;

}

/*
 * put a special symbol in hashtable (and undo_list) in order to mark the beginning
 * of a scope. This special marker will be interpreted by end_scope() function to
 * remove an entire scope in one time
 */
void
begin_scope (hashtable * t)
{
  insert_symbol (t, SPECIAL_SCOPE_SYMBOL, NULL);
  t->current_scope++;
}

/*
 * remove all insertions that were done since the last begin_scope() call. This allows to
 * restore a previous scope in one operation
 */
void
end_scope (hashtable * t)
{
  symbol *s;
  while (((s = undo_last (t)) != NULL)
	 && (strcmp (SPECIAL_SCOPE_SYMBOL, s->name) != 0));
  if (t->current_scope > 0)
    t->current_scope--;
}
