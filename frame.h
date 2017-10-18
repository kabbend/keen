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

#ifndef _FRAME_H_
#define _FRAME_H_

#include "temp.h"
#include "ir.h"
#include "type.h"

/*
 * complete definition of frame and accessor are to be done
 * in (arch)/frame_(arch).h file, as they depend on the machine
 */
typedef struct xframe frame;
typedef struct xaccessor accessor;

typedef struct xdata {
 int mode;
 int padding;
 char *value;
} data;

typedef List dataList;
typedef List accessorList;

data *create_data (int mode, int offset, char *value );

/* forge prologue/epilogue label names for a new frame, 
 * based on given name (typically the name of the C function) 
 */
char *prologue_label_name (char *name);
char *epilogue_label_name (char *name);

/* debug function to print frame details */
void print_frame (frame *);

/*
 * create a new frame with appropriate entry/exit labels. Frame
 * size or content is not known at that time
 */
frame *new_frame (label * entry_label, label * epilogue_label);

/* return frame labels */
label *get_frame_label (frame * f);
label *get_frame_label_epilogue (frame * f);

accessorList get_formals(frame *f);
accessorList get_locals(frame *f);

accessor *new_param (int mode, frame * f, int size);
accessor *new_var (int mode, frame * f, int escape, int size);
accessor *new_spill (int mode, frame * f);
accessor *new_global_var (int mode, label * l, int size);
accessor *new_data (int mode, label * l, int size);

/* return most appropriate mode for the type, 
 * depending on machine arch 
 */
int get_type_natural_mode (type * t);

/* return size (in bytes) of the type
 * equivalent in general to the natural mode, 
 * except for struct/array/union types
 */
int get_type_size (type *t);

/* return size (in bytes) of the type with
 * minimal alignment. This is used to compute
 * struct members offsets and struct total size
 */
int get_aligned_type_size (type *t);

/* return an irnode IR representing the address of (a) in memory
 * (typically an operation made of an offset applied on frame pointer register)
 * (a) is supposed in memory, not register
 */
irnode *getAddress (accessor * a);

/* return an irnode IR representing content of (a)
 * (for instance a MEM irnode based on address of a)
 * (a) can be in register or memory
 */
irnode *getMemoryExp (accessor * a);

/* return label for accessor (a) (supposed in data section) 
 * FIXME used for debug only, should be removed in the future */
label *get_access_label (accessor * a);

/* return an irnode IR representing the prologue of the frame, 
 * followed by the body */
irnode *generate_prologue (frame * f, irnode * body);

/* return an irnode IR representing the epilogue of the frame */
irnode *generate_epilogue (frame * f);

/* generate one fetch/store instruction irnode for spilling registers.
 * the stack-space is supposed reserved via accessor (a), the temporary 
 * to spill from/to is t
 */
irnode *generate_fetch ( accessor * a, temp * t );
irnode *generate_store ( accessor * a, temp * t );

/* rewrite prologue after register allocation and spilling phase,
 * so frame size is in accordance with newly spilled regs and new
 * required stack-space
 */
void    update_prologue   (frame * f);

/* store into frame a pointer to the very stack-allocation instruction, 
 * (typically "sub $offset, %esp") so this instruction can be easily 
 * rewritten or replaced during update_prologue() 
 */
void set_frame_stack_hook ( frame * f, irnode * insn );

#endif
