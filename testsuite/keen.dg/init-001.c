// { dg-do "compile" }
// { dg-options "-w" }

struct incomplete_struct ARRAY[] = { 	// { dg-error "incomplete" }
  { 1, 2, 3 },
  { 1, 2, 3 }
}; 	

