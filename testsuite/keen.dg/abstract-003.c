/* { dg-do compile } */
/* { dg-options "-w" } */

void 
f (int x, void, int y) /* { dg-error "syntax error" } */
{
 return;
}


