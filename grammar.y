/*
* Keen compiler 
* Copyright (C) 2003 Karim Ben Djedidia <kabend@free.fr> 
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not
* Foundation
* USA
*
*/

%{

#include "xmalloc.h"
#include "grammar.h"
#include "translate.h"
#include "error.h" 	/* for keen_error() */
#include "type.h"

/*
#define YYERROR_VERBOSE
*/

extern int yylex();
void yyerror(char *s);

n_keyword null_keyword = { { undefined, undefined, NULL }, undefined };

n_unit *unit;

%}

%union {
n_unit *u;
n_argument_expr_list *argument_expr_list;
n_external_declaration *external_declaration;
n_function_definition *function_definition;
n_compound_statement *compound_statement;
n_block_item_list *block_item_list;
n_block_item *block_item;
n_statement *stmt;
n_declaration *decl;
n_declarator_specifiers *decl_spec;
n_init_declarator_list *init_decl_list;
n_init_declarator *init_decl;
n_storage_class_specifier *storage_class;
n_type_specifier *type_spec;
n_type_qualifier *type_qual;
n_type_qualifier_list *type_qual_list;
n_declarator *declarator;
n_pointer *pointer;
n_direct_declarator *direct_decl;
n_parameter_type_list *parameter_type_list;
n_parameter_list *parameter_list;
n_parameter_declaration *parameter_declaration;
n_initializer *initializer;
n_initializer_list *initializer_list;
n_expression *e;
n_assignment *a;
n_logical_or *lor;
n_logical_and *land;
n_inclusive_or *inclor;
n_exclusive_or *exclor;
n_and *and;
n_equality *eq;
n_relational *rel;
n_shift *shift;
n_additive *add;
n_multiplicative *mul;
n_cast *cast;
n_unary *unary; 
n_postfix *postfix; 
n_primary *primary; 
n_keyword keyword;
n_integer i;
n_string s;
n_char c;
n_id id;
n_struct_declarator *sd;
n_struct_declarator_list *sdl;
n_struct_declaration *sdecl;
n_struct_declaration_list *sdecll;
n_struct_or_union_specifier *sus;
n_enumeration_specifier *enums;
n_enumerator_list *enuml;
n_enumerator *enumtor;
n_abstract_declarator *absdecl;
n_direct_abstract_declarator *absddecl;
n_type_name *tn;
n_type_name_specifiers_list *tnsl;
}


%token <keyword>  ELLIPSIS 
%token <keyword>  OPEN_BRACKET
%token <keyword>  CLOSE_BRACKET
%token <keyword>  OPEN_PAREN
%token <keyword>  CLOSE_PAREN
%token <keyword>  PLUS
%token <keyword>  MINUS
%token <keyword>  SEMICOLON
%token <keyword>  COLON
%token <keyword>  IF
%token <keyword>  OR
%token <keyword>  AND
%token <keyword>  BIOR
%token <keyword>  BXOR
%token <keyword>  BAND
%token <keyword>  NOT
%token <keyword>  MUL
%token <keyword>  DIV
%token <keyword>  GE
%token <keyword>  LE
%token <keyword>  LT
%token <keyword>  GT
%token <keyword>  EQ
%token <keyword>  NE
%token <keyword>  MOD
%token <keyword>  ELSE
%token <keyword>  WHILE
%token <keyword>  COMMA
%token <keyword>  FOR
%token <keyword>  DO
%token <keyword>  SWITCH 
%token <keyword>  GOTO 
%token <keyword>  CASE 
%token <keyword>  DEFAULT 
%token <keyword>  SP
%token <keyword>  RETURN
%token <keyword>  PPLUS
%token <keyword>  MMINUS
%token <keyword>  SHIFTL
%token <keyword>  SHIFTR
%token <keyword>  TILDE
%token <keyword>  IND
%token <keyword>  OPEN_ARRAY
%token <keyword>  CLOSE_ARRAY
%token <keyword>  ARROW 
%token <keyword>  DOT
%token <keyword>  ASSIGN
%token <keyword>  ASSIGNMUL
%token <keyword>  ASSIGNDIV
%token <keyword>  ASSIGNMOD
%token <keyword>  ASSIGNPLUS
%token <keyword>  ASSIGNMINUS
%token <keyword>  ASSIGNAND
%token <keyword>  ASSIGNBXOR
%token <keyword>  ASSIGNBAND
%token <keyword>  ASSIGNBOR
%token <keyword>  EXTERN
%token <keyword>  STATIC 
%token <keyword>  AUTO 
%token <keyword>  REGISTER 
%token <keyword>  TYPEDEF 
%token <keyword>  CONST 
%token <keyword>  RESTRICT 
%token <keyword>  VOLATILE 
%token <keyword>  BREAK 
%token <keyword>  CONTINUE 
%token <keyword>  STRUCT 
%token <keyword>  UNION 
%token <keyword>  ENUM 
%token <keyword>  SIZEOF 
%token <keyword>  QUESTIONMARK 
%token <i>        CONSTANT 
%token <s>        CONSTANT_STRING 
%token <c>        CONSTANT_CHAR
%token <id>       IDENTIFIER 
%token <id>       TYPENAME 

