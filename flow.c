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

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "canon.h"
#include "ir.h"
#include "flow.h"
#include "list.h"
#include "temp.h"		/* for get_temp_number() */
#include "regalloc.h"
#include "error.h"

#define min(x,y) ((x)<(y)?(x):(y))

extern moveSet worklistMovesSet;
extern t_nodeList initial_wl;
extern t_nodeList precolored_wl;

/*
 * BITVECTOR routines
 */

/* by construction we use uint32 32bits integer as bitvector unit */
/* Some low-level operations (memcpy, memset) will be done on bytes, though, so
 * we indicate that our bitvector-unit is 4 bytes long */
#define SIZE_IN_BITS	32
#define SIZE_IN_BYTES	4

uint32_t masks[] =
{
     1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384,
 32768, 65536, 131072, 262144, 524288, 1048576, 2097152, 4194304, 8388608,
 16777216, 33554432, 67108864, 134217728, 268435456, 536870912, 1073741824,
 2147483648
};

bitvector
new_bitvector (int size)
{
  bitvector new = (bitvector) xmalloc (sizeof (struct xbitvector));
  new->bit_size = size;
  new->vector_size = (size + (SIZE_IN_BITS - 1)) / SIZE_IN_BITS;
  new->v = (uint32_t *) xcalloc (new->vector_size, SIZE_IN_BYTES);
  return new;
}

void
free_bitvector (bitvector b)
{
  if (b)
    {
      free (b->v);
      free (b);
    }
}

void
set_bit (int bit_number, bitvector bit_vector)
{
  uint32_t mask = masks[ SIZE_IN_BITS - 1 - (bit_number % SIZE_IN_BITS)];
  bit_vector->v[bit_number / SIZE_IN_BITS] |= mask;
}

void
or_vector (bitvector b1, bitvector b2)
{
  int i;
  assert (b1->bit_size == b2->bit_size);
  for (i = 0; i < b1->vector_size; i++)
    {
      b1->v[i] |= b2->v[i];
    }
}

void
and_vector (bitvector b1, bitvector b2)
{
  int i;
  assert (b1->bit_size == b2->bit_size);
  for (i = 0; i < b1->vector_size; i++)
    {
      b1->v[i] &= b2->v[i];
    }
}

void
xor_vector (bitvector b1, bitvector b2)
{
  int i;
  assert (b1->bit_size == b2->bit_size);
  for (i = 0; i < b1->vector_size; i++)
    {
      b1->v[i] ^= b2->v[i];
    }
}

void
cmp_vector (bitvector b)
{
  int i;
  for (i = 0; i < b->vector_size; i++)
    {
      b->v[i] = ~(b->v[i]);
    }
}

void
copy_bitvector (bitvector dest, bitvector src)
{
  assert (dest->bit_size == src->bit_size);
  memcpy (dest->v, src->v, dest->vector_size * SIZE_IN_BYTES);
}

void
zero_bitvector (bitvector dest)
{
  memset (dest->v, 0, dest->vector_size * SIZE_IN_BYTES);
}

int
vector_is_null (bitvector v)
{
  int i;
  for (i = 0; i < v->vector_size - 1; i++)
    {
      if (v->v[i] != 0)
	return 0;
    }
  // last int is to be treated differently
  uint32_t mask = 0xFFFFFFFF << (SIZE_IN_BITS - (v->bit_size % SIZE_IN_BITS));
  uint32_t last = v->v[v->vector_size - 1] & mask;
  return (last == 0);
}

/*
 * GRAPH routines 
 */

/* return 1 if a and b gnodes are linked together by an edge */
/* if graph is directed, we search for a directed edge from a to b */
int
linked (gnode * a, gnode * b)
{
  if (!a || !b) return 0;
  return (find (a->out_edges, b) != NULL);
  return 0;
}

/* creates a new directed graph */
graph *
new_graph ()
{
  graph *new = (graph *) xcalloc (1, sizeof (graph));
  new->kind = g_directed;
  new->nodes = newList ();
  return new;
}

/* creates a new undirected graph */
graph *
new_undirected_graph ()
{
  graph *new = (graph *) xcalloc (1, sizeof (graph));
  new->kind = g_undirected;
  new->nodes = newList ();
  return new;
}

/* add an edge. Can be directed or not depending on graph kind */
void
add_edge (gnode * from, gnode * to)
{
  if (!linked (from, to))
    {
      add (from->out_edges, to);
      add (to->in_edges, from);
      if (from->g->kind == g_undirected)
	{
	  add (to->out_edges, from);
	  add (from->in_edges, to);
	}
    }
}

