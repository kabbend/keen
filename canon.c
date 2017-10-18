/*
 * Keen compiler 
 *
 * canon.c
 *
 * transform IR trees into canonical trees and basic blocks
 *
 * Copyright (C) 2005 Karim Ben Djedidia <kabend@free.fr> 
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

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "canon.h"
#include "ir.h"
#include "insn.h"
#include "temp.h"
#include "asm.h"
#include "error.h"
#include "list.h"

extern int option_bb_optimization_off;

#if (ASM_CJUMP_FALLTHROUGH_FALSE_LABEL == 0)
#define FALLTHROUGH	1
#define JUMPTO		2
#else
#define FALLTHROUGH	2
#define JUMPTO		1
#endif

/* fallthrough optimization pass 
 * remove any jump immediatly followed by its target label */
void
remove_fallthrough (trace tr)
{
  /* for all basic block lists in the trace... */
  ListElem t = get_first(tr);
  while(t)
    {
      BBList b = (BBList) object(t);

      /* for each block of the chained list (except last where no such optimization is possible) */
      ListElem u = get_first (b);
      while (u && u != get_last (b))
	{
        BB *currentBB = object (u);

	  /* get last stmt of this block. This is not necessarily a n_JUMP stmt */
	  ListElem last_instruction = get_last ( currentBB->stmtlist ); 
        irnode *last = (irnode *) object( last_instruction );

	  /* determine its target label, if applicable */
	  label *target_label;
	  switch (NODETYPE (last))
	    {
	    case n_JUMP:
	      target_label = (label *) GET_IRCHILD (last, 1);
	      break;
	    default:
	      /* nothing to optimize here. Go ahead */
	      u = get_next (u);
	      continue;
	    }
	  /* get label of next block */
        ListElem v = get_next (u);
	  label *next_label = (label *) GET_IRCHILD ( (irnode *) object( get_first( ((BB *) object (v))->stmtlist ) ), 0);
	  /* compare both labels */
	  if (same_label (target_label, next_label))
	    {
	      /* if same, remove useless last jump */
            /* this may lead to an empty stmtlist */
	      drop ( currentBB->stmtlist, last_instruction );
	    }
	  /* go ahead with next bblock */
	  u = get_next (u);
	}
      t = get_next (t);
    }
}

/* label optimization pass 
 * remove labels not referenced in any jump */
void
remove_useless_labels (trace t)
{

  /* 1st pass: count each label usage in the trace, i.e. 
     for each jump, increase target label reference count */
  ListElem tp = get_first (t);
  while (tp)
    {
      
      BBList b = (BBList) object (tp);
      ListElem bp = get_first (b);
      while (bp)
	{
	  BB *currentBB = (BB *) object (bp);
        ListElem p = get_first (currentBB->stmtlist);
	  while (p)
          {
	      irnode *i = object( p );
	      switch (NODETYPE (i))
		{
		case n_JUMP:
		  change_label_usage ( (label *) GET_IRCHILD (i, 1), +1);
		  break;
		case n_CJUMP:
		  change_label_usage (((label *) GET_IRCHILD (i, JUMPTO)), +1);
		  break;
		}
            p = get_next (p);
	    }
	  bp = get_next (bp);
	}
      tp = get_next (tp);
    }

  /* 2nd pass: remove each unused (ref count == 0) label */
  /* only internal labels are taken into account */
  tp = get_first (t);
  while (tp)
    {
      BBList b = (BBList) object (tp);
      ListElem bp = get_first (b);
      while (bp)
	{
	  BB *currentBB = (BB *) object (bp);
        ListElem p = get_first (currentBB->stmtlist);
	  if (p)
	    {
	      irnode *i = object (p);
	      if (NODETYPE (i) == n_LABEL)
		{
		  label *current_label = (label *) GET_IRCHILD (i, 0);
		  if (is_internal_label (current_label)
		      && label_usage (current_label) == 0)
		    {
		      /* remove the unused label */
		      drop (currentBB->stmtlist, p);
		    }
		}
	    }
	  bp = get_next (bp);
	}
      tp = get_next (tp);
    }

}