%start translation_unit 

%type  <u> unit 
%type  <u> translation_unit 
%type  <argument_expr_list> argument_expr_list
%type  <external_declaration> external_declaration
%type  <function_definition> function_definition
%type  <compound_statement> compound_statement
%type  <block_item_list> block_item_list
%type  <block_item> block_item
%type  <stmt> statement 
%type  <decl> declaration 
%type  <decl_spec> declaration_specifiers
%type  <init_decl_list> init_declarator_list
%type  <init_decl> init_declarator
%type  <storage_class> storage_class_specifier
%type  <type_spec> type_specifier
%type  <type_qual> type_qualifier
%type  <type_qual_list> type_qualifier_list
%type  <declarator> declarator
%type  <pointer> pointer
%type  <direct_decl> direct_declarator
%type  <parameter_type_list> parameter_type_list
%type  <parameter_list> parameter_list
%type  <parameter_declaration> parameter_declaration
%type  <initializer> initializer 
%type  <initializer_list> initializer_list
%type  <e> expression 
%type  <e> expression_opt
%type  <a> assignment 
%type  <lor> logical_or 
%type  <land> logical_and 
%type  <inclor> inclusive_or 
%type  <exclor> exclusive_or 
%type  <and> and
%type  <eq> equality
%type  <rel> relational
%type  <shift> shift
%type  <add> additive
%type  <mul> multiplicative
%type  <cast> cast
%type  <unary> unary 
%type  <postfix> postfix 
%type  <primary> primary 
%type  <primary> constant_string_list 
%type  <sd> struct_declarator
%type  <sdl> struct_declarator_list
%type  <sdecl> struct_declaration
%type  <decl_spec> struct_declaration_specifiers
%type  <sdecll> struct_declaration_list
%type  <sus> struct_or_union_specifier
%type  <enums> enumeration_specifier
%type  <enuml> enumerator_list
%type  <enumtor> enumerator
%type  <absdecl> abstract_declarator
%type  <absddecl> direct_abstract_declarator
%type  <tn> type_name
%type  <tnsl> type_name_specifiers_list
%type  <decl_spec> type_name_specifiers

%nonassoc ASSIGNMUL ASSIGNDIV ASSIGNPLUS ASSIGNMINUS ASSIGNBAND ASSIGNBXOR ASSIGNBOR ASSIGNMOD

%right ASSIGN
%left OR
%left AND
%left BIOR
%left BXOR
%left BAND
%left EQ NE 
%left GE GT LE LT 
%left SHIFTR SHIFTL 
%left PLUS MINUS
%left MUL DIV MOD
%right PPLUS MMINUS UNARY_PLUS UNARY_MINUS TILDE NOT INDIRECTION ADDRESS
%right ARROW 

%%

translation_unit: unit					{ unit = $1; }
		;

unit:		  external_declaration			{ $$ = new_unit( $1, NULL ); }
	 	| external_declaration unit		{ $$ = new_unit( $1, $2 ); }
    		;

external_declaration:
		  function_definition			{ $$ = new_external_declaration_function( $1 ); }
		| declaration				{ $$ = new_external_declaration_declaration( $1 ); 
							  /* update symbol table with new type declarations, if any */
							  parse_declaration_for_typedefs( $1 ); 
							}
		;

