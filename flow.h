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

#ifndef _FLOW_H_
#define _FLOW_H_

#include <stdint.h>
#include "ir.h"
#include "canon.h"
#include "list.h"
#include "reg.h"

/*
 * Bitvector declarations
 */
typedef struct xbitvector *bitvector;

struct xbitvector
{
  int bit_size;                 /* number of bits in vector */
  int vector_size;              /* size of vector in 4-bytes (uint32_t) units */
  uint32_t *v;         		/* 4-bytes vector */
};

bitvector new_bitvector (int size);
void free_bitvector (bitvector b);

void set_bit (int bit_number, bitvector b);

extern uint32_t masks[];
#define get_bit(bit_number,bit_vector) ((bit_vector)->v[(bit_number)>>5] & masks[31-((bit_number)%32)])
	/* get_bit is defined as a macro to improve performance */

void and_vector (bitvector dest, bitvector src);
void or_vector (bitvector dest, bitvector src);
void xor_vector (bitvector dest, bitvector src);
void cmp_vector (bitvector dest);
void zero_vector (bitvector v);
int vector_is_null (bitvector v);

/*
 * Graph declarations
 */
typedef List gnodeList;

typedef struct xgnode
{
  struct xgraph *g;		/* pointer to holding graph */
  void *object;			/* actual object held by graph node */
  gnodeList in_edges;		/* incoming egdes (used only in case of undirected graph) */
  gnodeList out_edges;		/* outgoing edges */
} gnode;

typedef struct xgraph
{
  enum graph_kind
  { g_directed, g_undirected } kind;
  gnodeList nodes;
} graph;

graph *new_graph ();		/* creates a new empty directed graph */
graph *new_undirected_graph ();	/* creates a new empty undirected graph */
gnode *add_gnode (graph * g, void *object);	/* creates and returns a new gnode, pointing to given object */
void add_edge (gnode * a, gnode * b);	/* creates a new edge between a and b nodes */
void remove_edge (gnode * a, gnode * b);	/* removes an existing edge between a and b nodes */
int linked (gnode * a, gnode * b);	/* returns 1 if nodes are linked by an edge */
gnodeList neighbours (gnode * node);	/* return neighbours of node in graph */

/*
 * Flowgraph and Interference 
 */

// flowgraph node, used in interference fgraph below
// could be hidden I suppose. I keep it here until further notice
typedef struct xflow_node
{
  int tn;			/* number of temporaries in bitvectors */
  bitvector in;			/* live-in bitvector */
  bitvector out;		/* live-out bitvector */
  bitvector use;		/* use temporaries bitvector */
  bitvector def;		/* def temporaries bitvector */
  int out_has_changed;		/* flag if out vector was modified during current iteration */
  int in_has_changed;		/* flag if in vector was modified during current iteration */
  int end_out;			/* max. index of the modified part of the out vector */
  irnode *insn;			/* insn represented by the flow node */
  int at_end_of_block;		

  enum flowk
  {
    coalesced_set,
    constrained_set,
    frozen_set,
    worklist_set,
    active_set
  } current_set;		/* if insn is move, current move-set */

  ListElem current_set_holder;	/* position in previous list */

} flow_node;

// temporary node, used in various interference data-structures
// a t_node represents a temporary during register allocation phase
typedef struct xt_node
{
  int index;			/* index of the tnode */
  gnode *g;			/* graph node holding the t_node in interference graph */
  temp *t;			/* temporary */
  accessor *spilled;            /* accessor (on frame) when node is spilled */
  long spill_value;		/* store spill value when node is candidate for spilling */

  enum t_nodek
  {
    wl_precolored,
    wl_initial,
    wl_simplify,
    wl_freeze,
    wl_spill,
    wl_spilled,
    wl_coalesced,
    wl_colored,
    wl_select
  } current_wl;			/* current worklist */

  ListElem current_wl_holder;	/* location in previous list */
 
} t_node;

typedef struct xinterference
{
  trace t;			/* initial trace                                                                */
  graph *fgraph;		/* flow_node flowgraph (directed)                                               */
  graph *igraph;		/* t_node interference graph (undirected)                                       */
  int size;			/* number of t_node in following data-structures :                              */
  bitvector *imatrix;		/* interference matrix, one bitvector per tnode                                 */
  gnodeList *movelists;		/* array of lists, to store lists of move insn for each tnode (when applicable) 
				   each list stores pointers to gnode in fgraph above                           */
  int *degree;			/* degree of each node                                                          */
  int *color;			/* color assigned to each node (register index)                                 */
  int *alias;			/* t_node index of the alias, when coalesced                                    */
} interference;


interference *build (trace t, interference * i);	/* returns interference information from trace */

void print_trace ();

#endif
