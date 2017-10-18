/*
 * Keen compiler 
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111_1307  
 * USA
 *
 */

#ifndef _GRAMMAR_H
#define _GRAMMAR_H

#define undefined -1

/* filled in by LEXER */
typedef struct xposition
{
  int line, column;
  char *filename;
} position;

typedef struct xkeyword
{
  position pos;
  int v;
} n_keyword;

typedef struct xid
{
  position pos;
  char *v;
} n_id;

typedef struct xinteger
{
  position pos;
  int v;
} n_integer;

typedef struct xchar
{
  position pos;
  char v;
} n_char;

typedef struct xstring
{
  position pos;
  char *v;
} n_string;

n_keyword setOp (n_keyword src, int dest);

typedef struct sequence_point_
{
  position pos;
  struct expression_ *side_effects;
} n_sequence_point;

/* filled in by PARSER */

typedef struct unit_
{
  position pos;
  struct external_declaration_ *ext;
  /* translation unit */
  struct unit_ *u;
} n_unit;

typedef struct external_declaration_
{
  position pos;
  enum
  { ext_decl_func, ext_decl_decl } kind;
  union
  {
    struct function_definition_ *def;
    struct declaration_ *decl;
  } u;
} n_external_declaration;

typedef struct function_definition_
{
  position pos;
  struct declarator_specifiers_ *spec;
  struct declarator_ *decl;
  struct compound_statement_ *block;
} n_function_definition;

typedef struct compound_statement_
{
  position pos;
  /* very first sequence point of the stmt list, 
   * to hold eventual side-effects prefix of 1st insn */
  struct block_item_list_ *list;
} n_compound_statement;

typedef struct block_item_list_
{
  position pos;
  struct block_item_ *item;
  struct block_item_list_ *list;
} n_block_item_list;

typedef struct block_item_
{
  position pos;
  enum
  { bi_decl, bi_stmt } kind;
  union
  {
    struct declaration_ *decl;
    struct statement_ *stmt;
  } u;
} n_block_item;

typedef struct statement_
{
  position pos;
  enum
  { stmt_compound, stmt_expression, stmt_if, stmt_return, stmt_while,
      stmt_for, stmt_break, stmt_continue, stmt_case, stmt_switch,
      stmt_do , stmt_goto, stmt_label } kind;
  union
  {
    struct compound_statement_ *compound;
    struct expression_ *expr;
    char *label;
    struct
    {
      struct expression_ *expr;
      struct statement_ *True;
      struct statement_ *False;
    } If;
    struct
    {
      struct expression_ *expr;
      struct statement_ *stmt;
    } While;
    struct
    {
      struct expression_ *expr;
      struct statement_ *stmt;
    } Switch;
    struct
    {
      enum { ci_case, ci_default } kind;
      struct expression_ *expr;
    } Case;
    struct 
    {
      struct expression_ *start;
      struct expression_ *cond;
      struct expression_ *end;
      struct statement_ *stmt;
    } For;
    struct
    {
      struct statement_ *stmt;
      struct expression_ *expr;
    } Do;
    struct expression_ *ret;
    /* break and continue do not require storage */
  } u;
  /* sequence point at the end of stmt_expression or in stmt_return before actual jump */
} n_statement;

typedef struct declaration_
{
  position pos;
  struct declarator_specifiers_ *decl;
  struct init_declarator_list_ *init;
} n_declaration;

typedef struct declarator_specifiers_
{
  position pos;
  enum
  { decl_spec_storage, decl_spec_type, decl_spec_qualifier } kind;
  union
  {
    struct storage_class_specifier_ *storage;
    struct type_specifier_ *type_spec;
    struct type_qualifier_ *qualifier;
  } u;
  struct declarator_specifiers_ *decl;
} n_declarator_specifiers;

typedef struct init_declarator_list_
{
  position pos;
  struct init_declarator_ *init;
  struct init_declarator_list_ *list;
} n_init_declarator_list;

typedef struct init_declarator_
{
  position pos;
  struct declarator_ *decl;
  struct initializer_ *init;
} n_init_declarator;

typedef struct storage_class_specifier_
{
  position pos;
  enum
  { storage_extern, storage_static, storage_auto, storage_register,
      storage_typedef } kind;
} n_storage_class_specifier;

typedef struct struct_declarator_ {
  position pos;
  struct declarator_ *d;
} n_struct_declarator;

typedef struct struct_declarator_list_ {
  position pos;
  struct struct_declarator_list_ *sdl;
  struct struct_declarator_ *sd;
} n_struct_declarator_list;

