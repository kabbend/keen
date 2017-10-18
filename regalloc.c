/* 
* Keen compiler 
* Copyright (C) 2007 Karim Ben Djedidia <kabend@free.fr> 
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
#include <limits.h>
#include "flow.h"
#include "list.h"
#include "regalloc.h"
#include "reg.h"
#include "error.h"
#include "temp.h"

/* total number of registers available for register allocation phase is 
 * reduced by number of special registers (at least stack pointer)
 */
#define K	(N_REG - (N_REG - REG_FIRST_SPECIAL))

interference *i;

t_nodeList initial_wl;
t_nodeList precolored_wl;
t_nodeList simplify_wl;
t_nodeList freeze_wl;
t_nodeList spill_wl;
t_nodeList spilled_nodes_wl;
t_nodeList coalesced_nodes_wl;
t_nodeList colored_nodes_wl;
t_nodeList select_wl;
moveSet coalescedMovesSet;
moveSet constrainedMovesSet;
moveSet frozenMovesSet;
moveSet worklistMovesSet;
moveSet activeMovesSet;
	/* worklists and sets used for register allocation */

t_node *best_spill = 0;
	/* always store best candidate for spilling */

/* recompute spill value of a tnode, when its degree has changed. If such tnode
   is currently in spill worklist (so candidate for spilling) and has lowest
   spill value, it becomes best_spill */
void
compute_spill_value( t_node *t )
{
  keen_debug("compute_spill_value(%d)(wl 4? %d)\n",t->index,t->current_wl);
  if (t->current_wl != wl_spill) return; // not candidate in any case
  if (is_a_spill(t->t)) 
	{ 
	  t->spill_value = LONG_MAX; 
  	  keen_debug("compute_spill_value(%d) on a spill\n",t->index);
	  return; /* a spilled register is never a good candidate for spilling again */ 
	}

  t->spill_value = get_usedef(t->t) * 1e6 / i->degree[t->index];

  if (best_spill && t->current_wl == wl_spill && t->spill_value < best_spill->spill_value) 
	{
	  /* becomes best spill */
	  best_spill = t;
  	  keen_debug("compute_spill_value(%d) becomes best spill\n",t->index);
	}

  if (!best_spill) 
	{
	  /* becomes best spill */
	  best_spill = t;
  	  keen_debug("compute_spill_value(%d) becomes best spill\n",t->index);
	}

}

/* When a node is removed from spill worklist, and is the current best spill, we need to
   find another best spill */
void
choose_another_best_spill ( List spill_list )
{
  t_node *t;
  long MIN = LONG_MAX;
  keen_debug("best spill was %d before\n",best_spill->index);
  best_spill = NULL; 
  FOR_EACH (t,spill_list,e)
    {
      if (t->spill_value < MIN) { best_spill = t; MIN = t->spill_value; }
      if (t->spill_value == 0) break; // no need to go further, we got the best candidate either
    }
  keen_debug("choose another best spill: %d among %d cand.\n",(best_spill)?best_spill->index:-1,size(spill_list));
}

/* return list_intersection (i->movelists[t->index], list_union (activeMovesSet, worklistMovesSet)) */
moveSet
node_moves (t_node * t)
{
  List ret = newList();
  gnode *g;
  FOR_EACH( g, i->movelists[t->index], e)
    {
	flow_node *f = g->object;
	if (f->current_set == active_set || f->current_set == worklist_set) add (ret, g); 
    }
  return ret;
}

int
is_move_related (t_node * t)
{
  gnode *g;
  FOR_EACH( g, i->movelists[t->index], e)
    {
	flow_node *f = g->object;
	if (f->current_set == active_set || f->current_set == worklist_set) return 1;
    }
  return 0;
}

void
add_interference_edge (t_node * u, t_node * v)
{
  if (!get_bit (v->index, i->imatrix[u->index]) && (u != v))
    {
      set_bit (v->index, i->imatrix[u->index]);
      set_bit (u->index, i->imatrix[v->index]);
      if (u->current_wl != wl_precolored)
	{
	  add (u->g->out_edges, v->g);	// add an edge
	  i->degree[u->index]++;
	  compute_spill_value (u);
	}
      if (v->current_wl != wl_precolored)
	{
	  add (v->g->out_edges, u->g);	// add an edge
	  i->degree[v->index]++;
	  compute_spill_value (v);
	}
    }
}