/* create and insert a new node into graph g */
gnode *
add_gnode (graph * g, void *o)
{
  gnode *new = (gnode *) xmalloc (sizeof (gnode));
  new->object = o;
  new->g = g;
  // initialize empty edges lists
  new->in_edges = newList ();
  new->out_edges = newList ();
  // add it to graph
  add (g->nodes, new);
  return new;
}

/* removes and returns a gnode from its graph, and edges as well */
gnode *
remove_gnode (gnode * g)
{

  gnode *node;

  // remove incoming edges
  ListElem e = get_first (g->in_edges);
  while (e)
    {
      node = object (e);
      remove_edge (node, g);
      e = get_next (e);
    }

  // no need to repeat again if graph is undirected, 
  // since previous pass removed all egdes 
  if (g->g->kind == g_directed)
    {
      e = get_first (g->out_edges);
      while (e)
	{
	  node = object (e);
	  remove_edge (g, node);
	  e = get_next (e);
	}
    }

  find_n_drop (g->g->nodes, g);

  return g;
}

/* removes an existing edge between a and b nodes */
void
remove_edge (gnode * a, gnode * b)
{

  find_n_drop (a->out_edges, b);
  find_n_drop (b->in_edges, a);
  if (a->g->kind == g_undirected)
    {
      find_n_drop (b->out_edges, a);
      find_n_drop (a->in_edges, b);
    }

}

/* return neighbours of node */
/* may contain duplicates in directed graph */
gnodeList
neighbours (gnode * node)
{
  return list_union (node->in_edges, node->out_edges);
}

/*
 * FLOWGRAPH 
 */

flow_node *
new_flow_node (int tn, irnode * insn)
{
  flow_node *new = (flow_node *) xmalloc (sizeof (struct xflow_node));
  new->tn = get_temp_number ();
  new->in = new_bitvector (new->tn);
  new->out = new_bitvector (new->tn);
  new->use = new_bitvector (new->tn);
  new->def = new_bitvector (new->tn);
  new->insn = insn;
  new->out_has_changed = 1;
  new->end_out = new->in->vector_size;
  new->at_end_of_block = 0;
  return new;
}

/* 
 * initialize use and def vectors for each node of graph
 */
void
set_initial_def_use (graph * g)
{

  ListElem t;
  gnode *node;

  ListElem e = get_first (g->nodes);
  while (e)
    {
      node = object (e);
      flow_node *f = (flow_node *) node->object;
      // set temporaries dest list
      t = get_first ((tempList) GET_IRCHILD (f->insn, 1));
      while (t)
	{
          temp *x = object(t);
	  set_bit (x->index, f->def);
	  // store this insn in the temp def list 
	  add (x->insn_usedefs, f->insn->holder);
	  t = get_next (t);
	}

      // set temporaries source list
      t = get_first ((tempList) GET_IRCHILD (f->insn, 2));
      while (t)
	{
          temp *x = object(t);
	  set_bit (x->index, f->use);
	  // store this insn in the temp use list 
	  add (x->insn_usedefs, f->insn->holder);
	  t = get_next (t);
	}

      // go to next node in graph
      e = get_next (e);
    }

}

/*
 * build interference matrix from liveness analysis made on i->fgraph
 * result is stored directly into i->igraph and vectors
 * this routine also modify regalloc structures worklist_moves 
 */