/*
typedef struct struct_declaration_specifiers_ {
  position pos;
  struct declarator_specifiers_ *ds;
} n_struct_declaration_specifiers;
*/

typedef struct struct_declaration_ {
  position pos;
  struct declarator_specifiers_ *sds;
  struct struct_declarator_list_ *sdl;
} n_struct_declaration;

typedef struct struct_declaration_list_ {
  position pos;
  struct struct_declaration_list_ *sdl;
  struct struct_declaration_ *decl;
} n_struct_declaration_list;

typedef struct struct_or_union_specifier_ {
  position pos;
  enum { su_struct, su_union } kind;
  int is_anonymous;
  n_id id; 
  struct struct_declaration_list_ *sdl; 
} n_struct_or_union_specifier;

typedef struct enumerator_ {
  position pos;
  char *id;
  struct logical_or_ *lor;
} n_enumerator;

typedef struct enumerator_list_ {
  position pos;
  struct enumerator_list_ *el;
  struct enumerator_ *en;
} n_enumerator_list;

typedef struct enumeration_specifier_ {
  position pos;
  char * id;
  struct enumerator_list_ *el;
} n_enumeration_specifier;

typedef struct type_specifier_
{
  position pos;
  enum { type_spe, type_spe_su, type_spe_enum } kind;
  char *name;
  union {
    struct struct_or_union_specifier_ *su;
    struct enumeration_specifier_ *en;
  } u;
} n_type_specifier;

typedef struct type_qualifier_
{
  position pos;
  enum { type_qual_const, type_qual_volatile, type_qual_restrict } kind;
} n_type_qualifier;

typedef struct type_qualifier_list_
{
  position pos;
  struct type_qualifier_list_ *tql;
  struct type_qualifier_ *tq;
} n_type_qualifier_list;

typedef struct declarator_
{
  position pos;
  struct pointer_ *p;
  struct direct_declarator_ *decl;
} n_declarator;

typedef struct pointer_
{
  position pos;
  struct type_qualifier_list_ *tql;
  struct pointer_ *p;
} n_pointer;

typedef struct direct_declarator_
{
  position pos;
  enum
  { dd_id, dd_ptl, dd_array } kind;
  union
  {
    char *name; // in case identifier
    struct 
    {
      // array
      struct direct_declarator_ *dd;
      struct expression_ *expr;
    } a;
    struct
    {
      // function (with parameter list)
      struct direct_declarator_ *dd;
      struct parameter_type_list_ *ptl;
    } d;
  } u;
} n_direct_declarator;

typedef struct parameter_type_list_
{
  position pos;
  struct parameter_list_ *pl;
  int ellipsis;
} n_parameter_type_list;

typedef struct parameter_list_
{
  position pos;
  struct parameter_declaration_ *pd;
  struct parameter_list_ *pl;
} n_parameter_list;

typedef struct parameter_declaration_
{
  position pos;
  enum { pdecl, pdecl_abstract } kind;
  struct declarator_specifiers_ *ds;
  struct declarator_ *decl;
  struct abstract_declarator_ *absdecl;
} n_parameter_declaration;

typedef struct initializer_
{
  position pos;
  enum { init_simple, init_bracket } kind;
  struct assignement_ *assign;
  struct initializer_list_ *init_list;
} n_initializer;

typedef struct initializer_list_
{
  position pos;
  struct initializer_list_ *init_list;
  struct initializer_ *init;
} n_initializer_list;

typedef struct expression_
{
  position pos;
  struct expression_ *expr;
  struct assignement_ *assign;
} n_expression;

typedef struct assignement_
{
  position pos;

  // always present
  struct logical_or_ *lor;

  // fields below not null if actual assignment
  struct assignement_ *assign;
  enum { opNONE, opASSIGN, opASSIGNPLUS, opASSIGNMINUS, opASSIGNDIV, opASSIGNMUL,
	 opASSIGNMOD, opASSIGNBOR, opASSIGNBAND, opASSIGNBXOR } op;

  // fields below not null if conditional 
  struct expression_ *expr;
  struct assignement_ *condassign;

  int compiler_internal;
  	/* flag to indicate if the assignment was issued
   	   internally by the compiler, as a side effect for
   	   ++ and -- operators */

  int is_declaration;
	/* flag to indicate if the assignment is also
	   a declaration */

} n_assignment;

typedef struct logical_or_
{
  position pos;
  struct logical_or_ *lor;
  struct logical_and_ *land;
} n_logical_or;

typedef struct logical_and_
{
  position pos;
  struct logical_and_ *land;
  struct inclusive_or_ *inclor;
} n_logical_and;

