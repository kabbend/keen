/* { dg-do "compile" } */

// forward declarations
int printf(char *, ...);
void abort();

signed x;
unsigned z;
signed signed y; // { dg-error "redundant" }



