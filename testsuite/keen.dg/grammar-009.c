// { dg-do compile }
// { dg-options "-w" }

int printf( char *f, ... );
void abort();

typedef int label;

label(); // { dg-error "" }


