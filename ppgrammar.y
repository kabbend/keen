/*
* Keen compiler 
* Copyright (C) 2007 Karim Ben Djedidia <kabend@free.fr> 
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "keen.h" 	/* for keen_error() */

/* 
 * Preprocessor grammar for expressions in #if directives
 * 
 * Today preprocessor is not based on yacc/bison parser for the whole
 * parsing but only for arithmetic expressions in directives.
 *
 * We expect here a pure arithmetic expression, ie. all macros have
 * been resolved previously, and the given expression is a logical line,
 * not a raw input text (ie. all trail, linefeed, special characters, etc.
 * have been removed).
 */

signed long int pp_expr_value;
	/* global variable that will hold expression result */


char *pp_string_expression;
	/* pointer into expression string */


/* read next character in expression */
int
pp_getc ()
{
  return *(pp_string_expression++);
}


/* undo last read in expression. No boundary check is done. */
void
pp_ungetc ()
{
  pp_string_expression--;
}


/* parse given expression string. Return numerical value. */
signed long int
prep_expr (char *expression)
{
  pp_string_expression = expression;

  int r = pp_yyparse ();
  if (r != 0) keen_error("error while parsing expression");

  return pp_expr_value;
}

%}

%token PP_OPEN_PAREN
%token PP_CLOSE_PAREN
%token PP_PLUS
%token PP_MINUS
%token PP_OR
%token PP_AND
%token PP_NOT
%token PP_MUL
%token PP_DIV
%token PP_GE
%token PP_LE
%token PP_LT
%token PP_GT
%token PP_EQ
%token PP_NE
%token PP_MOD
%token PP_CONSTANT 
%token PP_QMARK 
%token PP_DDOT 

%start expression 

%left PP_OR
%left PP_AND
%left PP_EQ PP_NE 
%left PP_GE PP_GT PP_LE PP_LT 
%left PP_PLUS PP_MINUS
%left PP_MUL PP_DIV PP_MOD
%left PP_QMARK PP_DDOT
%right PP_UNARY_PLUS PP_UNARY_MINUS PP_NOT

%%

expression:	  conditional_expression			{ pp_expr_value = $1; }
		;

conditional_expression:
		  logical_or									{ $$ = $1; }
		| logical_or PP_QMARK conditional_expression PP_DDOT conditional_expression 	{ $$ = ($1)?($3):($5);}
		;

logical_or: 	  logical_and					{ $$ = $1; }
	  	| logical_or PP_OR logical_and 			{ $$ = ($1 || $3); }
	  	;

logical_and: 	  equality					{ $$ = $1; }
	  	| logical_and PP_AND equality 			{ $$ = ($1 && $3); }
	  	;

equality:  	  relational					{ $$ = $1; }
	  	| equality PP_EQ relational			{ $$ = ($1 == $3); }
	  	| equality PP_NE relational			{ $$ = ($1 != $3); }
	  	;

relational:  	  additive					{ $$ = $1; }
	  	| relational PP_GE additive			{ $$ = ($1 >= $3); }
	  	| relational PP_GT additive			{ $$ = ($1 > $3); }
	  	| relational PP_LE additive			{ $$ = ($1 <= $3 ); }
	  	| relational PP_LT additive			{ $$ = ($1 < $3 ); }
	  	;

additive:  	  multiplicative				{ $$ = $1; }
		| additive PP_PLUS multiplicative		{ $$ = $1 + $3; }
		| additive PP_MINUS multiplicative		{ $$ = $1 - $3; }
	  	;

multiplicative:	  unary						{ $$ = $1; }
		| multiplicative PP_MUL unary 			{ $$ = $1 * $3; }
		| multiplicative PP_DIV unary 			{ if ($3 == 0) {
									keen_error("division by zero"); 
									exit(EXIT_FAILURE);
								  }
								  $$ = $1 / $3;
								}
		| multiplicative PP_MOD unary 			{ $$ = $1 % $3; }
	  	;

unary:		  primary					{ $$ = $1; }
     		| PP_PLUS unary	%prec PP_UNARY_PLUS		{ $$ = +$2; }
     		| PP_MINUS unary %prec PP_UNARY_MINUS		{ $$ = -$2; }
     		| PP_NOT unary 					{ $$ = ($2 == 0); }
     		;

primary:	  PP_CONSTANT						{ $$ = $1; }
		| PP_OPEN_PAREN expression PP_CLOSE_PAREN		{ $$ = $2; }
		;


%%

int pp_yylex() {

  int c, i;
  char value[64];

  /* read expression until non blank */
  c = pp_getc();
  while( isblank( c )) c = pp_getc();
  if (c == 0) return 0;

  /* NUMERIC */
  if (isdigit(c)) {

   i = 0;
   do { value[i++] = c; c = pp_getc(); } while(isdigit(c));

   /* treat eventual trailing "L" long symbol */
   if (c == 'l' || c == 'L') 
     {
	value[i++] = 'L';
     } 
   else
     pp_ungetc();

   // close string
   value[i] = 0;

   // get long value
   sscanf( value , "%d" , &yylval );
   return PP_CONSTANT;

  }

  switch(c) {

   case '+': return PP_PLUS;
   case '-': return PP_MINUS;
   case '(': return PP_OPEN_PAREN;
   case ')': return PP_CLOSE_PAREN;
   case '/': return PP_DIV;
   case '%': return PP_MOD;
   case '*': return PP_MUL;
   case '?': return PP_QMARK;
   case ':': return PP_DDOT;
   case '=':
	c = pp_getc();
	if (c == '=') return PP_EQ;
	keen_error("unknown operator '=%c' in expression",c);
	exit(EXIT_FAILURE);
   case '&':
	c = pp_getc();
	if (c == '&') return PP_AND;
	keen_error("unknown operator '&%c' in expression",c);
	exit(EXIT_FAILURE);
   case '|':
	c = pp_getc();
	if (c == '|') return PP_OR;
	keen_error("unknown operator '|%c' in expression",c);
	exit(EXIT_FAILURE);
   case '!':
	c = pp_getc();
	if (c == '=') return PP_NE;
	pp_ungetc();
	return PP_NOT;
   case '>':
	c = pp_getc();
	if (c == '=') return PP_GE;
	pp_ungetc();
	return PP_GT;	
   case '<':
	c = pp_getc();
	if (c == '=') return PP_LE;
	pp_ungetc();
	return PP_LT;	
   default:
	keen_error("unexpected character '%c' in expression",c);
	exit(EXIT_FAILURE);
  }
}

int pp_yyerror(char *s) {
 keen_error("syntax error in expression");
 exit(EXIT_FAILURE);
}

