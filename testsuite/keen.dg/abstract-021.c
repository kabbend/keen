/* { dg-do "compile" } */

// forward declarations
int printf(char *, ...);
void abort();

void f( int [10][] ); // { dg-error "empty array size" }