/* return all adjacent nodes, except if they are in select list
 * or in coalesced list */
gnodeList
adjacent (gnode * g)
{
  t_nodeList convert = newList ();
  gnodeList l = neighbours (g);
  ListElem e = get_first (l);
  while (e)
    {
      t_node *t = object (e);
      if (t->current_wl != wl_select && t->current_wl != wl_coalesced) add (convert, t);
      e = get_next (e);
    }
  return convert;
}

void
enable_moves (gnodeList gl)
{
  ListElem e = get_first (gl);
  while (e)
    {
      moveSet m = node_moves ((t_node *) ((gnode *) object (e))->object);
      ListElem f = get_first (m);
      while (f)
	{
	  gnode *fg = object (f);
	  flow_node *fn = (flow_node *) fg->object;
	  if (fn->current_set == active_set)
	    {
	      drop (activeMovesSet, fn->current_set_holder);
	      ListElem x = _add (worklistMovesSet, fg);
	      fn->current_set = worklist_set;
	      fn->current_set_holder = x;
	    }
	  f = get_next (f);
	}
      e = get_next (e);
    }
}

void
decrement_degree (gnode * g)
{
  t_node *t = (t_node *) g->object;
  if (t->current_wl == wl_precolored) return; // precolored are not concerned
  int d = i->degree[t->index];
  keen_debug("decrementing degree of tnode %d, from %d to %d\n",t->index,d,d-1);
  (i->degree[t->index])--;
  compute_spill_value (t);
  if (d == K && t->current_wl == wl_spill)
    {
      keen_debug("tnode %d: degree is now %d in spillwl, need to change list\n", t->index, K);
      List adjacents = adjacent (g);
      add( adjacents, g);
      enable_moves (adjacents);
      drop (spill_wl, t->current_wl_holder);
      if (t == best_spill) choose_another_best_spill(spill_wl);
      if (is_move_related (t))
	{
	  ListElem e = _add (freeze_wl, t);
	  t->current_wl = wl_freeze;
	  t->current_wl_holder = e;
          keen_debug("In decrement: adding tnode %d to freeze list\n",t->index);
	}
      else
	{
	  ListElem e = _add (simplify_wl, t);
	  t->current_wl = wl_simplify;
	  t->current_wl_holder = e;
          keen_debug("In decrement: adding tnode %d to select list\n",t->index);
	}
    }
}

void
simplify ()
{
  // get one node in simplify worklist
  ListElem e = drop (simplify_wl, get_first (simplify_wl));
  // add it to select stack
  t_node *t = (t_node *) object (e);
  ListElem pos = _add (select_wl, t);
  t->current_wl = wl_select;
  t->current_wl_holder = pos;
  keen_debug("In simplify: adding tnode %d to select list\n",t->index);
  // decrement degree for all adjacents
  gnodeList adjacents = adjacent (t->g);
  e = get_first (adjacents);
  while (e)
    {
      decrement_degree (object (e) /* gnode */ );
      e = get_next (e);
    }
}

int
get_alias (int temp_index)
{
  t_node *n = (t_node *) ((gnode *) object (get_ith (i->igraph->nodes, temp_index)))-> object;
  if (n->current_wl == wl_coalesced) return get_alias (i->alias[temp_index]);
  return temp_index;
}

void
add_worklist (t_node * u)
{
  keen_debug("In add_worklist(): called on %d\n", u->index);
  if ((u->current_wl != wl_precolored) && (!(is_move_related (u)))
      && (i->degree[u->index] < K))
    {
      drop (freeze_wl, u->current_wl_holder);
      ListElem e = _add (simplify_wl, u);
      u->current_wl = wl_simplify;
      u->current_wl_holder = e;
      keen_debug("In add_worklist: adding tnode %d to select list\n",u->index);
    }
  else
    {
      keen_debug("In add_worklist: do nothing with %d\n",u->index);
    }
}

int
OK (t_node * v, t_node * u)
{
  gnodeList l = adjacent (v->g);
  ListElem e = get_first (l);
  while (e)
    {
      t_node *t = (t_node *) ((gnode *) object (e))->object;
      if ((i->degree[t->index] >= K) && (t->current_wl != wl_precolored)
	  && (!get_bit (u->index, i->imatrix[t->index])))
	return 0;
      e = get_next (e);
    }
  return 1;
}

