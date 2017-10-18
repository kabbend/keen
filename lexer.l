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
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  
* USA
*
*/

D	[0-9]
L	[a-zA-Z_]
H	[a-fA-F0-9]
B	[01]
O	[0-7]

%{
#include "xmalloc.h"		
#include "grammar.h"		// for grammar function prototypes
#include "y.tab.h"		// for grammar tokens
#include "symbol.h"
#include "bind.h"
#include "error.h"

extern hashtable *env;

YYSTYPE yylval;

void count();
int check_constant();
int return_identifier_or_typename();
int return_keyword();
void preprocess_info();
int fileno();

enum {
    CONSTANT_BINARY,
    CONSTANT_OCTAL,
    CONSTANT_DECIMAL,
    CONSTANT_HEXA
};

%}

%%

^"#"			{ preprocess_info(); }

"{"			{ count();  return return_keyword(OPEN_BRACKET); }
"}"			{ count();  return return_keyword(CLOSE_BRACKET); }
"("                     { count();  return return_keyword(OPEN_PAREN); }
")"                     { count();  return return_keyword(CLOSE_PAREN); }
";"                     { count();  return return_keyword(SEMICOLON); }
","                     { count();  return return_keyword(COMMA); }
":"                     { count();  return return_keyword(COLON); }
"."			{ count();  return return_keyword(DOT); }
"..."			{ count();  return return_keyword(ELLIPSIS); }
"->"			{ count();  return return_keyword(ARROW); }
"["			{ count();  return return_keyword(OPEN_ARRAY); }
"]"			{ count();  return return_keyword(CLOSE_ARRAY); }
"~"			{ count();  return return_keyword(TILDE); }
"++"			{ count();  return return_keyword(PPLUS); }
"+"			{ count();  return return_keyword(PLUS); }
"--"			{ count();  return return_keyword(MMINUS); }
"-"                     { count();  return return_keyword(MINUS); }
">="                    { count();  return return_keyword(GE); }
"<="                    { count();  return return_keyword(LE); }
"<<"                    { count();  return return_keyword(SHIFTL); }
">>"                    { count();  return return_keyword(SHIFTR); }
"<"                     { count();  return return_keyword(LT); }
">"                     { count();  return return_keyword(GT); }
"=="                    { count();  return return_keyword(EQ); }
"="                     { count();  return return_keyword(ASSIGN); }
"+="                    { count();  return return_keyword(ASSIGNPLUS); }
"-="                    { count();  return return_keyword(ASSIGNMINUS); }
"*="                    { count();  return return_keyword(ASSIGNMUL); }
"/="                    { count();  return return_keyword(ASSIGNDIV); }
"&="                    { count();  return return_keyword(ASSIGNBAND); }
"|="                    { count();  return return_keyword(ASSIGNBOR); }
"^="                    { count();  return return_keyword(ASSIGNBXOR); }
"%="                    { count();  return return_keyword(ASSIGNMOD); }
"!="                    { count();  return return_keyword(NE); }
"*"                     { count();  return return_keyword(MUL); }
"/"                     { count();  return return_keyword(DIV); }
"%"                     { count();  return return_keyword(MOD); }
"^"                     { count();  return return_keyword(BXOR); }
"&"                     { count();  return return_keyword(BAND); }
"||"                    { count();  return return_keyword(OR); }
"|"                     { count();  return return_keyword(BIOR); }
"&&"                    { count();  return return_keyword(AND); }
"!"                     { count();  return return_keyword(NOT); }
"?"                     { count();  return return_keyword(QUESTIONMARK); }

"extern"                { count();  return return_keyword(EXTERN); }
"static"                { count();  return return_keyword(STATIC); }
"auto"                  { count();  return return_keyword(AUTO); }
"register"              { count();  return return_keyword(REGISTER); }
"typedef"               { count();  return return_keyword(TYPEDEF); }
"sizeof"                { count();  return return_keyword(SIZEOF); }

"const"                 { count();  return return_keyword(CONST); }
"volatile"              { count();  return return_keyword(VOLATILE); }
"restrict"              { count();  return return_keyword(RESTRICT); }

"if"                    { count();  return return_keyword(IF); }
"else"                  { count();  return return_keyword(ELSE); }
"switch"                { count();  return return_keyword(SWITCH); }
"case"                  { count();  return return_keyword(CASE); }
"default"               { count();  return return_keyword(DEFAULT); }
"goto"                  { count();  return return_keyword(GOTO); }

"for"                   { count();  return return_keyword(FOR); }
"while"                 { count();  return return_keyword(WHILE); }
"do"                    { count();  return return_keyword(DO); }

"return"                { count();  return return_keyword(RETURN); }
"break"                	{ count();  return return_keyword(BREAK); }
"continue"              { count();  return return_keyword(CONTINUE); }

"struct"                { count();  return return_keyword(STRUCT); }
"union"                 { count();  return return_keyword(UNION); }
"enum"                  { count();  return return_keyword(ENUM); }

{L}({L}|{D})*		{ count();  return return_identifier_or_typename(); }

