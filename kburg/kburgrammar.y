%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "kburg.h"
#include "list.h"
#include "xmalloc.h"
#include "assert.h"

#define isdigit(c) ((c)>='0' && (c)<='9')
#define isalpha(c) (((c)>='a' && (c)<='z')||((c)>='A' && (c)<='Z')||((c)=='_'))

int yylineno = 0;

%}

%union {
 char *s;
 clause *c;
 rule *r;
 body *b;
 knode *n;
 arg *a;
 int i;
 clauseList l;
 tList t;
 bodyList bl;
}

%token      INTERNAL 
%token <s>  STRING ID REGISTER VARIABLE MODE NODETYPE
%token <i>  INT
%type <n>  node 
%type <c>   clause
%type <l>   nodelist
%type <t>  arglist
%type <r>   rule	
%type <s>   nodetype functioncall
%type <t>   list
%type <b>   body
%type <bl>  body_list
%type <a>   arg

%%
spec  : rules					 
      ;

rules : rule rules
      | /* empty */
      ;

rule  : clause '{' body_list '}'    { new_rule( $1, $3 ); }
      ;

body_list: body                     { $$ = add( newList(), $1 ); }
         | body body_list           { $$ = join( add( newList(), $1 ), $2 ); }
         ;
	
body  : nodetype list list list STRING ';'            { $$ = new_body_string( $1, $2, $3, $4, $5 ); }
      | nodetype list list list functioncall ';'      { $$ = new_body_fc( $1, $2, $3, $4, $5 ); }
      | INTERNAL list list list ';'                   { $$ = new_body_internal( $2, $3, $4 ); }
      | functioncall ';'                              { $$ = new_body_multi( $1 ); }
      ;

functioncall: ID '(' ')'    { $$ = string( $1 ); }
      ;

nodetype: NODETYPE          { $$ = string( $1 ); }
      ;

list  : '(' arglist ')'      { $$ = $2; }
      | '(' ')'              { $$ = NULL; } 
      ;

arglist : arg                        { $$ = add( newList(), $1 ); }
        | arg ',' arglist            { $$ = join( add( newList(), $1 ), $3 ); }
        ;

arg : VARIABLE '=' functioncall      { $$ = new_arg_fc( $1, $3 ); }
    | VARIABLE '=' REGISTER          { $$ = new_arg_reg( $1, $3 ); }
    | VARIABLE '=' VARIABLE          { $$ = new_arg_var( $1, $3 ); }
    | VARIABLE                       { $$ = new_arg_var( NULL, $1 ); }
    | functioncall                   { $$ = new_arg_fc( NULL, $1 ); }
    | REGISTER                       { $$ = new_arg_reg( NULL, $1 ); }
    ;

clause  : node                   { $$ = new_clause( $1, 0 /* no value */, 0 , NULL ); }
        | node '(' nodelist ')'  { $$ = new_clause( $1, 0 /* no value */, 0 , $3 ); }
        | error                  { fprintf(stderr,"kburg: line %d: not a valid clause\n",yylineno); abort(); }
        ;

nodelist: clause                   { $$ = add( newList(), $1 ); }
        | clause ',' nodelist      { $$ = join( add( newList() , $1 ) , $3 ); }
        | node INT                 { $$ = add( newList() , new_clause( $1, 1, $2, NULL ) ); }
        | node INT ',' nodelist    { $$ = join( add( newList() , new_clause( $1, 1, $2, NULL ) ) , $4 ); }
        ;

node: ID                           { $$ = new_node( $1, NULL ); }   
    | ID ':' MODE                  { $$ = new_node( $1, $3 ); }
    | VARIABLE                     { $$ = new_node( $1, NULL ); }
    ;

%%

static char buf[1024], *bp = buf;
extern FILE *file_in;
extern FILE *file_out;