int
conservative (gnodeList l)
{
  ListElem e = get_first (l);
  int k = 0;
  while (e)
    {
      t_node *t = (t_node *) ((gnode *) object (e))->object;
      if (i->degree[t->index] >= K)
	k++;
      e = get_next (e);
    }
  return (k < K);
}

void
combine (t_node * u, t_node * v)
{
  if (v->current_wl == wl_freeze)
    {
      drop (freeze_wl, v->current_wl_holder);
    }
  else
    {
      drop (spill_wl, v->current_wl_holder);
      if (v == best_spill) choose_another_best_spill(spill_wl);
    }
  ListElem e = _add (coalesced_nodes_wl, v);
  v->current_wl = wl_coalesced;
  v->current_wl_holder = e;
  keen_debug("In combine: %d is now in coalesced_nodes_wl list.\n",v->index);
  i->alias[v->index] = u->index;
  i->movelists[u->index] = list_union (i->movelists[u->index], i->movelists[v->index]);
  enable_moves (add (newList (), v->g));
  gnodeList l = adjacent (v->g);
  e = get_first (l);
  while (e)
    {
      t_node *t = (t_node *) ((gnode *) object (e))->object;
      add_interference_edge (t, u);
      decrement_degree (t->g);
      e = get_next (e);
    }
  if ((i->degree[u->index] >= K) && (u->current_wl == wl_freeze))
    {
      keen_debug("In combine: %d is transferred from freeze to spill_wl.\n",u->index);
      drop (freeze_wl, u->current_wl_holder);
      ListElem e = _add (spill_wl, u);
      u->current_wl = wl_spill;
      u->current_wl_holder = e;
      compute_spill_value (u);
    }
}

void
coalesce ()
{

  // take 1st element of worklistMoves
  ListElem le = drop (worklistMovesSet, get_first (worklistMovesSet));
  gnode *g = (gnode *) object (le);	// gnode in flowgraph
  flow_node *fn = (flow_node *) g->object;	// flow_node in flowgraph
  irnode *insn = fn->insn;	// actual move insn

  // take both dest and src temporaries from this move insn
  tempList destL = (tempList) GET_IRCHILD (insn, 1);
  tempList srcL = (tempList) GET_IRCHILD (insn, 2);
  temp *xtemp = (temp *) object (get_first (destL));	// FIXME: we suppose one dest only. check this
  temp *ytemp = (temp *) object (get_first (srcL));	// FIXME: we suppose one src only. check this

  keen_debug("In coalesce: treat move with %d and %d\n", xtemp->index, ytemp->index);

  int x = get_alias (xtemp->index);
  int y = get_alias (ytemp->index);

  keen_debug("In coalesce: lead to aliases %d and %d\n", x, y);

  // order them
  t_node *xt = (t_node *) ((gnode *) object (get_ith (i->igraph->nodes, x)))->object;
  t_node *yt = (t_node *) ((gnode *) object (get_ith (i->igraph->nodes, y)))->object;

  keen_debug("In coalesce: fetch aliases %d and %d\n", xt->index, yt->index);
  keen_debug("In coalesce: (%d wl %d, %d wl %d)\n", xt->index, xt->current_wl, yt->index, yt->current_wl);
  
  t_node *u, *v;
  if (yt->current_wl == wl_precolored)
    {
      u = yt;
      v = xt;
    }
  else
    {
      u = xt;
      v = yt;
    }

  // coalesce
  if (u == v)
    {
      ListElem x = _add (coalescedMovesSet, g);
      fn->current_set = coalesced_set;
      fn->current_set_holder = x;
      add_worklist (u);
      keen_debug("In coalesce: are same. add %d to worklist\n", u->index);
    }
  else if ((v->current_wl == wl_precolored)
	   || (get_bit (v->index, i->imatrix[u->index])))
    {
      ListElem x = _add (constrainedMovesSet, g);
      fn->current_set = constrained_set;
      fn->current_set_holder = x;
      add_worklist (u);
      add_worklist (v);
      keen_debug("In coalesce: add %d to worklist\n", u->index);
      keen_debug("In coalesce: add %d to worklist\n", v->index);
    }
  else if (((u->current_wl == wl_precolored) && OK (v, u)) ||
	   ((u->current_wl != wl_precolored)
	    && conservative (list_union (adjacent (u->g), adjacent (v->g)))))
    {
      ListElem x = _add (coalescedMovesSet, g);
      fn->current_set = coalesced_set;
      fn->current_set_holder = x;
      keen_debug("In coalesce: Before combine. current_wl of %d is %d\n", u->index, u->current_wl);
      combine (u, v);
      keen_debug("In coalesce: Combine %d and %d. add %d to worklist\n", u->index, v->index, u->index);
      keen_debug("In coalesce: current_wl of %d is %d\n", u->index, u->current_wl);
      add_worklist (u);
      keen_debug("In coalesce: current_wl of %d is %d\n", u->index, u->current_wl);
    }
  else
    {
      ListElem x = _add (activeMovesSet, g);
      fn->current_set = active_set;
      fn->current_set_holder = x;
      keen_debug("In coalesce: do not add to worklist\n");
    }

}