void
build_interference (interference * i)
{

  graph *g = i->fgraph;
  graph *h = i->igraph;
  int x;

  // for all nodes in flowgraph
  ListElem e = get_first (g->nodes);
  while (e)
    {
      gnode *node = object (e);
      flow_node *f = (flow_node *) node->object;
      tempList defs = (tempList) GET_IRCHILD (f->insn, 1);
      tempList uses = (tempList) GET_IRCHILD (f->insn, 2);
      ListElem use, def;
      int def_index;
      t_node *def_tnode;
      gnode *def_gnode;
      if (is_move (f->insn))
	{
	  // A MOVE INSTRUCTION
	  // put this insn into worklistMoves
	  add (worklistMovesSet, node);
	  f->current_set = worklist_set;
	  // update movelists for all concerned temporaries
	  def = get_first (defs);
	  while (def)
	    {
	      if (!find(i->movelists[get_temp_index_number (object (def))], node))
                add (i->movelists[get_temp_index_number (object (def))], node);
	      def = get_next (def);
	    }
	  use = get_first (uses);
	  while (use)
	    {
	      if (!find(i->movelists[get_temp_index_number (object (use))], node))
	        add (i->movelists[get_temp_index_number (object (use))], node);
	      use = get_next (use);
	    }
	  // put interference between defs and all liveout except move-src (uses) 
	  def = get_first (defs);
	  while (def)
	    {
	      def_index = get_temp_index_number (object (def));
	      def_gnode = (gnode *) object (get_ith (h->nodes, def_index));
	      def_tnode = (t_node *) (def_gnode->object);
	      for (x = 0; x < f->tn ; x++)
		{
		  if (get_bit (x, f->out))
		    {
		      int need_to_interfere = 1;
		      ListElem z = get_first (uses);
		      while (z)
			{
			  int z_index = get_temp_index_number (object (z));
			  if (z_index == x)
			    {
			      need_to_interfere = 0;
			      break;
			    }
			  z = get_next (z);
			}
		      // interference between defs and temp number x
		      if (need_to_interfere)
			{
			  add_interference_edge (def_tnode, (t_node *) ((gnode *) object (get_ith (h-> nodes, x)))-> object);
			}
		    }
		}
	      def = get_next (def);
	    }			/* end while(def) */

	}
      else
	{

	  // NOT A MOVE INSTRUCTION
	  // put interference between defs and all liveout
	  def = get_first (defs);
	  while (def)
	    {
	      def_index = get_temp_index_number (object (def));
	      def_gnode = (gnode *) object (get_ith (h->nodes, def_index));
	      def_tnode = (t_node *) (def_gnode->object);
	      for (x = 0; x < f->tn ; x++)
		{
		  if (get_bit (x, f->out))
		    {
		      // interference between defs and temp number x
		      add_interference_edge (def_tnode, (t_node *) ((gnode *) object (get_ith (h->nodes, x)))-> object);
		    }
		}
	      def = get_next (def);
	    }			/* end while(def) */
	}			/* end else */

      e = get_next (e);
    }

}


/*
 * compute static liveness for each node in flowgraph i->fgraph 
 * result is stored directly into i->fgraph
 */
void
compute_live (interference * i)
{

  graph *g = i->fgraph;

  set_initial_def_use (g);
  	/* initialize bitvector with def,use */

  void *ignore_block = 0;
	/* if a basic block can be ignored, we store its reference here */

  int first_pass = 1;
	/* computations are made completely on 1st pass */

  int fixed_point_reached;

  do
    {

      fixed_point_reached = 1;	
		// hope so, but will likely be changed below

      // for all nodes, in reverse order preferably
      ListElem e = get_last (g->nodes);
      while (e)
	{

	  gnode *node = object (e);
	  flow_node *f = (flow_node *) node->object;

  	  int i;
  	  uint32_t save_in, save_out;

	  if (ignore_block && f->insn->basic_block == ignore_block) 
		{ 
			/* if the last instruction of a basic block was not modified during previous iteration,
 			   all preceding nodes of the block don't need to be checked */
		  e = get_prev(e); 
		  continue; 
		}

	  f->out_has_changed = 0; 		// a priori, may be changed below
	  f->in_has_changed = 0; 		// a priori, may be changed below

  	  /* step 1 : live-out(node) = U live-in( successors ) */
	  /* Before we start, check if any of the successor has actually a changed */
	  /* live-in vector. If not, no need to go further */ 
 	  int in_has_changed = 0;
          ListElem succ = get_first (node->out_edges);
	  while(succ) {
            flow_node *g = ((flow_node *) ((gnode *) object (succ))->object);
	    if (g->in_has_changed) { in_has_changed = 1; break; }
            succ = get_next (succ);
	  }

	  if (in_has_changed)
  	    for (i = 0; i < f->in->vector_size; i++)
	      {
                save_out = f->out->v[i];
                f->out->v[i] = 0;
                succ = get_first (node->out_edges);
                while (succ)
              	  {
              	    flow_node *g = ((flow_node *) ((gnode *) object (succ))->object);
              	    f->out->v[i] |= g->in->v[i]; 
              	    succ = get_next (succ);
            	  }

                if (f->out->v[i] ^ save_out) 
		  { 
		    fixed_point_reached = 0; 
		    if (!f->out_has_changed) f->out_has_changed = i + 1; 
	  	    f->end_out = i + 1;
			/* we store start and end of modified part of out-vector */
		  } 
	      }

	  /* step 2 : live-in(node) = use(node) U (live-out(node) - def(node)) */
	  // this is done only if out vector was modified in current iteration, and 
	  // only on modified part of the vector (between f->has_changed - 1 and f->end_out)
	  if (first_pass) 
	   {
  	     for (i = 0; i < f->in->vector_size; i++)
	       	{
            	save_out = f->out->v[i];
            	save_in = f->in->v[i];
            	f->in->v[i] = f->use->v[i] | ( ~f->def->v[i] & save_out );
            	if (f->in->v[i] ^ save_in) { fixed_point_reached = 0; f->in_has_changed = 1; }
	   	}
	   } 
	  else if (f->out_has_changed)
	   {
  	     for (i = f->out_has_changed - 1; i < f->end_out; i++)
	       	{
            	save_out = f->out->v[i];
            	save_in = f->in->v[i];
            	f->in->v[i] = f->use->v[i] | ( ~f->def->v[i] & save_out );
            	if (f->in->v[i] ^ save_in) { fixed_point_reached = 0; f->in_has_changed = 1; }
	   	}
	   }

	  if (!first_pass && f->at_end_of_block && !f->in_has_changed)
		{
		  ignore_block = f->insn->basic_block;
			/* if the last instruction of a block is not modified, we ignore the whole block */
		}
  
	  // treat previous node	
	  e = get_prev (e);

	}

        first_pass = 0;

    }
  while (!fixed_point_reached);

  // build interference matrix
  build_interference (i);

}

