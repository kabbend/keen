// { dg-do compile }
// { dg-options "-w" }

int printf( char *f, ... );
void abort();

typedef int label;

struct { int x; } label; // { dg-error "" } 

int 
main()
{
}