void
freeze_moves (t_node * u)
{
   
  ListElem e = get_first (node_moves (u));
  while (e)
    {
      gnode *g = (gnode *) object (e);	// gnode in flowgraph
      flow_node *fn = (flow_node *) g->object;	// flow_node in flowgraph
      irnode *insn = fn->insn;	// actual move insn
      tempList destL = (tempList) GET_IRCHILD (insn, 1);
      tempList srcL = (tempList) GET_IRCHILD (insn, 2);
      temp *xtemp = (temp *) object (get_first (destL));	// FIXME: we suppose one dest only. check this
      temp *ytemp = (temp *) object (get_first (srcL));	// FIXME: we suppose one src only. check this
      t_node *x = (t_node *) ((gnode *) object (get_ith (i->igraph->nodes, xtemp->index)))->object;
      t_node *y = (t_node *) ((gnode *) object (get_ith (i->igraph->nodes, ytemp->index)))->object;
      t_node *v;
      if (get_alias (y->index) == get_alias (u->index))
	{
	  v = (t_node *) ((gnode *) object (get_ith (i->igraph->nodes, get_alias (x->index))))->object;
	}
      else
	{
	  v = (t_node *) ((gnode *) object (get_ith (i->igraph->nodes, get_alias (y->index))))->object;
	}
      drop (activeMovesSet, fn->current_set_holder);
      ListElem pos = _add (frozenMovesSet, g);
      fn->current_set = frozen_set;
      fn->current_set_holder = pos;
      if (is_empty (node_moves (v)) && (i->degree[v->index] < K) && !(v->current_wl == wl_precolored))
	{
	  drop (freeze_wl, v->current_wl_holder);
	  ListElem e = _add (simplify_wl, v);
	  v->current_wl = wl_simplify;
	  v->current_wl_holder = e;
          keen_debug("In freeze_moves: add %d to simplify wl \n",v->index);
	}
      e = get_next (e);
    }

}

void
freeze ()
{
  // get one node in freeze worklist
  t_node *t = (t_node *) object (drop (freeze_wl, get_first (freeze_wl)));
  ListElem e = _add (simplify_wl, t);
  t->current_wl = wl_simplify;
  t->current_wl_holder = e;
  keen_debug("In freeze: adding %d to simplify\n",t->index);
  freeze_moves (t);
}

t_node *
select_a_spill_heuristic( List spill_candidates )
{

  keen_debug("choose %d among %d candidates\n",best_spill->index, size(spill_candidates));
  return best_spill;
	/* the best candidate for spilling has already been registered */
}

int
select_spill ( interference *i, trace tr )
{

  keen_debug("Need to choose a spill candidate for simplification\n");

  t_node *t = select_a_spill_heuristic( spill_wl );

  if (is_a_spill ( t->t )) 
    {

      // added this test to detect potential issues upstream
      // in particular if some instructions wrongly define uses and defs and cause
      // overwhelming spilling effects

      // all nodes were previously spilled. We probably are in trouble...
      keen_warning("warning. Can't choose a spill temp not already subject to prior spill");
      keen_debug("warning. Can't choose a spill temp not already subject to prior spill");

      // Need to perform a brute change to avoid infinite loop. Choose another spill even
      // if previous pass did not identify it for spilling. This imply immediate rewrite of
      // code and restart regalloc
      // FIXME not sure it is efficient
      int p;
      List voisinage = newList();
      for(p=K;p<i->size;p++) 
        {
          t_node *o = (t_node *)((gnode *) object(get_ith( i->igraph->nodes, p )))->object;
          if ( (get_bit(p,i->imatrix[t->index])) && o->current_wl == wl_select 
               && get_temp_mode(p) == t->t->mode ) add(voisinage, o);
        }
      t_node *t = select_a_spill_heuristic( voisinage );
      ListElem e = _add(spilled_nodes_wl, t);
      t->current_wl = wl_spilled;
      t->current_wl_holder = e;
      return 1;
 
    }

  drop (spill_wl, t->current_wl_holder);
  if (t == best_spill) choose_another_best_spill(spill_wl);
  ListElem e = _add (simplify_wl, t);
  t->current_wl = wl_simplify;
  t->current_wl_holder = e;

  keen_debug("In select_spill: adding %d to simplify\n",t->index);

  freeze_moves (t);

  return 0;

}