function_definition:
		  declaration_specifiers declarator compound_statement	{ $$ = new_function_definition( $1, $2, $3 ); }
		| declaration_specifiers OPEN_PAREN { keen_error ("function redeclares an existing symbol"); }
		| declaration_specifiers declarator OPEN_BRACKET error 		
						{ $$ = NULL; 
						  SET_POS( $3.pos );
						  keen_error("(in grammar) syntax error in function body"); 
						}  
		| declaration_specifiers error OPEN_BRACKET		
						{ $$ = NULL; 
						  SET_POS( $3.pos );
						  keen_error("(in grammar) syntax error in function definition"); 
						}  
		| error OPEN_BRACKET 		{ $$ = NULL; 
					  	  SET_POS( $2.pos );
					  	  keen_error("(in grammar) syntax error"); 
						}  
		| declaration_specifiers error CLOSE_BRACKET		
				 		{ $$ = NULL; 
					  	  SET_POS( $3.pos );
					  	  keen_error("(in grammar) syntax error"); 
						}  
		;

compound_statement: 
		  OPEN_BRACKET CLOSE_BRACKET	{
		  				  $$ = new_compound_statement( NULL ); 
						}

		| OPEN_BRACKET { open_scope(); } block_item_list CLOSE_BRACKET	
						{
						  $$ = new_compound_statement( $3 ); 
						  close_scope();	
						}
		;

block_item_list:  block_item				{ $$ = new_block_item_list( $1, NULL ); }
	        | block_item block_item_list		{ $$ = new_block_item_list( $1, $2 ); }
		;

block_item:	  declaration				{ $$ = new_block_item_declaration( $1 ); 
							  /* update symbol table with new type declarations, if any */
							  parse_declaration_for_typedefs( $1 ); 
							}
	  	| statement				{ $$ = new_block_item_statement( $1 ); }
		;

statement:        compound_statement					{ $$ = new_statement_compound( $1 ); }
		| SEMICOLON	 					{ $$ = new_statement_expression( NULL ); }
		| expression SEMICOLON 					{ $$ = new_statement_expression( $1 ); }
		| IF OPEN_PAREN expression CLOSE_PAREN statement	{ $$ = new_statement_if( $3, $5, NULL ); }
		| IF OPEN_PAREN expression CLOSE_PAREN statement ELSE statement	{ $$ = new_statement_if( $3, $5, $7 ); }
		| RETURN SEMICOLON					{ $$ = new_statement_return( NULL ); }
		| RETURN expression SEMICOLON				{ $$ = new_statement_return( $2 ); }
		| WHILE OPEN_PAREN expression CLOSE_PAREN statement	{ $$ = new_statement_while( $3, $5 ); }
		| BREAK SEMICOLON					{ $$ = new_statement_break(); }
		| CONTINUE SEMICOLON					{ $$ = new_statement_continue(); }
		| FOR OPEN_PAREN 
                             expression_opt SEMICOLON 
                             expression_opt SEMICOLON
                             expression_opt CLOSE_PAREN
                             statement					{ $$ = new_statement_for( $3, $5, $7, $9 ); }
                | SWITCH OPEN_PAREN expression CLOSE_PAREN 
                    statement                                           { $$ = new_statement_switch ( $3, $5 ); }
                | CASE expression COLON                                 { $$ = new_statement_case( ci_case, $2 ); }
                | DEFAULT COLON                                         { $$ = new_statement_case( ci_default, NULL ); }
                | GOTO IDENTIFIER SEMICOLON                             { $$ = new_statement_goto( $2 ); }
                | IDENTIFIER COLON                                      { $$ = new_statement_label( $1 ); }
                | DO statement WHILE OPEN_PAREN expression CLOSE_PAREN  
                     SEMICOLON                                          { $$ = new_statement_do( $2, $5 ); }
		;

declaration:	  declaration_specifiers init_declarator_list SEMICOLON { $$ = new_declaration( $1 , $2 ); }  
		| declaration_specifiers SEMICOLON 			{ $$ = new_declaration( $1 , NULL ); }  
		| error SEMICOLON		{ $$ = NULL; 
						  SET_POS( $2.pos );
						  keen_error("(in grammar) syntax error in declaration"); 
						}  
		;