typedef struct inclusive_or_
{
  position pos;
  struct inclusive_or_ *inclor;
  struct exclusive_or_ *exclor;
} n_inclusive_or;

typedef struct exclusive_or_
{
  position pos;
  struct exclusive_or_ *exclor;
  struct and_ *and;
} n_exclusive_or;

typedef struct and_
{
  position pos;
  struct and_ *and;
  struct equality_ *eq;
} n_and;

typedef struct equality_
{
  position pos;
  struct equality_ *eq;
  enum
  { opEQ, opNE } op;
  struct relational_ *rel;
} n_equality;

typedef struct relational_
{
  position pos;
  struct relational_ *rel;
  enum
  { opGE, opGT, opLT, opLE } op;
  struct shift_ *shift;
} n_relational;

typedef struct shift_
{
  position pos;
  struct shift_ *shift;
  enum
  { opSHIFTL, opSHIFTR } op;
  struct additive_ *add;
} n_shift;

typedef struct additive_
{
  position pos;
  struct additive_ *add;
  enum
  { opPLUS, opMINUS } op;
  struct multiplicative_ *mul;
} n_additive;

typedef struct multiplicative_
{
  position pos;
  struct multiplicative_ *mul;
  enum
  { opMUL, opDIV, opMOD } op;
  struct cast_ *cast;
} n_multiplicative;

typedef struct
{
  position pos;
  struct unary_ *unary;
} n_cast_unary;

typedef struct type_name_
{
  position pos;
  struct type_name_specifiers_list_ *l;
  struct abstract_declarator_ *absdecl;
} n_type_name;

typedef struct type_name_specifiers_list_
{
  position pos;
  struct declarator_specifiers_ *s;
  struct type_name_specifiers_list_ *l;
} n_type_name_specifiers_list;

typedef struct cast_cast_
{
  position pos;
  struct type_name_ *tname;
  struct cast_ *cast;
} n_cast_cast;

typedef struct cast_
{
  position pos;
  enum
  { cast_cast, cast_unary } kind;
  union
  {
    n_cast_cast *cast;
    n_cast_unary *unary;
  } u;
} n_cast;

typedef struct unary_postfix_
{
  position pos;
  struct postfix_ *postfix;
} n_unary_postfix;

typedef struct unary_prefix_
{
  position pos;
  enum
  { opPPLUS, opMMINUS } op;
  struct unary_ *unary;
} n_unary_prefix;

typedef struct unary_cast_
{
  position pos;
  enum
  { opUPLUS, opUMINUS, opTILDE, opIND, opADDR, opNOT } op;
  struct cast_ *cast;
} n_unary_cast;

typedef struct unary_sizeof_
{
  position pos;
  enum { sizeof_typename, sizeof_expr } kind;
  union {
    struct type_name_ *tn;
    struct unary_ *un;
  } u;
} n_unary_sizeof;

typedef struct unary_
{
  position pos;
  enum
  { unary_postfix, unary_prefix, unary_cast, unary_sizeof } kind;
  union
  {
    n_unary_postfix *postfix;
    n_unary_prefix *prefix;
    n_unary_cast *cast;
    n_unary_sizeof *sof;
  } u;
} n_unary;

typedef struct postfix_primary_
{
  position pos;
  struct primary_ *pri;
} n_postfix_primary;

typedef struct postfix_pointer_
{
  position pos;
  struct postfix_ *postfix;
  char *id;
} n_postfix_pointer;

typedef struct postfix_dot_
{
  position pos;
  struct postfix_ *postfix;
  char *id;
} n_postfix_dot;

typedef struct postfix_post_
{
  position pos;
  struct postfix_ *postfix;
  enum
  { opPPPLUS, opPMMINUS } op;
} n_postfix_post;

typedef struct postfix_array_
{
  position pos;
  struct postfix_ *postfix;
  struct expression_ *expr;
} n_postfix_array;

typedef struct argument_expr_list_
{
  position pos;
  struct argument_expr_list_ *list;
  struct assignement_ *assign;
} n_argument_expr_list;

typedef struct postfix_call_
{
  position pos;
  struct postfix_ *designator;
  struct argument_expr_list_ *list;
} n_postfix_call;

typedef struct postfix_
{
  position pos;
  enum
  { postfix_primary, postfix_pointer, postfix_dot, postfix_post,
      postfix_array, postfix_call } kind;
  union
  {
    n_postfix_primary *pri;
    n_postfix_pointer *pointer;
    n_postfix_dot *dot;
    n_postfix_post *post;
    n_postfix_array *array;
    n_postfix_call *call;
  } u;
} n_postfix;