void
replace_label (trace t, label * x, label * y)
{
  ListElem tp = get_first (t);
  while (tp)
    {
      BBList b = (BBList) object (tp);
      ListElem bp = get_first (b);
      while (bp)
	{
        BB *currentBB = (BB *) object (bp);
	  ListElem p = get_first (currentBB->stmtlist);	 /* p points on first stmt */
	  while (p)
	    {
	      irnode *i = object (p);
	      label *target_label = NULL;
	      label *target_pt = NULL;
	      switch (NODETYPE (i))
		{
		case n_JUMP:
		  target_pt = (label *) GET_IRCHILD (i, 1);
		  if (same_label (target_pt, x))
		    {
		      //change (get_first (target_pt), y);
		      SET_IRCHILD( i, 1, (void *) y );
		    }
		  break;
		case n_CJUMP:
		  target_label = ((label *) GET_IRCHILD (i, JUMPTO));
		  if (same_label (target_label, x))
		    {
		      SET_IRCHILD (i, JUMPTO, (void *) y);
		    }
		  break;
		}
	      p = get_next (p);
	    }
	  bp = get_next (bp);
	}
      tp = get_next (tp);
    }
}

/* useless blocks optimization pass 
 * remove blocks containing immediatly one jump stmt and that are only
 * reachable by another jump ( = there is no fallthrough) */
void
remove_jump_only_blocks (trace t)
{

  ListElem tp = get_first (t);
  while (tp)  
    {
      BBList b = (BBList) object (tp);
      ListElem bp = get_first (b);
      while (bp)
	{
        
        int could_fallthrough = 1; // a priori. may be changed below

        BB *currentBB = object (bp);

        /* keep pointer on next block because we may drop current one and loose reference */
        ListElem next_block = get_next (bp);

        /* consider previous block. If no such block, then no possible fallthrough */
        label *previous_jump = NULL;
        ListElem prev = get_prev(bp);
        BB *previousBB = (prev)?object(prev):NULL; 
        if(!previousBB)
          {
            /* FIXME: Maybe we should be more conservative here ! */
            could_fallthrough = 0;
          }
        else
          {
            ListElem last = get_last( previousBB->stmtlist );
            irnode *l = (last)?object(last):NULL;
            if (l && NODETYPE (l) == n_JUMP)
	        {
	          previous_jump = (label *) GET_IRCHILD (l, 1);
                }
          }

        /* from now we got previous block and last jump stmt. Now check if same or not */
 
        ListElem p = get_first (currentBB->stmtlist);	/* p points on first stmt */
	ListElem q = (p)?get_next(p):NULL;		/* q points on 2nd stmt */
	  
        if (p && q)
          {
            irnode *ip = object (p);
            irnode *iq = object (q);  
            if (NODETYPE (ip) == n_LABEL && NODETYPE (iq) == n_JUMP)
	        {

	          label *target_label = (label *) GET_IRCHILD (iq, 1);
	          label *label_to_replace = (label *) GET_IRCHILD (ip, 0);

                  /* check with previous jump, if needed */
                  if ( could_fallthrough == 0 || (could_fallthrough == 1 && 
                                                  previous_jump != NULL && !same_label(previous_jump, label_to_replace)))
                    {
                      /* remove the block. but before redirect all occurences to heading label */
	              replace_label (t, label_to_replace, target_label);
	              /* discard whole block */
                      drop (b, bp);
                    }
	        }
           }

	  bp = next_block;
	}
      tp = get_next (tp);
    }

}

/* tell if statement s and expression e can commute */
int
commute (irnode * s, irnode * e)
{
  int etype = NODETYPE (e);
  int stype = NODETYPE (s);
  if ((stype == n_NOP) ||
      (etype == n_CONST) ||
      ((stype == n_EXPR) && (NODETYPE (GET_IRCHILD (s, 0)) == n_CONST)) ||
      ((stype == n_EXPR) && (NODETYPE (GET_IRCHILD (s, 0)) == n_MEM)) ||
      (etype == n_NAME))
    return 1;
  return 0;
}


typedef struct xir_couple
{
  irnode *s;
  irnode *e;
} ir_couple;