declaration_specifiers:	
		  storage_class_specifier				{ $$ = new_declarator_specifiers_storage( $1, NULL); }
		| storage_class_specifier declaration_specifiers 	{ $$ = new_declarator_specifiers_storage( $1, $2); } 
		| type_specifier					{ $$ = new_declarator_specifiers_type( $1, NULL); }
		| type_specifier declaration_specifiers 		{ $$ = new_declarator_specifiers_type( $1, $2); } 
		| type_qualifier					{ $$ = new_declarator_specifiers_qualifier( $1, NULL); }
		| type_qualifier declaration_specifiers 		{ $$ = new_declarator_specifiers_qualifier( $1, $2); } 
		;

storage_class_specifier:
		  EXTERN	{ $$ = new_storage_class_specifier( storage_extern, 0, 0, NULL ); }
		| STATIC	{ $$ = new_storage_class_specifier( storage_static, 0, 0, NULL ); }
		| AUTO		{ $$ = new_storage_class_specifier( storage_auto, 0, 0, NULL); }
		| REGISTER	{ $$ = new_storage_class_specifier( storage_register, 0, 0, NULL ); }
		| TYPEDEF	{ $$ = new_storage_class_specifier( storage_typedef, 0, 0, NULL ); }
		;

init_declarator_list:
		  init_declarator				{ $$ = new_init_declarator_list( $1, NULL ); }
		| init_declarator COMMA init_declarator_list	{ $$ = new_init_declarator_list( $1, $3 ); }	 	
		;

init_declarator:  
		  declarator					{ $$ = new_init_declarator($1, NULL); }
		| declarator ASSIGN initializer			{ $$ = new_init_declarator($1, $3); }
		;

type_specifier:	  TYPENAME					{ $$ = new_type_specifier( $1 ); }
                | struct_or_union_specifier                     { $$ = new_type_specifier_su( $1 ); }
                | enumeration_specifier                         { $$ = new_type_specifier_enum( $1 ); }
		;

enumeration_specifier:	ENUM IDENTIFIER				{ $$ = new_named_enumeration_specifier( $2 , NULL ); }
		|       ENUM IDENTIFIER OPEN_BRACKET 
				enumerator_list	CLOSE_BRACKET	{ $$ = new_named_enumeration_specifier( $2 , $4 ); }
		|       ENUM OPEN_BRACKET 
				enumerator_list	CLOSE_BRACKET	{ $$ = new_anonymous_enumeration_specifier( $3 ); }
		;

enumerator_list:  enumerator					{ $$ = new_enumerator_list( NULL, $1); }
		| enumerator COMMA enumerator_list		{ $$ = new_enumerator_list( $3, $1 ); }
		| enumerator COMMA 				{ $$ = new_enumerator_list( NULL, $1 ); }
		;

enumerator:	  IDENTIFIER					{ $$ = new_enumerator( $1 , NULL ); }
		| IDENTIFIER ASSIGN logical_or			{ $$ = new_enumerator( $1, $3 ); }
		;

struct_or_union_specifier :
                  STRUCT OPEN_BRACKET struct_declaration_list CLOSE_BRACKET { $$ = new_struct_or_union_specifier_anonymous( su_struct, $3 ); }
                | UNION OPEN_BRACKET struct_declaration_list CLOSE_BRACKET { $$ = new_struct_or_union_specifier_anonymous( su_union, $3 ); }
                | STRUCT IDENTIFIER OPEN_BRACKET struct_declaration_list CLOSE_BRACKET { $$ = new_struct_or_union_specifier( su_struct, $2, $4 ); }
                | STRUCT TYPENAME OPEN_BRACKET struct_declaration_list CLOSE_BRACKET { $$ = new_struct_or_union_specifier( su_struct, $2, $4 ); }
                | UNION IDENTIFIER OPEN_BRACKET struct_declaration_list CLOSE_BRACKET { $$ = new_struct_or_union_specifier( su_union, $2, $4 ); }
                | UNION TYPENAME OPEN_BRACKET struct_declaration_list CLOSE_BRACKET { $$ = new_struct_or_union_specifier( su_union, $2, $4 ); }
                | STRUCT IDENTIFIER                    { $$ = new_struct_or_union_specifier( su_struct, $2, NULL ); }
                | STRUCT TYPENAME                    { $$ = new_struct_or_union_specifier( su_struct, $2, NULL ); }
                | UNION IDENTIFIER                     { $$ = new_struct_or_union_specifier( su_union, $2, NULL ); }
                | UNION TYPENAME                     { $$ = new_struct_or_union_specifier( su_union, $2, NULL ); }
			/* rules with TYPENAME are valid because a typename and struct typename can be disambiguated
			   by the keyword 'struct', so can share same name. Note that TYPENAME here cannot be a reserved
			   keyword (as int, short...). This is tested during translation phase, not here */
                ;