"0x"{H}+		{ count();  return check_constant(CONSTANT_HEXA); }
"0b"{B}+		{ count();  return check_constant(CONSTANT_BINARY); }
{D}+			{ count();  return check_constant(CONSTANT_DECIMAL); }

"\""([^"]*("\\\"")?[^"]*)*"\""		{ count(); return return_string(); }
'\\.'			{ count(); return return_character_sequence(); }
'.'			{ count(); return return_character_sequence(); }

[ \r\t\v\n\f]		{ count(); }
.			{ keen_error("unexpected token '%s'",yytext); exit(EXIT_FAILURE);}

%%

void preprocess_info()
{
  // A line beginning with "#" is written by preprocessor and gives
  // information on line/column/filename currently parsed
  char buffer[256];
  int idx=0;
  char c;
  idx=0;
  while ( (c = input()) != ',') buffer[idx++] = c;
  buffer[idx]='\0';
  SET_LINE( atol(buffer) );
  idx=0;
  while ( (c = input()) != '\n') buffer[idx++] = c;
  buffer[idx]='\0';
  SET_FILE( string( buffer ) );
  SET_COLUMN(1);
}

void count()
{

	int i;
	for (i = 0; yytext[i] != '\0'; i++)
	  {
	    if (yytext[i] == '\n') { SET_COLUMN(1); INC_LINE(); continue; }
  	    if (yytext[i] == '\t') { SET_COLUMN( GET_COLUMN() + 8 - (GET_COLUMN() % 8)); continue; } 
	    INC_COLUMN();
	  }

}

int return_keyword(int k)
{
  yylval.keyword.v = k;
  yylval.keyword.pos.line = GET_LINE();
  yylval.keyword.pos.column = GET_COLUMN();
  yylval.keyword.pos.filename = string( GET_FILE() );
  return k;
}

int return_identifier_or_typename()
{
  int r;
  yylval.id.v = string(yytext);
  yylval.id.pos.line = GET_LINE();
  yylval.id.pos.column = GET_COLUMN();
  yylval.id.pos.filename = string( GET_FILE() );
  if ( (lookup_symbol(env, yytext) != NULL) && (lookup_symbol(env,yytext)->binding->kind == b_type)) {
   r = TYPENAME;
  } else {
   r = IDENTIFIER;
  }
  return r;
}

int return_character_sequence() {
  
  yylval.c.pos.line = GET_LINE();
  yylval.c.pos.column = GET_COLUMN();
  yylval.c.pos.filename = string( GET_FILE() );
  char *sequence = string( yytext );
  char *s = sequence+1;            // remove heading quote '
  s[strlen(s)-1] = 0;              // remove trailing quote '
  if (strlen(s)==0) 
    {
      keen_error("invalid empty character sequence");
    }
  if (s[0]=='\\')
    {
      // ESCAPE SEQUENCE
      s++;
      if (strlen(s)!=1)
        {
          keen_error("invalid character sequence");
        }
      switch(s[0])
        {
          case '0': yylval.c.v = 0; break;
          case 't': yylval.c.v = '\t'; break;
          case 'n': yylval.c.v = '\n'; break;
          case 'a': yylval.c.v = '\a'; break;
          case 'b': yylval.c.v = '\b'; break;
          case 'v': yylval.c.v = '\v'; break;
          case 'f': yylval.c.v = '\f'; break;
          case 'r': yylval.c.v = '\r'; break;
          case '\\': yylval.c.v = '\\'; break;
          case '\'': yylval.c.v = '\''; break;
          case '\"': yylval.c.v = '\"'; break;
          default:
            keen_error("invalid character escape-sequence");
        }
    }
  else 
    {
      // STANDARD SEQUENCE
      if (strlen(s)!=1)
        {
          keen_error("invalid character sequence");
        }
      yylval.c.v = s[0];
    }
  free(sequence);
  return CONSTANT_CHAR;
}

int return_string() {
  
  yylval.s.pos.line = GET_LINE();
  yylval.s.pos.column = GET_COLUMN();
  yylval.s.pos.filename = string( GET_FILE() );
  char *s = string( yytext );
  s[ strlen(s) - 1 ] = 0; // remove last " char
  yylval.s.v = s+1;	  // remove first " char
  return CONSTANT_STRING;
}

int check_constant(int literalType)
{
  int type;
  long long int llvalue;
  char *on_error;
  int base;

  /* store exact type and value */
  base = 0;
  switch(literalType) {
    case CONSTANT_BINARY:
      base = 2;
    case CONSTANT_OCTAL:
      if (base == 0) base = 8;
    case CONSTANT_DECIMAL:
      if (base == 0) base = 10;
    case CONSTANT_HEXA:
      if (base == 0) base = 16;
      llvalue = strtoll(yytext,&on_error,base);
      yylval.i.v = (signed long int) llvalue;
      break;
  }
  yylval.i.pos.line = GET_LINE();
  yylval.i.pos.column = GET_COLUMN();
  yylval.i.pos.filename = string( GET_FILE() );
  return CONSTANT;
}

int yywrap(void) {
  SET_LINE(0);
  SET_COLUMN(1);
  return 1;
} 


