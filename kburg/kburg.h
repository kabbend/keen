
#ifndef _KBURG_H_
#define _KBURG_H_

#include "list.h"

typedef List tList;        /* temporaries-list: List of arg */
typedef List clauseList;
typedef List bodyList;

typedef struct xbody
{
  enum { body_template, body_functioncall, body_multi, body_internal } kind;
  char *insn_type;
  tList dest_list;
  tList src_list;
  tList label_list;
  char *template;
} body;

typedef struct xarg
{
  enum { arg_register, arg_functioncall, arg_variable } kind;
  char *var;
  char *value;
} arg;

typedef struct xrule
{
  int id;
  struct xclause *c;
  bodyList bodies;
} rule;

typedef struct xknode 
{ 
  char *node;
  char *mode;
} knode;

typedef struct xclause
{
  knode *node;
  int has_value;
  int value;
  clauseList child;
} clause;

rule *new_rule (clause * c, bodyList bodies);
body *new_body_string (char *nodetype, tList d, tList s, tList l, char *template);
body *new_body_fc (char *nodetype, tList d, tList s, tList l, char *fc);
body *new_body_internal (tList d, tList s, tList l);
body *new_body_multi (char *fc);
knode *new_node (char *node, char *mode);
arg *new_arg_reg (char *var, char *v);
arg *new_arg_fc (char *var, char *v);
arg *new_arg_var (char *var, char *v);
clause *new_clause (knode *node, int has_value, int value, clauseList children);

int yyparse (void);

#endif