struct_declaration_list:
                  struct_declaration                            { $$ = new_struct_declaration_list( NULL, $1 ); }
                | struct_declaration_list struct_declaration    { $$ = new_struct_declaration_list( $1, $2 ); }
                ;

struct_declaration_specifiers:	
                  /* of type n_declarator_specifiers, but with no storage */
		  type_specifier					{ $$ = new_declarator_specifiers_type( $1, NULL); }
		| type_specifier struct_declaration_specifiers 		{ $$ = new_declarator_specifiers_type( $1, $2); } 
		| type_qualifier					{ $$ = new_declarator_specifiers_qualifier( $1, NULL); }
		| type_qualifier struct_declaration_specifiers 		{ $$ = new_declarator_specifiers_qualifier( $1, $2); } 
		;

type_name_specifiers_list:
		  type_name_specifiers					{ $$ = new_type_name_specifiers_list( $1, NULL); }
		;

type_name_specifiers:
		  struct_declaration_specifiers			// same as for struct, so no need to redefine something else...
		;

struct_declaration:
                  struct_declaration_specifiers struct_declarator_list SEMICOLON { $$ = new_struct_declaration( $1, $2 ); }
                | struct_declaration_specifiers SEMICOLON { $$ = new_struct_declaration( $1, NULL ); }
			/* this rule is used when a member has same name as an existing type. This is valid because
			   both names can be disambiguated according to the context. But as the grammar is not able
			   to match with rule 1 above, we need to allow empty declarator, and rewrite the declaration
			   later on during translation phase */
                ;

struct_declarator_list:
                  struct_declarator                                 { $$ = new_struct_declarator_list( NULL, $1 ); } 
                | struct_declarator_list COMMA struct_declarator    { $$ = new_struct_declarator_list( $1, $3 ); } 
                ;

struct_declarator:
                  declarator                                        { $$ = new_struct_declarator( $1 ); }
                  /* there is room here to add bit-fields */
                ;

type_qualifier:
		  CONST		{ $$ = new_type_qualifier( type_qual_const, $1.pos.line, $1.pos.column, $1.pos.filename ); }
		| RESTRICT	{ $$ = new_type_qualifier( type_qual_restrict, $1.pos.line, $1.pos.column, $1.pos.filename ); }
		| VOLATILE	{ $$ = new_type_qualifier( type_qual_volatile, $1.pos.line, $1.pos.column, $1.pos.filename ); }
		;

declarator:	  direct_declarator				{ $$ = new_declarator( NULL, $1 ); }
	  	| pointer direct_declarator			{ $$ = new_declarator( $1, $2 ); }
		;

type_qualifier_list:
		  type_qualifier				{ $$ = new_type_qualifier_list ( NULL, $1 ); }
		| type_qualifier_list type_qualifier		{ $$ = new_type_qualifier_list ( $1, $2 ); }
		;

pointer:	  MUL						{ $$ = new_pointer( NULL, NULL ); /* FIXME we should pass position in this case */ }
       		| MUL pointer					{ $$ = new_pointer( NULL, $2 ); }
       		| MUL type_qualifier_list pointer		{ $$ = new_pointer( $2, $3 ); }
       		| MUL type_qualifier_list 			{ $$ = new_pointer( $2, NULL ); }
		;