/* 
 * creates a directed flowgraph from insn present in trace 
 */
graph *
trace_to_graph (trace tr)
{

  graph *g = new_graph ();

  gnode *previous = NULL, *current = NULL;

  // cleanup all insn internal pointers
  ListElem t = get_first (tr);
  while (t) 
    {
      BBList bblist = (BBList) object(t);
      ListElem bp = get_first (bblist);
      while (bp)
	{
	  BB *bb = (BB *) object (bp);
	  insnList insnl = bb->insnlist;
          ListElem ip = get_first (insnl);
          while (ip)
	    {
	      irnode *insn = object (ip);
	      insn->internal_pointer = 0;
	      ip = get_next(ip);
	    }
	  bp = get_next(bp);
	}
      t = get_next(t);
    }

  // here we go again... 
  t = get_first (tr);
  while (t) 
    {
      BBList bblist = (BBList) object(t);
      ListElem bp = get_first (bblist);
      while (bp)
	{
	  BB *bb = (BB *) object (bp);
	  insnList insnl = bb->insnlist;
          ListElem ip = get_first (insnl);
	  flow_node *fn;
          while (ip)
	    {
	      irnode *insn = object (ip);
	      insn->basic_block = (void *) bb;
	      insn->holder = ip;

	      // add this insn to graph
	      // If the insn is at beginning of the block, it may already
	      // have been inserted from another block. Check this, in order to avoid
	      // duplicates
	      if ((current = insn->internal_pointer) == NULL)
		    {
	      	      fn = new_flow_node (get_temp_number (), insn);
		      if (!get_next(ip)) fn->at_end_of_block = 1;
		      current = add_gnode (g, fn);
		      insn->internal_pointer = (void *)current;
		    }
	      // store an edge between previous and current
	      if (previous) add_edge (previous, current);
	      // change previous to current. This may be changed below, though
	      previous = current;
	      // if insn is last, perform some more treatments below
	      irnode *target;
	      gnode *gnode_target;
	      if ( ip == get_last(insnl) )
		{
		  switch (NODETYPE (insn))
		    {
		    case n_INSN_JUMP:
		      // find label-target insn in trace
		      target = label_location (object (get_first ((labelList) GET_IRCHILD (insn, 3))));
		      if (!target) { abort ();	/* target label should be somewhere around ! */  }
		      // add this insn to graph (if needed) and add an edge
		      gnode_target = target->internal_pointer;
		      if (gnode_target == NULL)
			{
			  fn = new_flow_node (get_temp_number (), target);
			  gnode_target = add_gnode (g, fn);
			  target->internal_pointer = (void *) gnode_target;
			}
		      add_edge (current, gnode_target);
		      // do not add fallthrough in next iteration, since the jump is always performed
		      previous = NULL;
		      break;
		    case n_INSN_CJUMP:
		      // find label-target insn in trace
		      target = label_location (object (get_first ((labelList) GET_IRCHILD (insn, 3))));
		      if (!target) { abort ();	/* target label should be somewhere around ! */  }
		      // add this insn to graph (if needed) and add an edge
		      gnode_target = target->internal_pointer;
		      if (gnode_target == NULL)
			{
			  fn = new_flow_node (get_temp_number (), target);
			  gnode_target = add_gnode (g, fn);
			  target->internal_pointer = (void *) gnode_target;
			}
		      add_edge (current, gnode_target);
		      // fallthrough must be added in next iteration, since the jump may not be done
		      break;
		    case n_INSN_RET:
		      // do not add fallthrough in next iteration, since end of control flow
		      previous = NULL;
		      break;
		    default:
		      // do nothing particular
		      break;
		    }
		}		/* end of test last insn */
                ip = get_next (ip);
	    }			/* end of for(insnl) */
            bp = get_next (bp);
	}			/* end of for(bblist) */
        t = get_next (t);
    }				/* end of for(t) */

  return g;
}


