/*
* Keen compiler 
* Copyright (C) 2003 Karim Ben Djedidia <kabend@free.fr> 
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

#include <assert.h>
#include "xmalloc.h"
#include "frame.h"
#include "ir.h"
#include "type.h"
#include "reg.h"
#include "temp.h"
#include "list.h"
#include "mode.h"
#include "error.h"

#define WORDSIZE	4
#define STACK_ALIGN	WORDSIZE

//
// FRAME STRUCTURE
//
struct xframe
{

  /* label of the function */
  label *l;

  /* label of the epilogue */
  label *l_epilogue;

  /* list of formals (parameters) */
  accessorList formals;

  /* list of locals (variables) */
  accessorList locals;

  /* holds current offset from frame pointer for locals */
  int fp_loc_offset;

  /* holds current offset from frame pointer for formals */
  int fp_for_offset;

  /* points to stack allocation instruction for the frame
   * this instruction needs to be rewritten after spilling in register
   * allocation phase
   */
  irnode *stack_insn;

  /* list of temps used to store callee-save registers
   */
  tempList callee_save_temps;

};

accessorList
get_formals(frame *f)
{
  return f->formals;
}

accessorList
get_locals(frame *f)
{
  return f->locals;
}

tempList
get_callee_save( frame *f )
{
  return f->callee_save_temps;
}

void
set_frame_stack_hook( frame *f, irnode *i )
{
 f->stack_insn = i;
}

enum memkind
{
  mem_data,			/* data is accessed as MEM( NAME ( label ) ) */
  mem_variable			/* variable is accessed as LABEL */
};

//
// ACCESSOR STRUCTURE
//
struct xaccessor
{
  enum
  { inFrame, inReg, inMem, inData } kind;
  int mode;
  union
  {
    struct
    {
      temp *reg;
    } inReg;
    struct
    {
      int fp_offset;
      int size;
    } inFrame;
    struct
    {
      enum memkind kind;
      label *l;
      int size;
    } inMem;
  } u;
};


/*
 * return required alignment on stack (i.e. wordsize)
 */
int
align_on_stack (size)
{
  if (size % STACK_ALIGN == 0)
    return size;
  return (((size / STACK_ALIGN) + 1) * STACK_ALIGN);
}

accessor *
new_accessor_inFrame (int mode, int fp_offset, int size)
{
  accessor *a = (accessor *) xmalloc (sizeof (accessor));
  a->kind = inFrame;
  a->mode = mode;
  a->u.inFrame.fp_offset = fp_offset;
  a->u.inFrame.size = size;
  return a;
}

accessor *
new_accessor_inReg (int mode, temp * reg)
{
  accessor *a = (accessor *) xmalloc (sizeof (accessor));
  a->kind = inReg;
  a->mode = mode;
  a->u.inReg.reg = reg;
  return a;
}

accessor *
new_accessor_inMem (int kind, int mode, label * l, int size)
{
  accessor *a = (accessor *) xmalloc (sizeof (accessor));
  a->kind = inMem;
  a->mode = mode;
  a->u.inMem.kind = kind;
  a->u.inMem.l = l;
  a->u.inMem.size = size;
  return a;
}

frame *
new_frame (label * entry, label * epilogue)
{
  frame *f = (frame *) xmalloc (sizeof (frame));
  f->l = entry;
  f->l_epilogue = epilogue;
  f->fp_loc_offset = 0;
  f->fp_for_offset = 2 * WORDSIZE;
  f->formals = newList ();
  f->locals = newList ();
  f->callee_save_temps = newList();
  return f;
}

accessor *
new_param (int mode, frame * f, int size)
{
  accessor *a = new_accessor_inFrame (mode, f->fp_for_offset, size);
  f->fp_for_offset += align_on_stack (size);
  f->formals = add (f->formals, a);
  return a;
}

accessor *
new_var (int mode, frame * f, int escape, int size)
{
  accessor *a;
  if (escape)
    {
      f->fp_loc_offset -= align_on_stack (size);
      a = new_accessor_inFrame (mode, f->fp_loc_offset, size);
    }
  else
    {
      a = new_accessor_inReg (mode, new_temp ());
    }
  f->locals = add (f->locals, a);
  return a;
}

accessor *
new_spill (int mode, frame * f)
{
  return new_var (mode, f, 1, STACK_ALIGN);
}

accessor *
new_global_var (int mode, label * l, int size)
{
  return new_accessor_inMem (mem_variable, mode, l, size);
}

accessor *
new_data (int mode, label * l, int size)
{
  return new_accessor_inMem (mem_data, mode, l, size);
}

irnode *
getAddress (accessor * a)
{
  switch (a->kind)
    {
    case inFrame:
      return nADD (nTEMP (FP ()), nCONST (a->u.inFrame.fp_offset));
    case inMem:
      return nNAME (a->u.inMem.l);
    case inReg:
      assert (0);
    }
}

irnode *
getMemoryExp (accessor * a)
{
  switch (a->kind)
    {
    case inFrame:
      return nMEMm (a->mode, getAddress (a));
    case inReg:
      return nTEMPm (a->mode, a->u.inReg.reg);
    case inMem:
      switch (a->u.inMem.kind)
	{
	case mem_variable:
	  return nMEMm( a->mode, nMEM (getAddress (a)));
	case mem_data:
	  return nMEMm (a->mode, getAddress (a));
	}
    }
}