ir_couple *
new_couple (irnode * s, irnode * e)
{
  ir_couple *c = (ir_couple *) xmalloc (sizeof (ir_couple));
  c->s = s;
  c->e = e;
  return c;
}

irnode *
SEQ (irnode * left, irnode * right)
{
  if (NODETYPE (left) == n_NOP)
    return right;
  if (NODETYPE (right) == n_NOP)
    return left;
  return nSEQ (left, right);
}

// forward declaration
irnode *removeESEQfromStmt ();
irnode *reorder ();

ir_couple *
removeESEQfromExp (irnode * e)
{
  irnodeRefList list;
  ListElem l;
  ir_couple *couple;
  irnode *stmt;
  switch (NODETYPE (e))
    {
    case n_ESEQ:
      couple = removeESEQfromExp (GET_IRCHILD (e, 1));
      return new_couple (SEQ (removeESEQfromStmt (GET_IRCHILD (e, 0)), couple->s), couple->e);
    case n_MEM:
      list = newList();
      add (list , GET_IRCHILD_PTR (e, 0));
      return new_couple (reorder (list), e);
    case n_ADD:
    case n_SUB:
    case n_MUL:
    case n_DIV:
    case n_MOD:
    case n_AND:
    case n_OR:
    case n_LSHIFT:
    case n_RSHIFT:
    case n_BOR:
    case n_BAND:
    case n_BXOR:
    case n_EQ:
    case n_NE:
    case n_GT:
    case n_LT:
    case n_GE:
    case n_LE:
      list = newList();
      add (list, GET_IRCHILD_PTR (e, 0));
      add (list, GET_IRCHILD_PTR (e, 1));
      return new_couple (reorder (list), e);
    case n_CALL:
      list = newList();
      add (list, GET_IRCHILD_PTR (e, 0));
      l = get_first( (irnodeList) GET_IRCHILD (e, 1) );  
      while (l)
	{
        add (list, get_ref(l));
        l = get_next (l);
	}
      return new_couple (removeESEQfromStmt (reorder (list)), e);
    }
  return new_couple (nNOP (), e);
}

irnode *
removeESEQfromStmt (irnode * s)
{
  irnodeRefList list;
  ListElem l;
  irnode *stmt;
  if (!s) return nNOP ();
  switch (NODETYPE (s))
    {
    case n_JUMP:
      list = newList();
      add (list, GET_IRCHILD_PTR (s, 0));
      return SEQ (reorder (list), s);
    case n_EXPR:
      if (NODETYPE (GET_IRCHILD (s, 0)) == n_CALL)
	{
        list = newList();
	add (list,(GET_IRCHILD_PTR (GET_IRCHILD (s, 0), 0)));
        l = get_first( (irnodeList) GET_IRCHILD (GET_IRCHILD (s, 0), 1) );
	while (l)
          {
	    add (list, get_ref(l));
            l = get_next (l);
	  }
	  return SEQ (reorder (list), s);
	}
      list = newList();
      add (list, GET_IRCHILD_PTR (s, 0));
      return SEQ (reorder (list), s);
    case n_SEQ:
      return SEQ (removeESEQfromStmt (GET_IRCHILD (s, 0)),
		  removeESEQfromStmt (GET_IRCHILD (s, 1)));
    case n_CJUMP:
      list = newList();
      add (list,GET_IRCHILD_PTR (GET_IRCHILD (s, 0), 0));
      add (list,GET_IRCHILD_PTR (GET_IRCHILD (s, 0), 1));
      return SEQ (reorder (list), s);
    case n_MOVE:
      if (NODETYPE (GET_IRCHILD (s, 1)) == n_CALL
	  && NODETYPE (GET_IRCHILD (s, 0)) == n_TEMP)
	{
        list = newList();
        add (list,(GET_IRCHILD_PTR (GET_IRCHILD (s, 1), 0)));
	l = get_first( (irnodeList) GET_IRCHILD (GET_IRCHILD (s, 1), 1));
        while (l)
	    {
	    add (list, get_ref(l) );
            l = get_next (l);
	    }
	return SEQ (reorder (list), s);
	}
      switch (NODETYPE (GET_IRCHILD (s, 0)))
	{
	case n_TEMP:
          list = newList();
	  add (list, GET_IRCHILD_PTR (s, 1));
	  return SEQ (reorder (list), s);
	case n_MEM:
	  list = newList();
          add (list, GET_IRCHILD_PTR (GET_IRCHILD (s, 0), 0));
	  add (list, GET_IRCHILD_PTR (s, 1));
	  return SEQ (reorder (list), s);
	case n_ESEQ:
	  stmt = removeESEQfromStmt (GET_IRCHILD (GET_IRCHILD (s, 0), 0));
	  *GET_IRCHILD_PTR (s, 0) = GET_IRCHILD (GET_IRCHILD (s, 0), 1);
	  return removeESEQfromStmt (SEQ (stmt, s));
	}
      assert (0);
    }
  return s;
}

