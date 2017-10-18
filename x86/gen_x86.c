/*
* Keen compiler 
* Copyright (C) 2006 Karim Ben Djedidia <kabend@free.fr> 
*
* This program is free software{ } you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation{ } either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY{ } without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program{ } if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  
* USA
*
*/

#include "ir.h"
#include "insn.h"
#include "temp.h"
#include "frame.h"
#include "list.h"
#include "mode.h"
#include "xmalloc.h"
#include "asm.h"
#include "reg.h"
#include <assert.h>

temp *nopt(irnode *node /* ignored */) { temp *t = new_temp_mode( DEFAULT_MODE ); t->is_kburg_product = 1; return t; }
temp *n(irnode *node /* ignored */) { return new_temp_mode(DEFAULT_MODE); }
temp *n1B(irnode *node /* ignored */) { return new_temp_mode(m8); }
temp *n2B(irnode *node /* ignored */) { return new_temp_mode(m16); }
temp *n4B(irnode *node /* ignored */) { return new_temp_mode(m32); }

irnodeList
x86_gen_frame_stack (irnode * node, int rule_id)
{
 irnodeList ret = newList();
 frame  *f = (frame *) GET_IRCHILD (node, 0);
 int size = get_frame_size( f );
 irnode *insn =  nINSN_OPER (   stringf ("addl $%d, %%s0", size ), 
                                newList (), 	/* dest list */
				add (newList (), new_named_temp ("esp")),	/* src list */
				newList (),	/* jump list */
				nNOP(),
				-1);
 add(ret,insn);
 set_frame_stack_hook( f, insn );
 return ret;
}

char *
x86_gen_func_globl (irnode * node)
{
  return stringf (".globl %s",
		  get_label_name (get_frame_label
				  ((frame *) GET_IRCHILD (node, 0))));
}

char *
x86_gen_get_data (irnode * node)
{
  return stringf ("movl $%s, %%d0",
		  get_label_name ((label *)
				  GET_IRCHILD (GET_IRCHILD (node, 0), 0)));
}

char *
x86_gen_scalar_data_entry ( data *d )
{

  char *value = d->value;
  if (!value) value = string("0");
  char *padstring = string("");

  switch (d->mode)
        {
          case m8:
	    if (d->padding) padstring = stringf("\n\t.zero %d", d->padding); 
            return stringf (".byte %s%s", value, padstring);
          case m16:
	    if (d->padding) padstring = stringf("\n\t.zero %d", d->padding); 
            return stringf (".word %s%s", value, padstring);
          case m32:
	    if (d->padding) padstring = stringf("\n\t.zero %d", d->padding); 
            return stringf (".int %s%s", value, padstring);
          case STR_mode:
            return stringf (".string \"%s\"", value);
          default:
            abort ();
        }
             
}

char *
x86_gen_data_entry (irnode * node)
{

  int is_scalar = *(int *) GET_IRCHILD (node, 0);
  int is_initialized = *(int *) GET_IRCHILD (node, 1);
  int total_size = *(int *) GET_IRCHILD (node, 2);
  label *l = (label *) GET_IRCHILD (node, 3);
  dataList dl = (dataList) GET_IRCHILD (node, 4);

  if (is_scalar)
    {
      // SCALAR
      assert (size(dl)==1);
      data *d = object (get_first(dl));
      return stringf("%s:\n\t%s",get_label_name(l), x86_gen_scalar_data_entry ( d )); 
    }
  else
    {
      // AGGREGATE 
      if (!is_initialized)
        {
          return stringf("\t.comm %s,%d", get_label_name(l), total_size); 
        }
      else
        {
          ListElem e = get_first(dl);
          data *d = object (e);
          char *ret = stringf("%s:\n\t%s", get_label_name(l), x86_gen_scalar_data_entry(d));
          e = get_next (e);
          while (e)
            {
              d = object (e);
              ret = stringf("%s\n\t%s",ret,x86_gen_scalar_data_entry(d)); 
              e = get_next (e);
            } 
          return ret;
        }
    }

}

char *
x86_gen_data_section (irnode * node)
{
  // FIXME ! this is not always .data
  // return string(".section .rodata");
  return string (".data");
}