static int 
get(void) 
{

 int in_comment = 0;

 start:

 /* fill in buffer if empty */
 if (in_comment || *bp == 0) 
   {
      bp = buf;
      *bp = 0;
      if (fgets(buf, sizeof buf, file_in) == NULL) 
        {
          if (in_comment)
            {
              fprintf(stderr,"kburg: line %d: comment not closed properly\n",yylineno); 
              abort();
            } 
          return EOF;
        }
      yylineno++;
   }

 /* eat C-style comments */
 if (in_comment) 
   {

 eat:
 
     while (*bp && *bp != '*') bp++;
     if (*bp==0) 
       {
         // need to read another line
         goto start;
       }
     else
       {
         bp++;
         if (*bp=='/')
           {
             bp++;
             in_comment = 0;
             return ' ';
           }
         else
           {
             goto eat;
           }
       }
   }

 if (*bp == '/')
   {
     bp++;
     if (*bp == '*')
       {
         // this is a comment...
         bp++;
         in_comment = 1;
         goto eat;
       }
     else
       {
         // rewind
         bp--;
       }
   }

 return *bp++;
}


int 
yylex(void) 
{
  int c;
  while ((c = get()) != EOF) {

  switch (c) {
      case '\n': case ' ': 
      case '\f': case '\t': 
        continue;
      case '(': case ')': case ',':
      case ':': case '{': case '}':
      case ';': case '=':
        return c;
  }

  // STRING 
  if (c == '"') {
	char *p = strchr(bp, '"');
	if (p == NULL) {
	  keen_error("missing closing \" in string");
	}
	yylval.s = (char *) xmalloc(p - bp + 1);
	if (p-bp) strncpy(yylval.s, bp, p - bp);
	yylval.s[p - bp] = 0;
	bp = p + 1;
	return STRING;
   }  

   // VARIABLE
   if (c=='$') {
	char *p = bp - 1;
	while ( isalpha(*bp) || isdigit(*bp) ) bp++;
	yylval.s = (char *) xmalloc(bp - p + 1);
	strncpy(yylval.s, p, bp - p);
	yylval.s[bp - p] = 0;
	return VARIABLE;
   }  

   // REGISTER
   if (c=='%') {
	char *p = bp - 1;
	while ( isalpha(*bp) || isdigit(*bp) ) bp++;
	yylval.s = (char *) xmalloc(bp - p + 1);
	strncpy(yylval.s, p, bp - p);
	yylval.s[bp - p] = 0;
	return REGISTER;
   }  

   // ID, MODE, NODETYPE or INT
   if (c=='*') 
     {
	yylval.s = (char *) xmalloc(2);
	strcpy(yylval.s, "*");
       return ID;  // special node '*'
     }

   if (isalpha(c)) {
	char *p = bp - 1;
	while ( isalpha(*bp) || isdigit(*bp) ) bp++;
	yylval.s = (char *) xmalloc(bp - p + 1);
	strncpy(yylval.s, p, bp - p);
	yylval.s[bp - p] = 0;

      // check for INTERNAL keyword
      if (strcmp(yylval.s,"internal")==0) return INTERNAL;

      // check for NODETYPE
      if (strncmp(yylval.s,"INSN_",5)==0) return NODETYPE;

      // check for known modes
      if (strcmp(yylval.s,"CC")==0) return MODE;
      if (strcmp(yylval.s,"STR")==0) return MODE;
      if (strcmp(yylval.s,"BIT")==0) return MODE;
      if (strcmp(yylval.s,"SBY")==0) return MODE;
      if (strcmp(yylval.s,"TBY")==0) return MODE;
      if (strcmp(yylval.s,"FBY")==0) return MODE;
      if (strcmp(yylval.s,"m8")==0) return MODE;
      if (strcmp(yylval.s,"m16")==0) return MODE;
      if (strcmp(yylval.s,"m32")==0) return MODE;

      // else this is an ID     
      return ID;
   }  

   if (isdigit(c)) {
      int value;
      char *p = bp - 1;
      while ( isdigit(*bp) ) bp++;
      sscanf(p,"%d",&value);
      yylval.i = value;
      return INT;
   }

   // otherwise...
   keen_error("invalid character '%c'", c);

 }

 return 0;
}

int yyerror(char *msg) {
 return 1;
}

