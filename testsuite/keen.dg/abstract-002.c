/* { dg-do compile } */
/* { dg-options "-w" } */

// accept function with (void)

void 
f (int) /* { dg-error "missing" } */
{
 return;
}