irnodeList
x86_gen_call (irnode * node, int rule_id)
{

  tempList args_temps = newList ();

  // prepare push & pop arg instructions 
  irnodeList argl = (irnodeList) GET_IRCHILD (GET_IRCHILD (node, 0), 1);
  int pop_size = 0;
  irnodeList ins = newList();
  ListElem args = get_first (argl);
  while (args)
    {
      // get type of the argument
      irnode *arg = object(args);
      type *actual_type = GET_TYPE (arg);

      // generate instruction for each expression
      irnode *arg_insn = genInsn (ins, arg);
      tempList t = (tempList) GET_IRCHILD (arg_insn, 1);
      
      // In general a push instruction is produced for each argument
      // except if argument is a structure, in which case a copy on stack
      // of the entire structure is necessary

      if ( !is_struct_or_union( actual_type ) )
        {
          // generate push
          add( ins, nINSN_OPER (string ("pushl %s0"), newList (),	/* dest list */
					t,				/* src list */
					newList (),			/* jump list */
					node,
					rule_id));
          // prepare pop
          pop_size += 4;
        }
      else
        {
          // is a struct or union, need to copy on stack

	  // source
          tempList tl = (tempList) GET_IRCHILD ( arg_insn, 1 );
	  assert (size(tl) == 1);
          temp *src = object ( get_first ( tl ) );

	  // dest
          temp *dest = new_named_temp( "esp" ); // dest is the stack

          // we need to adjust stack pointer accordingly
          add( ins, nINSN_OPER (stringf ("addl $-%d, %%s0", get_type_size(actual_type)), 
               newList(), add (newList (), new_named_temp("esp")), newList(), node, rule_id ) );

          int size_in_word = align_on_stack( get_type_size(actual_type)) / 4;
	  // generate copy operation
          add( ins, nINSN_OPER (string ("# copy on stack"),  newList(), newList (),  newList(), node, rule_id ) );
          add( ins, nINSN_MOVE (string ("movl %s0, %d0"),  add (newList(), new_named_temp("edi")),
                                                             add (newList(), dest), 
                                                             newList(), node, rule_id ) );
          add( ins, nINSN_MOVE (string ("movl %s0, %d0"),  add (newList(), new_named_temp("esi")),
                                                             add (newList(), src), 
                                                             newList(), node, rule_id ) );
          add( ins, nINSN_OPER (stringf("movl $%d, %%d0", size_in_word), add (newList(), new_named_temp("ecx")),
                                                             newList(), newList(), node, rule_id ) );
          add( ins, nINSN_OPER (string ("rep movsl"),  add( add( add( newList(), new_named_temp("edi")) , new_named_temp("esi")), new_named_temp("ecx")),
          					       add( add( add( newList(), new_named_temp("edi")) , new_named_temp("esi")), new_named_temp("ecx")),
                                                       newList(), node, rule_id ) );


          // prepare pop
          pop_size += get_type_size (actual_type);
        }

      // go next argument
      args = get_next (args); 
    }

  List dests = newList();
  int i;
  for(i=REG_FIRST_CALLER;i<=REG_LAST_CALLER;i++)
    {
      add( dests, new_named_temp( GET_REG_NAME(i) ) );
    }

  // insert actual call
  add (ins, nINSN_OPER (stringf ("call %s", get_label_name ((label *) GET_IRCHILD (GET_IRCHILD (GET_IRCHILD (node, 0), 0), 0))), 					   dests,	/* dest list */
			    args_temps,	/* src list */
			    newList (),	/* jump list */
			    node,
			    rule_id));

  // insert stack cleanup instruction
  add( ins, nINSN_OPER (stringf ("addl $%d, %%s0", pop_size), 
					newList (),	/* dest list */
                              add( newList(), new_named_temp("esp")),	/* src list */
					newList (),	/* jump list */
					node,
					rule_id));

  return ins;
}

irnodeList
x86_gen_call_assign (irnode * node, int rule_id)
{
  irnodeList l =
    x86_gen_call ( nEXPR (node) /* cast to be compliant with x86_gen_call */ ,
		       rule_id);
  add (l, nINSN_MOVE (string ("movl %s0, %d0"), add (newList (), nopt(NULL)),	/* dest list */
				add (newList (), new_named_temp ("eax")),	/* src list */
				newList (),	/* jump list */
				node,
				rule_id));
  return l;
}

temp *
x86_gen_temp (irnode * node)
{
  return (temp *) GET_IRCHILD (node, 0);
}

label *
x86_gen_label (irnode * node)
{
  return (label *) GET_IRCHILD (node, 0);
}

temp *
x86_move_mem_temp (irnode * node)
{
  return (temp *) GET_IRCHILD (GET_IRCHILD (GET_IRCHILD (node, 0), 0), 0);
}

temp *
x86_move_temp (irnode * node)
{
  return (temp *) (GET_IRCHILD (GET_IRCHILD (node, 0), 0) );
}

label *
x86_gen_jump_unique_name (irnode * node)
{
  return (label *) GET_IRCHILD (node, 1);
}

label *
x86_get_false_label (irnode * node)
{
  return (label *) GET_IRCHILD (node, 2);
}

label *
x86_get_true_label (irnode * node)
{
  return (label *) GET_IRCHILD (node, 1);
}

irnodeList
x86_set_callee_save(irnode *node, int rule)
{
 irnodeList ret = newList();
 frame *f = (frame *) GET_IRCHILD( node , 0);
 int i;
 for(i=REG_FIRST_CALLEE;i<=REG_LAST_CALLEE;i++)
  {
    temp *d = n(NULL);
    temp *s = get_temp(i);
    add ( ret, nINSN_MOVE ( string ("movl %s0, %d0"), 
                    add( newList (), d ),                      /* dest list */
                    add( newList (), s ),                      /* src list */
                    newList (),                                /* jump list */
                    nNOP(),
                    rule ) );
    /* store this temp into frame. Will be used in epilogue */
    add ( (tempList) get_callee_save(f), d );
  }
 return ret;
}

irnodeList
x86_get_callee_save(irnode *node, int rule)
{
 irnodeList ret = newList();
 frame *f = (frame *) GET_IRCHILD( node , 0);
 ListElem e = get_last ( (tempList) get_callee_save(f));
 int i;
 for(i=REG_LAST_CALLEE;i>=REG_FIRST_CALLEE;i--)
  {
    temp *s = (temp *) object (e);
    temp *d = get_temp(i);
    add ( ret, nINSN_MOVE ( string ("movl %s0, %d0"), 
                    add( newList (), d ),                      /* dest list */
                    add( newList (), s ),                      /* src list */
                    newList (),                                /* jump list */
                    nNOP(),
                    rule ) );
    e = get_prev(e);
  }
 return ret;
}

char *
x86_gen_info(irnode *node)
{
  char *file = (char *) GET_IRCHILD( node, 0);
  char *keen_version = (char *) GET_IRCHILD( node, 1);
  return stringf("\t.file \"%s\"\n\t.ident \"%s\"", file, keen_version );
}