label *
get_frame_label (frame * f)
{
  return f->l;
}

label *
get_frame_label_epilogue (frame * f)
{
  return f->l_epilogue;
}

label *
get_access_label (accessor * a)
{
  return a->u.inMem.l;
}

int
get_frame_size (frame * f)
{
  return f->fp_loc_offset;
}

int
get_type_natural_mode(type * t)
{
  switch( t->kind )
    {
      case t_char:
      case t_void: 	
        return m8;

      case t_short: 
	return m16;

      case t_int: 	
      case t_long:
      case t_pointer:
      case t_struct:
      case t_union:
      case t_func:
         return m32;		
      }
   // should not get here...
   assert(0);
}

int
get_type_size(type * t)
{
  switch( t->kind )
    {
      case t_char:
      case t_void: 	
        return 1;

      case t_short: 
	return 2;

      case t_int: 	
      case t_long:
      case t_func:
         return 4;		

      case t_struct:
      case t_union:

	 if (t->su_size != undefined) 
	  {
	    return t->su_size;
	  }
	 else
	  {
            memberList mlist = t->u.members;
            int total_size = 0;
            ListElem e, f; member *m;

	    /* UNION */
            if (t->kind == t_union) {

             e = get_first(mlist);
             while(e)
              {
                m = object(e);

		/* compute member size. Is the union total size if max. */
                if (get_aligned_type_size(m->t) > total_size) 
		    total_size = get_aligned_type_size(m->t);  

                e = get_next (e);
              } 

	      /* compute all paddings accordingly */
              e = get_first(mlist);
              while(e)
               {
		 m = object (e);
		 m->padding = total_size - get_type_size(m->t);
                 e = get_next (e);
	       }
	      
            }

	   /* STRUCT */
           if (t->kind == t_struct) 
            {
             e = get_last(mlist);
             m = object(e);
	
	     /* get total size */
             total_size = m->offset + get_aligned_type_size( m->t );

	     /* compute padding of all members */
	     e = get_first(mlist);
	     while (e)
              {

                m = object(e);
	        f = get_prev (e);
		if (f) 
  		  {
		    member *previous = object (f);
		    previous->padding = m->offset - (previous->offset + get_type_size(previous->t));
		  }
		if (get_next(e) == NULL) /* last member */
		  m->padding = total_size - (m->offset + get_type_size(m->t));

		e = get_next(e);
	      }
            }

	   t->su_size = total_size;
           return total_size; 

	  }
 
      case t_pointer:
         if (is_array(t) && t->array_size != undefined) return get_type_size( t->u.pointer_on ) * t->array_size;
         return 4;
      }
   // should not get here...
   assert(0);
}

int
get_aligned_type_size(type *t)
{
 return align_on_stack( get_type_size( t ));
}

char *
prologue_label_name (char *name)
{
  return name;
}

char *
epilogue_label_name (char *name)
{
  return stringf ("%s__%s_epilogue__", ASM_LABELBASE, name);
}

irnode *
generate_prologue (frame * f, irnode * body)
{
  return nSEQ (nGLOBAL (f),
	       nSEQ (nLABEL
		     (new_named_label
		      (stringf ("%s", get_label_name (get_frame_label (f))))),
		     nSEQ (nPROLOGUE (f), body)));
}

irnode *
generate_epilogue (frame * f)
{
  return nSEQ (nLABEL (get_frame_label_epilogue (f)), nEPILOGUE (f));
}

irnode *
generate_fetch (accessor * a , temp * t )
{
 assert (a->kind == inFrame);
 return nINSN_OPER (stringf ("movl %d(%%s0), %%d0", a->u.inFrame.fp_offset), 
                    add (newList (), t),                       /* dest list */
                    add (newList (), new_named_temp("ebp")),   /* src list */
                    newList (),                                /* jump list */
                    nNOP(),
                    -1 );
}

irnode *
generate_store (accessor * a , temp * t )
{
 assert (a->kind == inFrame);
 List src = newList();
 add(src, t);
 add(src, new_named_temp("ebp"));
 return nINSN_OPER (stringf ("movl %%s0, %d(%%s1)", a->u.inFrame.fp_offset), 
                    newList (),                                /* dest list */
                    src,                                       /* src list */
                    newList (),                                /* jump list */
                    nNOP(),
                    -1 );
}

void
update_prologue(frame *f)
{
  assert(f->stack_insn);
  char *new_insn_string = stringf("addl $%d, %%s0", get_frame_size(f) );
  SET_IRCHILD( f->stack_insn, 0, (void *) new_insn_string );
}

void
print_frame (frame *f)
{
 keen_debug ("FRAME DETAILS FOR FUNCTION '%s':\n", get_label_name(f->l)); 
 keen_debug ("> PARAMETERS:\n"); 
 accessor *a;
 FOR_EACH (a,f->formals,e1) keen_debug(">  %%ebp %+d to %+d: size %d\n", a->u.inFrame.fp_offset, 
                                                                        a->u.inFrame.fp_offset + a->u.inFrame.size, 
                                                                        a->u.inFrame.size );
 keen_debug ("> VARIABLES:\n"); 
 FOR_EACH (a,f->locals,e2) keen_debug(">  %%ebp %+d to %+d: size %d\n", a->u.inFrame.fp_offset, 
                                                                        a->u.inFrame.fp_offset + a->u.inFrame.size, 
                                                                        a->u.inFrame.size );
}


