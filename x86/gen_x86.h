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

#ifndef _GEN_X86_H_
#define _GEN_X86_H_

label *x86_gen_label (irnode * node);
label *x86_gen_jump_unique_name (irnode * node);
label *x86_get_false_label (irnode * node);
label *x86_get_true_label (irnode * node);
temp *x86_gen_cc_reg (irnode * node);
temp *x86_gen_temp (irnode * node);
char *x86_gen_func_globl (irnode * node);
char *x86_gen_get_data (irnode * node);
char *x86_gen_data_section (irnode * node);
char *x86_gen_data_entry (irnode * node);
irnodeList x86_gen_frame_stack (irnode * node, int rule_id);
irnodeList x86_gen_call (irnode * node, int rule_id);
irnodeList x86_gen_call_assign (irnode * node, int rule_id);
irnodeList x86_set_callee_save (irnode * node, int rule_id);
irnodeList x86_get_callee_save (irnode * node, int rule_id);
irnodeList x86_gen_add_const (irnode * node, int rule_id);
temp *x86_move_mem_temp (irnode * node);
temp *x86_move_temp (irnode * node);

temp *nopt(irnode *node /* ignored */);
temp *n(irnode *node /* ignored */);
temp *n1B(irnode *node /* ignored */);
temp *n2B(irnode *node /* ignored */);
temp *n4B(irnode *node /* ignored */);

#endif
