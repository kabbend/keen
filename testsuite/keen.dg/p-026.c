/* { dg-do compile } */
/* { dg-options "-w" } */

int y = 0;
int x[] = { 1 + y - 3, 2 * 3 * 4, !3 + 5 }; // { dg-error "constant" }






