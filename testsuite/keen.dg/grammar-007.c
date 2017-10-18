// { dg-do compile }
// { dg-options "-w" }

int printf( char *f, ... );
void abort();

typedef int label;

int 
main()
{
  const i;
  volatile j;
  const long int k;
  int long const l;
}


