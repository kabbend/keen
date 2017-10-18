// { dg-do compile }
// { dg-options "-w" }

int printf( char *f, ... );
void abort();

typedef int label;

int f( int label ); // { dg-error "" }