irnode *
reorder (irnodeRefList list)
{
  temp *t;
  // NULL
  if (list == NULL || is_empty(list))
    return nNOP ();

  ListElem e = get_first(list);
  irnode **node = (irnode **) object(e);
  irnode *nodep = *node;

  // CALL
  if (NODETYPE (nodep) == n_CALL)
    {
      t = new_temp ();
      *node = nESEQ (nMOVE (nTEMP (t), nodep), nTEMP (t));
      return reorder (list);
    }
  // Otherwise...
  irnodeRefList tail = copy (list);
  drop( tail, get_first(tail) );
  ir_couple *first = removeESEQfromExp (nodep);
  irnode *s = reorder (tail);
  if (commute (s, first->e))
    {
      *node = first->e;
      return SEQ (first->s, s);
    }
  else
    {
      t = new_temp ();
      *node = nTEMP (t);
      return SEQ (first->s, SEQ (nMOVE (nTEMP (t), first->e), s));
    }
}

//
// unfold one statement (eventually a SEQ of SEQuences...) to a linear list of stmt
//
stmtList
unfold (stmtList l, irnode * s)
{
  if (NODETYPE (s) != n_SEQ)
    {
      // add this stmt to list, unless it is
      // - NOP() 
      // - sEXPR( e ) where e != eCALL
      if (NODETYPE (s) != n_NOP
	  && !(NODETYPE (s) == n_EXPR
	       && NODETYPE (GET_IRCHILD (s, 0)) != n_CALL))
	{
	  add (l, s);
	}
    }
  else
    {
      l = unfold (l, GET_IRCHILD (s, 0));
      l = unfold (l, GET_IRCHILD (s, 1));
    }
  return l;
}


stmtList
linearize (irnode * tree)
{
  // remove all ESEQ from stmt
  irnode *s = removeESEQfromStmt (tree);
  // get a linear list of stmt, instead of one big sSEQ()
  return unfold (newList(), s);
}

BB *
new_BB (label * l)
{
  BB *bb = (BB *) xmalloc (sizeof (BB));
  bb->l = l;
  bb->insnlist = newList();
  bb->stmtlist = newList();
  bb->marked = 0;
  return bb;
}