typedef struct primary_
{
  position pos;
  enum
  { primary_constant, primary_constant_string, primary_constant_char, 
    primary_identifier, primary_expr } kind;
  union
  {
    int constant;
    char *constant_string;
    char constant_char;
    char *id;
    struct expression_ *expr;
  } u;
  char *label_name;	
	/* if primary is a constant string, and the translation phase has already generated
      	   corresponding data fragment, hold the label name (otherwise is NULL). This allows
	   to avoid duplicates when creating string data fragments */
} n_primary;

typedef struct direct_abstract_declarator_
{
  position pos;
  enum { abstract_ptl, abstract_array } kind;
  union {
    struct {
      struct direct_abstract_declarator_ *d;
      n_parameter_type_list *l;
    } ptl;
    struct {
      struct direct_abstract_declarator_ *d;
      n_expression *expr;
    } array;
  } u;
} n_direct_abstract_declarator;

typedef struct abstract_declarator_
{
  position pos;
  n_pointer *p;
  n_direct_abstract_declarator *d;
} n_abstract_declarator;

n_unit *new_unit (n_external_declaration * ext, n_unit * u);
n_external_declaration *new_external_declaration_function (n_function_definition * f);
n_external_declaration *new_external_declaration_declaration (n_declaration * d);
n_function_definition *new_function_definition (n_declarator_specifiers * spec, n_declarator * d, n_compound_statement * stmt);
n_compound_statement *new_compound_statement (n_block_item_list * l);
n_block_item_list *new_block_item_list (n_block_item * i, n_block_item_list * l);
n_block_item *new_block_item_statement (n_statement * stmt);
n_block_item *new_block_item_declaration (n_declaration * decl);
n_statement *new_statement_compound (n_compound_statement * p);
n_statement *new_statement_expression (n_expression * p);
n_statement *new_statement_return (n_expression * p);
n_statement *new_statement_if (n_expression * expr, n_statement * True, n_statement * False);
n_statement *new_statement_while (n_expression * expr, n_statement * stmt);
n_statement *new_statement_for (n_expression * start, n_expression * cond, n_expression * end, n_statement * stmt);
n_statement *new_statement_do (n_statement * stmt, n_expression * cond);
n_statement *new_statement_switch (n_expression * cond, n_statement * stmt);
n_statement *new_statement_case (int kind, n_expression * cond);
n_statement *new_statement_goto (n_id goto_label);
n_statement *new_statement_label (n_id goto_label);
n_statement *new_statement_break ();
n_statement *new_statement_continue ();
n_declaration *new_declaration (n_declarator_specifiers * decl, n_init_declarator_list * init);
n_declarator_specifiers *new_declarator_specifiers_storage (n_storage_class_specifier * storage, n_declarator_specifiers * decl);
n_declarator_specifiers *new_declarator_specifiers_type (n_type_specifier * spec, n_declarator_specifiers * decl);
n_declarator_specifiers *new_declarator_specifiers_qualifier (n_type_qualifier * qual, n_declarator_specifiers * decl);
n_init_declarator_list *new_init_declarator_list (n_init_declarator * init, n_init_declarator_list * list);
n_init_declarator *new_init_declarator (n_declarator * decl, n_initializer * init);
n_storage_class_specifier *new_storage_class_specifier (int kind, int line, int column, char *filename);
n_struct_declarator *new_struct_declarator (n_declarator *d );
n_struct_declarator_list *new_struct_declarator_list (n_struct_declarator_list *sdl, n_struct_declarator *sd );
n_struct_declaration *new_struct_declaration (n_declarator_specifiers *sds, n_struct_declarator_list *sdl );
n_struct_declaration_list *new_struct_declaration_list (n_struct_declaration_list *l, n_struct_declaration *d );
n_struct_or_union_specifier *new_struct_or_union_specifier (int kind, n_id id, n_struct_declaration_list *sdl );
n_struct_or_union_specifier *new_struct_or_union_specifier_anonymous (int kind, n_struct_declaration_list *sdl );
n_type_specifier *new_type_specifier (n_id id);
n_type_specifier *new_type_specifier_su (n_struct_or_union_specifier *su);
n_type_specifier *new_type_specifier_enum (n_enumeration_specifier *es);
n_enumeration_specifier *new_named_enumeration_specifier( n_id id, n_enumerator_list *enuml);
n_enumeration_specifier *new_anonymous_enumeration_specifier( n_enumerator_list *enuml);
n_enumerator_list *new_enumerator_list( n_enumerator_list *el, n_enumerator *n);
n_enumerator *new_enumerator( n_id id, n_logical_or *lor);
n_type_qualifier *new_type_qualifier (int kind, int line, int column, char *filename);
n_type_qualifier_list *new_type_qualifier_list (n_type_qualifier_list *tql, n_type_qualifier *tq);
n_declarator *new_declarator (n_pointer * p, n_direct_declarator * direct);
n_pointer *new_pointer (n_type_qualifier_list *tql , n_pointer * p);
n_direct_declarator *new_direct_declarator_id (n_id id);
n_direct_declarator *new_direct_declarator_ptl (n_direct_declarator * dd, n_parameter_type_list * ptl);
n_direct_declarator *new_direct_declarator_array (n_direct_declarator * dd, n_expression * expr);
n_direct_abstract_declarator *new_direct_abstract_declarator_ptl (n_direct_abstract_declarator * dd, n_parameter_type_list * ptl);
n_direct_abstract_declarator *new_direct_abstract_declarator_array (n_direct_abstract_declarator * dd, n_expression * expr);
n_parameter_type_list *new_parameter_type_list (n_parameter_list * pl, int ellipsis);
n_parameter_list *new_parameter_list (n_parameter_declaration * pd, n_parameter_list * pl);
n_parameter_declaration *new_parameter_declaration (n_declarator_specifiers * ds, n_declarator * decl);
n_parameter_declaration *new_parameter_declaration_abstract (n_declarator_specifiers * ds, n_abstract_declarator * decl);
n_abstract_declarator *new_abstract_declarator (n_pointer *p, n_direct_abstract_declarator *d);
n_initializer *new_initializer (n_assignment * assign);
n_initializer *new_initializer_bracket (n_initializer_list * init_list);
n_initializer_list *new_initializer_list (n_initializer_list * init_list, n_initializer * init);
n_expression *new_expression (n_expression * expr, n_assignment * assign);
n_assignment *new_assignment (n_assignment * assign, n_keyword op, n_logical_or * lor);
n_assignment *new_assignment_conditional (n_logical_or * , n_expression * , n_assignment * );
n_logical_or *new_logical_or (n_logical_or * lor, n_logical_and * land);
n_logical_and *new_logical_and (n_logical_and * land, n_inclusive_or * inclor);
n_inclusive_or *new_inclusive_or (n_inclusive_or * inclor, n_exclusive_or * exclor);
n_exclusive_or *new_exclusive_or (n_exclusive_or * exclor, n_and * and);
n_and *new_and (n_and * and, n_equality * eq);
n_equality *new_equality (n_equality * eq, n_keyword op, n_relational * rel);
n_relational *new_relational (n_relational * rel, n_keyword op, n_shift * shift);
n_shift *new_shift (n_shift * shift, n_keyword op, n_additive * add);
n_additive *new_additive (n_additive * add, n_keyword op, n_multiplicative * mul);
n_multiplicative *new_multiplicative (n_multiplicative * mul, n_keyword op, n_cast * cast);
n_cast *new_cast_unary (n_unary * unary);
n_cast *new_cast_cast (n_type_name *tn, n_cast * cast);
n_type_name *new_type_name (n_type_name_specifiers_list *l, n_abstract_declarator *abs);
n_type_name_specifiers_list *new_type_name_specifiers_list( n_declarator_specifiers *s, n_type_name_specifiers_list *l);
n_unary *new_unary_postfix (n_postfix * postfix);
n_unary *new_unary_prefix (n_keyword op, n_unary * unary);
n_unary *new_unary_cast (n_keyword op, n_cast * cast);
n_unary *new_unary_sizeof_unary (n_unary *);
n_unary *new_unary_sizeof_type (n_type_name *);
n_postfix *new_postfix_primary (n_primary * pri);
n_postfix *new_postfix_pointer (n_postfix * postfix, n_id id);
n_postfix *new_postfix_dot (n_postfix * postfix, n_id id);
n_postfix *new_postfix_post (n_postfix * postfix, n_keyword op);
n_postfix *new_postfix_array (n_postfix * postfix, n_expression * expr);
n_postfix *new_postfix_call (n_postfix * designator, n_argument_expr_list * list);
n_argument_expr_list *new_argument_expr_list (n_argument_expr_list * list, n_assignment * assign);
n_primary *new_primary_constant (n_integer constant);
n_primary *new_primary_constant_string (n_string constant_string);
n_primary *new_primary_constant_string_from_list ( n_primary *, n_string constant_string);
n_primary *new_primary_constant_char (n_char constant_char);
n_primary *new_primary_identifier (n_id id);
n_primary *new_primary_expr (n_expression * expr);

#endif