direct_declarator: IDENTIFIER					{ $$ = new_direct_declarator_id( $1 ); }
 		|  TYPENAME					{ $$ = new_direct_declarator_id( $1 ); }
		|  direct_declarator OPEN_PAREN parameter_type_list CLOSE_PAREN { $$ = new_direct_declarator_ptl( $1, $3 ); }
		|  direct_declarator OPEN_PAREN CLOSE_PAREN     { $$ = new_direct_declarator_ptl( $1, NULL ); }
		|  direct_declarator OPEN_ARRAY CLOSE_ARRAY     { $$ = new_direct_declarator_array( $1, NULL ); }
		|  direct_declarator OPEN_ARRAY expression CLOSE_ARRAY     { $$ = new_direct_declarator_array( $1, $3 ); }
		/* room here for '( direct_declarator )' */
		;

direct_abstract_declarator:
		   direct_abstract_declarator OPEN_PAREN parameter_type_list CLOSE_PAREN { $$ = new_direct_abstract_declarator_ptl( $1, $3 ); }
		|  direct_abstract_declarator OPEN_PAREN CLOSE_PAREN     { $$ = new_direct_abstract_declarator_ptl( $1, NULL ); }
		|  OPEN_PAREN parameter_type_list CLOSE_PAREN { $$ = new_direct_abstract_declarator_ptl( NULL, $2 ); }
		|  OPEN_PAREN CLOSE_PAREN { $$ = new_direct_abstract_declarator_ptl( NULL, NULL ); }
		|  direct_abstract_declarator OPEN_ARRAY CLOSE_ARRAY     { $$ = new_direct_abstract_declarator_array( $1, NULL ); }
		|  direct_abstract_declarator OPEN_ARRAY expression CLOSE_ARRAY     { $$ = new_direct_abstract_declarator_array( $1, $3 ); }
		|  OPEN_ARRAY CLOSE_ARRAY     { $$ = new_direct_abstract_declarator_array( NULL, NULL ); }
		|  OPEN_ARRAY expression CLOSE_ARRAY     { $$ = new_direct_abstract_declarator_array( NULL, $2 ); }
		/* room here for '( direct_abstract_declarator )' */
		;

abstract_declarator:
		   direct_abstract_declarator			{ $$ = new_abstract_declarator(NULL, $1); }
		|  pointer direct_abstract_declarator		{ $$ = new_abstract_declarator($1, $2); }
		|  pointer					{ $$ = new_abstract_declarator($1, NULL); }
		;
 
parameter_type_list: parameter_list				{ $$ = new_parameter_type_list( $1, /* no ellipsis */ 0 ); }
		|    parameter_list COMMA ELLIPSIS		{ $$ = new_parameter_type_list( $1, /*    ellipsis */ 1 ); }
		;

parameter_list:	  parameter_declaration				{ $$ = new_parameter_list( $1, NULL ); }
	      	| parameter_list COMMA parameter_declaration	{ $$ = new_parameter_list( $3, $1 ); } 
		;

parameter_declaration: 
		  declaration_specifiers declarator		{ $$ = new_parameter_declaration( $1, $2 ); }
		| declaration_specifiers abstract_declarator	{ $$ = new_parameter_declaration_abstract( $1, $2 ); }
		| declaration_specifiers 			{ $$ = new_parameter_declaration_abstract( $1, NULL ); }
		;

initializer:	  assignment					      { $$ = new_initializer( $1 ); }
                | OPEN_BRACKET initializer_list CLOSE_BRACKET         { $$ = new_initializer_bracket( $2 ); }
	   	;

initializer_list: 
                  initializer                                         { $$ = new_initializer_list( NULL, $1 ); }
                | initializer COMMA                                   { $$ = new_initializer_list( NULL, $1 ); }
                | initializer COMMA initializer_list                  { $$ = new_initializer_list( $3, $1 ); }
                ;

expression_opt:	  expression 					{ $$ = $1; }
                |                                               { $$ = NULL; }
                ;

expression:	  assignment					{ $$ = new_expression( NULL, $1 ); }
	  	| expression COMMA assignment			{ $$ = new_expression( $1 , $3 ); }
	  	;

