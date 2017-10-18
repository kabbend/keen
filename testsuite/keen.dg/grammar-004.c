// { dg-do run }
// { dg-options "-w" }

int printf( char *f, ... );
void abort();

typedef int label;

label x;		

struct {
  int an_int;
  int a, b, c;
  label *label[3];		
} S;

int 
main()
{
 S.label[1] = &x;
 printf("S.label[1] = %x\n", S.label[1] );
 return 0;
}


