// { dg-do run }
// { dg-options "-w" }

int printf( char *f, ... );
void abort();

typedef int label;

struct label {
  int a, b, c;
} S1;

int 
main()
{
 struct label S2;
 S1.a = 3;
 S2 = S1;
 if (S2.a != 3) abort();
 return 0;
}