assignment:	  logical_or						{ $$ = new_assignment( NULL, null_keyword, $1); }
	  	| logical_or ASSIGN assignment 				{ $$ = new_assignment( $3, setOp( $2, opASSIGN ), $1); }
	  	| logical_or ASSIGNPLUS assignment 			{ $$ = new_assignment( $3, setOp( $2, opASSIGNPLUS ), $1); }
	  	| logical_or ASSIGNMINUS assignment			{ $$ = new_assignment( $3, setOp( $2, opASSIGNMINUS ), $1); }
	  	| logical_or ASSIGNMUL assignment			{ $$ = new_assignment( $3, setOp( $2, opASSIGNMUL ), $1); }
	  	| logical_or ASSIGNDIV assignment			{ $$ = new_assignment( $3, setOp( $2, opASSIGNDIV ), $1); }
	  	| logical_or ASSIGNBOR assignment			{ $$ = new_assignment( $3, setOp( $2, opASSIGNBOR ), $1); }
	  	| logical_or ASSIGNBAND assignment			{ $$ = new_assignment( $3, setOp( $2, opASSIGNBAND ), $1); }
	  	| logical_or ASSIGNBXOR assignment			{ $$ = new_assignment( $3, setOp( $2, opASSIGNBXOR ), $1); }
	  	| logical_or ASSIGNMOD assignment			{ $$ = new_assignment( $3, setOp( $2, opASSIGNMOD ), $1); }
		| logical_or QUESTIONMARK expression COLON assignment	{ $$ = new_assignment_conditional ( $1, $3, $5 ); } 
		;

logical_or: 	  logical_and					{ $$ = new_logical_or( NULL, $1 ); }
	  	| logical_or OR logical_and 			{ $$ = new_logical_or( $1, $3 ); }
	  	;

logical_and: 	  inclusive_or					{ $$ = new_logical_and( NULL, $1 ); }
	  	| logical_and AND inclusive_or 			{ $$ = new_logical_and( $1, $3 ); }
	  	;

inclusive_or: 	  exclusive_or					{ $$ = new_inclusive_or( NULL, $1 ); }
	  	| inclusive_or BIOR exclusive_or		{ $$ = new_inclusive_or( $1, $3 ); }
	  	;

exclusive_or: 	  and						{ $$ = new_exclusive_or( NULL, $1 ); }
	  	| exclusive_or BXOR and				{ $$ = new_exclusive_or( $1, $3 ); }
	  	;

and: 	  	  equality					{ $$ = new_and( NULL, $1 ); }
	  	| and BAND equality				{ $$ = new_and( $1, $3 ); }
	  	;

equality:  	  relational					{ $$ = new_equality( NULL, null_keyword, $1 ); }
	  	| equality EQ relational			{ $$ = new_equality( $1, setOp( $2, opEQ), $3 ); }
	  	| equality NE relational			{ $$ = new_equality( $1, setOp( $2, opNE), $3 ); }
	  	;

relational:  	  shift						{ $$ = new_relational( NULL, null_keyword, $1 ); }
	  	| relational GE shift				{ $$ = new_relational( $1, setOp( $2, opGE), $3 ); }
	  	| relational GT shift				{ $$ = new_relational( $1, setOp( $2, opGT), $3 ); }
	  	| relational LE shift				{ $$ = new_relational( $1, setOp( $2, opLE), $3 ); }
	  	| relational LT shift				{ $$ = new_relational( $1, setOp( $2, opLT), $3 ); }
	  	;

shift:  	  additive					{ $$ = new_shift( NULL, null_keyword, $1 ); }
	  	| shift SHIFTL additive				{ $$ = new_shift( $1, setOp( $2, opSHIFTL), $3 ); }
	  	| shift SHIFTR additive				{ $$ = new_shift( $1, setOp( $2, opSHIFTR), $3 ); }
	  	;

additive:  	  multiplicative				{ $$ = new_additive( NULL, null_keyword, $1 ); }
		| additive PLUS multiplicative			{ $$ = new_additive( $1, setOp( $2,opPLUS), $3 ); }
		| additive MINUS multiplicative			{ $$ = new_additive( $1, setOp( $2,opMINUS), $3 ); }
	  	;

multiplicative:	  cast						{ $$ = new_multiplicative( NULL, null_keyword,  $1 ); }
		| multiplicative MUL cast 			{ $$ = new_multiplicative( $1, setOp($2,opMUL), $3 ); }
		| multiplicative DIV cast 			{ $$ = new_multiplicative( $1, setOp($2,opDIV), $3 ); }
		| multiplicative MOD cast 			{ $$ = new_multiplicative( $1, setOp($2,opMOD), $3 ); }
	  	;