void
assign_colors ()
{

  int p;
  int *okColors = (int *) xmalloc (sizeof (int) * K);
  int remaining_colors;

  ListElem e;
  while (e = drop (select_wl, get_last (select_wl)))
    {

      t_node *n = (t_node *) object (e);
      keen_debug("In assign color: drop object t_node %d\n", n->index );

      if (i->color[n->index] != NOREG) { keen_debug("Apparently %d has already been treated\n", n->index); continue; }

      for (p = 0; p < K; p++) okColors[p] = 1;	// prepare colors
      remaining_colors = K;

      // remove already assigned colors
      gnodeList adjacents = adjacent(n->g);
      gnode *adj;
      FOR_EACH ( adj, adjacents, z)
	{
	  t_node *w = adj->object;
	  int p = w->index;
	  int w_index = get_alias (w->index);
          keen_debug("In assign color: t_node %d interferes with %d (alias %d,%s)\n", n->index, w->index, w_index,get_temp_name(w->t) );

	  t_node *x = (t_node *) ((gnode *) object (get_ith (i->igraph->nodes, w_index)))->object;
	  if ((x->current_wl == wl_colored) || (x->current_wl == wl_precolored))
            {
                  /* only consider non-special registers when choosing a color ... */
	          if (i->color[w_index]<REG_FIRST_SPECIAL) 
			{
			  if (okColors[i->color[w_index]] == 1) remaining_colors--;
			  okColors[i->color[w_index]] = 0;
			}
                  keen_debug("In assign color: t_node %d interferes with %d already colored (%d)\n", n->index, w_index, i->color[w_index] );
	    }
           else
               keen_debug("In assign color: t_node %d interferes with %d not colored...\n", n->index, w->index );
	
	  if (remaining_colors == 0) break;
	}

      // choose a remaining color
      if (remaining_colors == 0)
	{
	  ListElem e = _add (spilled_nodes_wl, n);
	  n->current_wl = wl_spilled;
	  n->current_wl_holder = e;
          keen_debug("> cannot assign color to %d. It becomes actual spill\n",n->index);
	}
      else
	{
          int assign = -1;
          for (p = 0; p < K; p++) if (okColors[p] != 0) { assign = p; break; }
	  assert(assign != -1);
	  ListElem e = _add (colored_nodes_wl, n);
	  n->current_wl = wl_colored;
	  n->current_wl_holder = e;
	  i->color[n->index] = p;
          keen_debug("> assigning color %d to %d\n",p,n->index);
	}

    }
  free (okColors);

  e = get_first (coalesced_nodes_wl);
  while (e)
    {
      t_node *n = (t_node *) object (e);
      i->color[n->index] = i->color[get_alias (n->index)];
      if (n->index < K) keen_debug("warning: %d is recolored with %d\n",n->index,get_alias(n->index));
      e = get_next (e);
    }

}