/* DEBUG output */
void
print_trace (trace t)
{
  keen_debug ("\n");
  keen_debug
    ("BL| L#|RULE#|CODE                                           |DEST               |SRC                |JMP\n");
  ListElem tp = get_first (t);
  while (tp)
    {
      BBList bblist = object (tp);
      ListElem bp = get_first (bblist);
      while (bp)
	{
	  BB *bb = object (bp);
	  print_insnList (bb->insnlist);
          bp = get_next (bp);
	}
      tp = get_next (tp);
    }
}

/*
 * create a new t_node
 */
t_node *
new_tnode (int index, temp * t, enum t_nodek current_wl)
{
  t_node *new = (t_node *) xcalloc (1, sizeof (t_node));
  new->t = t;
  new->index = index; // same as t
  new->current_wl = current_wl;
  return new;
}

void
optimize_temps_from_trace (trace tr)
{

  ListElem t = get_first (tr);
  while (t) 
    {
      BBList bblist = (BBList) object(t);
      ListElem bp = get_first (bblist);
      while (bp)
	{
	  BB *bb = (BB *) object (bp);
	  insnList insnl = bb->insnlist;
          ListElem ip = get_first (insnl);
          while (ip)
	    {
	      irnode *insn = object (ip);

      		ListElem th = get_first ((tempList) GET_IRCHILD (insn, 1));
      		while (th)
		{
          	temp *x = object(th);
          	inc_usedef( x );
	  	th = get_next (th);
		}

      		th = get_first ((tempList) GET_IRCHILD (insn, 2));
      		while (th)
		{
          	temp *x = object(th);
          	inc_usedef( x );
	  	th = get_next (th);
		}

	      ip = get_next(ip);
	    }
	  bp = get_next(bp);
	}
      t = get_next(t);
    }

  cleanup_temps();

}

/* 
 * perform static liveness analysis and produce interference objects
 */
interference *
build (trace t, interference * i)
{

  // produce directed flowgraph from trace t
  // each flow_node represents an insn in initial trace t
  i->fgraph = trace_to_graph (t);

  // initialize interference graph with t_node nodes, but with no edge so far
  // some t_nodes are precolored, and are marked so
  int p;
  for (p = 0; p < i->size; p++)
    {

      // for each existing temp...
      temp *t = get_temp (p);
      t->is_a_spill = 0;
      gnode *n_gnode;
      t_node *n_tnode;

      // clean insn lists
      t->insn_usedefs = newList();

      if ( p < size( registers_templist ) ) 
	{
	  n_tnode = new_tnode (p, t, wl_precolored);
	  i->color[p] = p;	/* reg index */
          keen_debug("Precoloring reg index %d (%s) with %d\n", p, get_temp_name(t), i->color[p]);
	  n_gnode = add_gnode (i->igraph, n_tnode);
	  n_tnode->g = n_gnode;	// keep track of holding gnode
	  ListElem e = _add (precolored_wl, n_tnode);
	  n_tnode->current_wl_holder = e;
	}
      else
	{
	  n_tnode = new_tnode (p, t, wl_initial);
	  i->color[p] = NOREG;	/* not mapped */
	  n_gnode = add_gnode (i->igraph, n_tnode);
	  n_tnode->g = n_gnode;	// keep track of holding gnode 
	  ListElem e = _add (initial_wl, n_tnode);
	  n_tnode->current_wl_holder = e;
	}
    }

  // compute liveness information from flowgraph 
  // and initialize interference graph, interference matrix, 
  // degree and movelists arrays as well 
  compute_live (i);

  return i;

}