BBList
get_BB (irnode * body, frame * fr)
{

  // if original fragment is a function, store reference to function frame
  // for further usage (in register allocation) 
  int store_frame_required = (fr!=NULL);

  // name of the function frame, is applicable
  label *f = NULL;
  if (fr) f = get_frame_label (fr);

  BBList bblist = newList();

  BB *current_bb = new_BB (NULL);
  if (store_frame_required) current_bb->f = fr;

  stmtList l = linearize (body);

  int just_opened = 1;
  int within_epilogue = 0;
  ListElem lp = get_first (l);
  while (lp)
    {
      irnode *s = object (lp);
      // IF LABEL...
      if (NODETYPE (s) == n_LABEL)
	{
	  if (just_opened)
	    {
	      add (current_bb->stmtlist, s);
	      current_bb->l = (label *) GET_IRCHILD (s, 0);
	      just_opened = 0;
              lp = get_next (lp);
	      continue;
	    }

	  // add closing jump
	  add (current_bb->stmtlist,
             nJUMP (nNAME((label *)GET_IRCHILD (s, 0)), (label *)GET_IRCHILD (s,0)));

	  // check if a label was set
	  if (current_bb->l == NULL)
	    {
	      label *lab = new_label ();
	      current_bb->l = lab;
	      current_bb->stmtlist = join ( add(newList(),nLABEL (lab)), current_bb->stmtlist );
	    }

	  // store this block
	  add (bblist, current_bb);

	  // open next block
	  current_bb = new_BB ((label *) GET_IRCHILD (s, 0));
          if (store_frame_required) current_bb->f = fr;
	  add (current_bb->stmtlist, s);
	  just_opened = 0;
          lp = get_next (lp);
	  continue;
	}

      // IF JUMP...
      if (NODETYPE (s) == n_JUMP || NODETYPE (s) == n_CJUMP)
	{
	  add (current_bb->stmtlist, s);
	  // check if a label was set
	  if (current_bb->l == NULL)
	    {
	      label *lab = new_label ();
	      current_bb->l = lab;
	      current_bb->stmtlist = join ( add(newList(),nLABEL (lab)), current_bb->stmtlist );
	    }

	  // store this block
	  add (bblist, current_bb);

	  // open next block
	  current_bb = new_BB (NULL);
          if (store_frame_required) current_bb->f = fr;
	  just_opened = 1;
          lp = get_next (lp);
	  continue;
	}

      if (NODETYPE (s) == n_EPILOGUE)
	{
	  within_epilogue = 1;
	}

      // otherwise
      add (current_bb->stmtlist, s);
      just_opened = 0;
      lp = get_next (lp);
    }

  // check if a label was set
  if (current_bb->l == NULL)
    {
      label *lab = new_label ();
      current_bb->l = lab;
      current_bb->stmtlist = join ( add(newList(),nLABEL (lab)), current_bb->stmtlist );
    }

  // add closing jump to epilogue unless we already are within epilogue
  if (!within_epilogue && f)
    {
      label *epilogue = new_named_label (epilogue_label_name (get_label_name (f)));
      add (current_bb->stmtlist, nJUMP (nNAME (epilogue), epilogue));
    }
  
  // store this block
  add (bblist, current_bb);
  return bblist;
}