trace
rewrite_trace ( trace t )
{

  // we store all impacted frames, to rewrite prologue instructions ( = stack
  // allocation impacted by spilled registers) at the end
  List impacted_frames = newList();

  // for each spilled nodes, add store/fetch instructions
  ListElem sp = get_first( spilled_nodes_wl );
  while (sp)
    {

      t_node *spilled_tnode = object (sp);
      spilled_tnode->spilled = NULL;
      temp *spilled_temp = spilled_tnode->t;
      int mode = spilled_temp->mode;


      ListElem insn_holder;
      FOR_EACH (insn_holder, spilled_temp->insn_usedefs , eusedef )
	{
		irnode *insn = object(insn_holder);

      		if (!spilled_tnode->spilled) 
		  {
      		    // allocate & declare new spill on the frame for the tnode.
      		    // this can only be done when 1st block using the node 
      		    // (and thus the corresponding function frame) is known
		     BB *bb = ((BB *) insn->basic_block );
	  	     spilled_tnode->spilled = new_spill( mode, bb->f );
                     // this frame is impacted and prologue need to be rewritten at the end
                     if (!find(impacted_frames,bb->f)) add(impacted_frames,bb->f);
		  }

		tempList uses = (tempList) GET_IRCHILD( insn , 2 );
		ListElem is_a_use = finde( uses , spilled_temp );
		tempList defs = (tempList) GET_IRCHILD( insn , 1 );
		ListElem is_a_def = finde( defs , spilled_temp );
	
      		// replace spilled temp by a new one, with same mode
      		temp *newtemp = new_temp_mode(mode);
      		temp_is_a_spill (newtemp) ;
	
		if (is_a_use)
		  {
                    change( is_a_use , newtemp );
                    irnode *fetch = generate_fetch( spilled_tnode->spilled, newtemp );
                    add_before( insn_holder->l , insn_holder , fetch );
		  }				
		
		if (is_a_def)
		  {
                    change( is_a_def , newtemp );
                    irnode *store = generate_store( spilled_tnode->spilled, newtemp );
                    add_after( insn_holder->l , insn_holder , store );
		  }
	}

      sp = get_next (sp); 
    }

  // rewrite impacted frames
  ListElem e = get_first(impacted_frames);
  while (e)
    {
      update_prologue( (frame *) object(e) );
      e = get_next (e);
    }

  return t;
}

int first_pass = 1;