cast:		  OPEN_PAREN type_name CLOSE_PAREN cast		{ $$ = new_cast_cast( $2, $4 ); }
    		| unary						{ $$ = new_cast_unary( $1 ); }
    		;

type_name:	  type_name_specifiers_list abstract_declarator	{ $$ = new_type_name( $1, $2 ); }
		| type_name_specifiers_list 			{ $$ = new_type_name( $1, NULL ); }
		;
 
unary:		  postfix					{ $$ = new_unary_postfix( $1 ); }
     		| PPLUS unary					{ $$ = new_unary_prefix( setOp( $1,opPPLUS), $2 ); }
     		| MMINUS unary					{ $$ = new_unary_prefix( setOp( $1,opMMINUS), $2 ); }
     		| PLUS cast	%prec UNARY_PLUS		{ $$ = new_unary_cast( setOp( $1,opUPLUS), $2 ); }
     		| MINUS cast	%prec UNARY_MINUS		{ $$ = new_unary_cast( setOp( $1,opUMINUS), $2 ); }
     		| TILDE cast					{ $$ = new_unary_cast( setOp( $1,opTILDE), $2 ); }
     		| MUL cast      %prec INDIRECTION		{ $$ = new_unary_cast( setOp( $1,opIND), $2 ); }
     		| BAND cast 	%prec ADDRESS			{ $$ = new_unary_cast( setOp( $1,opADDR), $2 ); }
     		| NOT cast 					{ $$ = new_unary_cast( setOp( $1,opNOT), $2 ); }
		| SIZEOF unary					{ $$ = new_unary_sizeof_unary ( $2 ); }
		| SIZEOF OPEN_PAREN type_name CLOSE_PAREN	{ $$ = new_unary_sizeof_type ( $3 ); }
     		;

postfix:	  primary					{ $$ = new_postfix_primary( $1 ); }
		| postfix PPLUS					{ $$ = new_postfix_post( $1, setOp( $2, opPPPLUS) ); } 
		| postfix MMINUS				{ $$ = new_postfix_post( $1, setOp( $2, opPMMINUS) ); } 
		| postfix OPEN_ARRAY expression CLOSE_ARRAY	{ $$ = new_postfix_array( $1, $3 ); } 
		| postfix OPEN_PAREN CLOSE_PAREN		{ $$ = new_postfix_call( $1, NULL ); } 
		| postfix OPEN_PAREN argument_expr_list CLOSE_PAREN 	{ $$ = new_postfix_call( $1, $3 ); } 
		| postfix ARROW IDENTIFIER			{ $$ = new_postfix_pointer( $1, $3 ); } 
		| postfix DOT IDENTIFIER			{ $$ = new_postfix_dot( $1, $3 ); } 
		| postfix ARROW TYPENAME			{ $$ = new_postfix_pointer( $1, $3 ); } 
		| postfix DOT TYPENAME				{ $$ = new_postfix_dot( $1, $3 ); } 
			/* rules with typename allow struct/union names to share type names */
		;

argument_expr_list:
		  assignment					{ $$ = new_argument_expr_list( NULL , $1 ); }
		| argument_expr_list COMMA assignment		{ $$ = new_argument_expr_list( $1, $3 ); }
		;

constant_string_list:
		  constant_string_list CONSTANT_STRING		{ $$ = new_primary_constant_string_from_list ( $1, $2 ); }
		| CONSTANT_STRING				{ $$ = new_primary_constant_string ($1 ); }
		;

primary:	  CONSTANT					{ $$ = new_primary_constant( $1 ); }
		| constant_string_list 				{ $$ = $1; }
		| CONSTANT_CHAR 				{ $$ = new_primary_constant_char( $1 ); }
       		| IDENTIFIER					{ $$ = new_primary_identifier( $1 ); }
		| OPEN_PAREN expression CLOSE_PAREN		{ $$ = new_primary_expr( $2 ); }
		;


%%

void yyerror (char *s) {
 // do nothing 
 // actual messages are treated directly in error action rules
}

void parse() {
 /* go parse */
 extern int yydebug;
 yydebug = 0;
 SET_LOC( location_column );
 yyparse();
}

