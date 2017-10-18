/* { dg-do "compile" } */

// forward declarations
int printf(char *, ...);
void abort();

void f( int [][] ); // { dg-error "empty array size" }