trace
register_allocation (trace t)
{

  ListElem e;

  print_trace (t);

  /* initialize worklists and sets */
  initial_wl = newList ();
  precolored_wl = newList ();
  simplify_wl = newList ();
  freeze_wl = newList ();
  spill_wl = newList ();
  spilled_nodes_wl = newList ();
  coalesced_nodes_wl = newList ();
  colored_nodes_wl = newList ();
  select_wl = newList ();
  coalescedMovesSet = newList ();
  constrainedMovesSet = newList ();
  frozenMovesSet = newList ();
  worklistMovesSet = newList ();
  activeMovesSet = newList ();

  // optimize before register allocation
  if (first_pass) { 
	optimize_temps_from_trace (t); 
	first_pass = 0; 
  	print_trace (t);
	}

  // initialize interference structure 
  int p;
  i = (interference *) xcalloc (sizeof (interference), 1);
  i->igraph = new_undirected_graph ();
  i->t = t;
  i->size = get_temp_number ();
  i->imatrix = (bitvector *) xcalloc (sizeof (bitvector), i->size);
  i->degree = (int *) xcalloc (sizeof (int), i->size);
  i->alias = (int *) xcalloc (sizeof (int), i->size);
  i->color = (int *) xcalloc (sizeof (int), i->size);
  i->movelists = (gnodeList *) xcalloc (sizeof (gnodeList), i->size);
  for (p = 0; p < i->size; p++)
    {
      i->imatrix[p] = new_bitvector (i->size);
      i->movelists[p] = newList ();
      i->color[p] = NOREG;
    }

  /* Perform static liveness analysis and build interference matrix */
  build (t, i);

//printf("temps=%d\n",size(initial_wl));

  /* add more interferences depending on temporary mode */
  t_nodeList all_temps = initial_wl;
  e = get_first (all_temps);
  while (e)
    {
      t_node *t = object (e);
      int requested_mode = t->t->mode;
      ListElem f = get_first (precolored_wl);
      while (f)
	{
	  t_node *u = object (f);
	  int other_mode = u->t->mode;
	  if (requested_mode != other_mode)
	    add_interference_edge (t, u);
	  f = get_next (f);
	}
      e = get_next (e);
    }

  /* prepare worklists */
  e = get_first (precolored_wl);
  while (e)
    {
      t_node *t = object (e);
      keen_debug("initial precolored: node %d (degree %d)\n", t->index, i->degree[t->index]);
      e = get_next(e);
    }

  long SPILL_MIN = LONG_MAX;
	/* used below to find the lowest spill value and the best spill candidate */

  while (e = drop (initial_wl, get_first (initial_wl)))
    {
      t_node *t = object (e);
      if ((i->degree[t->index]) >= K)
	{
	  ListElem e = _add (spill_wl, t);
	  t->current_wl = wl_spill;
	  t->current_wl_holder = e;

	  t->spill_value = get_usedef(t->t) * 1e6 / i->degree[t->index];
          if (t->spill_value < SPILL_MIN) { SPILL_MIN = t->spill_value; best_spill = t; }
		/* we compute spill values for all potential candidates, and choose the best one
 		   at the same time. When the best spill is removed from worklist, we use the
		   stored values to find another best candidate */

          keen_debug("initial spill: node %d (degree %d)\n", t->index, i->degree[t->index]);
	}
      else if (is_move_related (t))
	{
	  ListElem e = _add (freeze_wl, t);
	  t->current_wl = wl_freeze;
	  t->current_wl_holder = e;
          keen_debug("initial freeze: node %d (degree %d)\n", t->index, i->degree[t->index]);
	}
      else
	{
	  ListElem e = _add (simplify_wl, t);
	  t->current_wl = wl_simplify;
	  t->current_wl_holder = e;
          keen_debug("initial simplify: node %d (degree %d)\n", t->index, i->degree[t->index]);
	}
    }

  /*
   * Main Loop
   */
  do
    {

      if (debug_level())
	{
          t_node *tn;
      	  keen_debug("SIZES\n");
      	  keen_debug("  simplify_wl %d = ",size(simplify_wl));
      	  FOR_EACH(tn,simplify_wl,es) { keen_debug("%d ",tn->index); } keen_debug("\n");
      	  keen_debug("  freeze_wl %d = ",size(freeze_wl));
      	  FOR_EACH(tn,freeze_wl,ef) { keen_debug("%d ",tn->index); } keen_debug("\n");
      	  keen_debug("  spill_wl %d = ",size(spill_wl));
      	  FOR_EACH(tn,spill_wl,ep) { keen_debug("%d ",tn->index); } keen_debug("\n");
	}

      if (!is_empty (simplify_wl))
	{
	  simplify ();
	  continue;
	}
      if (!is_empty (worklistMovesSet))
	{
	  coalesce ();
	  continue;
	}
      if (!is_empty (freeze_wl))
	{
	  freeze ();
	  continue;
	}
      if (!is_empty (spill_wl))
	{
	  int need_to_rewrite = select_spill (i,t);
          if (need_to_rewrite) goto rewrite;
	  continue;
	}

    }
  while ((!is_empty (simplify_wl)) ||
	 (!is_empty (freeze_wl)) ||
	 (!is_empty (spill_wl)) || 
         (!is_empty (worklistMovesSet)));

  /*
   * try to assign a color to all temps 
   */
  assign_colors ();

  /*
   * test for recursion
   */
  rewrite:
  if (!is_empty (spilled_nodes_wl))
    {
      keen_debug ("regalloc: need to rewrite trace\n");
      ListElem e = get_first (spilled_nodes_wl);
      t = rewrite_trace (t);
      print_trace (t);
      return register_allocation(t); 
    }

  /* 
   * store colors definitely into temps
   */
  for (p = 0; p < i->size; p++)
    {
      if (!is_mapped (get_temp (p)))
	map_temp (p, i->color[p]);
    }

  // DEBUG
  print_trace (t);

  /*
   * remove coalesced moves from trace (dest = src)
   */
  int count_removed = 0;
  ListElem pt = get_first (t);
  while (pt)
    {
      BBList bblist = object (pt);
      ListElem bp = get_first (bblist);
      while (bp)
	{
	  BB *bb = (BB *) object(bp);
	  insnList insnl = bb->insnlist;
          ListElem ip = get_first (insnl);
          while (ip)
	    {
	      irnode *insn = object (ip);
	      if (is_move (insn))
		{
		  tempList defs = (tempList) GET_IRCHILD (insn, 1);
		  tempList uses = (tempList) GET_IRCHILD (insn, 2);
		  temp *def = object (get_first (defs));
		  temp *use = object (get_first (uses));
		  if (get_temp_map (def) == get_temp_map (use))
		    {
		      change( ip, NULL );
		      count_removed++;
		    }
	        }
                ip = get_next (ip);
	    }
            bp = get_next (bp);
	}
        pt = get_next (pt);
    }

  // DEBUG
  if (debug_level () >= DEBUG_VERBOSE)
    {
      keen_debug ("removed %d move instructions\n", count_removed);
      print_trace (t);
    }

  return t;

}