trace
get_canonical_trace (fragmentList fraglist)
{

  BBList all = newList();
  BBList l;
  ListElem p;

  char nodename[128];
  int i;

  /* simple exit condition: if no fragment ( = empty source file ) */
  if (is_empty (fraglist))
    {
      return NULL;
    }

  /* gather all basic blocks */
  ListElem e = get_first (fraglist);
  while (e)
    {
      fragment *f = object (e);
      switch (f->kind)
	{
	case frag_func:
	  l = get_BB (f->u.func.body, f->u.func.f);
	  break;
	case frag_var:
	  l = get_BB (f->u.var.body, NULL);
	  break;
	case frag_data:
	  l = get_BB (f->u.data.body, NULL);
	  break;
	case frag_info:
	  l = get_BB (f->u.info.body, NULL);
	  break;
	default:
	  abort ();		// should not get here
	}
      p = get_first (l);
      while (p)
          {
	      add (all, (BB *) object (p) );
            p = get_next (p);
          }
	
      e = get_next (e);
    }

  /* order basic blocks */
  BBList order = newList();
  BBList current_BBlist = newList();
  trace t = newList();
  
  int remaining_unmarked_bb = 1;
  label *target_label;

  BB *bb = object (get_first( all ));
  while (remaining_unmarked_bb)
    {
      bb->marked = 1;
      add (current_BBlist, bb);
      /* get the final jump insn of the block */
      irnode *jump = object (get_last(bb->stmtlist));
      /* get the jump label */
      switch (NODETYPE (jump))
	{
	case n_JUMP:
	  target_label = (label *) GET_IRCHILD (jump, 1);
	  break;
	case n_CJUMP:
	  target_label = ((label *) GET_IRCHILD (jump, FALLTHROUGH));
	  break;
	case n_INFO:
	case n_DATA:
	case n_EPILOGUE:
	  target_label = NULL;
	  break;
	default:
	  keen_error ("unexpected final irnode %s in reoder basic-blocks\n",
		      get_node_name_and_mode (jump, nodename));
	  abort ();
	  break;
	}

      /* search for this label */
      int target_found = 0;
      if (!target_label)
	{
	  goto new_trace;
	}
      label *bb_label;
      p = get_first (all);
      while (p)
	{
        BB *other_bb = object(p);
	  if (!other_bb->marked)
	    {
	      bb_label = other_bb->l;
	      if (same_label (bb_label, target_label))
		{
		  target_found = 1;
		  bb = other_bb;
		  break;
		}
	    }
        p = get_next (p);
	}

    new_trace:

      if (!target_found && NODETYPE (jump) == n_CJUMP)
	{
          // fallthrough was not found among unmarked bb
          // this means that the target label is already included in trace, and we need to add
          // a jump after CJUMP to simulate "fallthrough"
          keen_debug("canon: get_canonical_trace(): adding a jump for label %s\n", get_label_name(target_label));
          add ( bb->stmtlist , nJUMP( nNAME( target_label), target_label ) );
	}

      if (!target_found)
	{
	  add (t, current_BBlist);
	  current_BBlist = newList();
	  // start a new trace
	  p = get_first (all);
        while (p)
	    {
          BB *other_bb = object(p);
	    if (!other_bb->marked)
		{
		  bb = other_bb;
		  break;
		}
          p = get_next (p);
	    }
	}

      remaining_unmarked_bb = 0;
      p = get_first (all);
      while (p)
	{
        BB *other_bb = object(p);
        if (!other_bb->marked)
	    {
	      remaining_unmarked_bb = 1;
	      break;
	    }
        p = get_next (p);
      }
	  
    }

  if (current_BBlist && !is_empty(current_BBlist)) add(t, current_BBlist);

  /* optimization passes */
  if (option_bb_optimization_off == 0)
    {
      remove_fallthrough (t);
      remove_useless_labels (t);
      remove_jump_only_blocks (t);
    }
  if (option_bb_optimization_off > 0)
    {
      remove_fallthrough (t);
    }
  if (option_bb_optimization_off > 1)
    {
      remove_useless_labels (t);
    }
  if (option_bb_optimization_off > 2)
    {
      remove_jump_only_blocks (t);
    }


  /* populate ordered blocks with insn lists */
  int it = 0;
  ListElem pt = get_first(t);
  while(pt)
    {
      BBList bblist = object (pt);
      i = 0;
      p = get_first (bblist);
      while(p)
	{

        BB *bb = object(p);

	  /* DEBUG output */
	  keen_debug ("** TRACE %2d | BASIC BLOCK %2d:\n", it, i);
	  if (debug_level()) print_stmtList (bb->stmtlist);
	  /* END DEBUG */

	  // generate pseudo-asm instructions for the whole block 
	  ListElem s = get_first (bb->stmtlist);
	  while(s)
	    {
	      genInsn (bb->insnlist, object(s));
            s = get_next (s);
	    }

	  // if first block instruction is a label, 
	  // put reference to this insn within label object to ease further searches/parses 
	  ListElem l = get_first(bb->insnlist);
	  if (l)
	    {
	      irnode *first = object(l);
	      if (NODETYPE (first) == n_INSN_LABEL)
		{
		  label *la = (label*) (object (get_first
				 ((labelList) GET_IRCHILD (first, 3))));
		  set_label_location (la, first);
		}
	    }
        p = get_next (p);
        i++;
	}	

      pt = get_next (pt);
      it++;
    }

  /* return trace */
  return t;
}

// DEBUG
void
print_stmtList (stmtList l)
{
  ListElem p = get_first(l);
  int i = 0;
  while (p)
    {
      keen_debug ("[%d] ", i++);
      print_tree ( object(p), 0);
      p = get_next (p);
    }
}

// OUTPUT TO FILE
void
output_trace (trace t, int fd)
{
  ListElem p = get_first(t);
  while (p)
    {
      BBList bblist = (BBList) object (p);
      ListElem q = get_first(bblist);
      while (q)
        {
	  BB *bb = object(q);
	  insnList insnl = bb->insnlist;
	  ListElem r = get_first(insnl);
        while(r)
	    {
	      irnode *insn = (irnode *) object(r);
	      print_insn_to_file (insn, fd);
            r = get_next (r);
	    }
        q = get_next (q);
	}
      p = get_next (p);
    }
}
