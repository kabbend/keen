/* { dg-do run } */
/* { dg-options "-w" } */

// forward declarations
int printf(char *, ...);
void abort();
// struct st is not declared yet but pointer on it is allowed. 
// but anyway struct st is not declared just inside function, though. Maybe we
// should change this
int f( struct st *, int );  

struct st { int x; int y[3]; };

int
f( struct st *s, int x)
{
 return s->y[x];
}

int
main(int argc, char **argv)
{
 struct st a;
 a.x = 1;
 a.y[0]=8; a.y[1]=9; a.y[2]=10;
 if (f(&a,1) != 9) abort();
 printf("value=%d\n",f(&a,1));
 return 0;
}


