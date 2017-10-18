// { dg-do run }

int printf( char *f, ... );
void abort();

typedef int label;

label x;		

struct {
  int an_int;
  int a, b, c;
  short label;		// { dg-warning "has same name" }
} S;

int 
main()
{
 S.label = 3;
 label X = 2;
 x = 5; 
 printf("S.label=%d,X=%d,x=%d\n",S.label,X,x);
 if (S.label != 3 || X != 2 || x != 5) abort();
 return 0;
}


