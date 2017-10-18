// { dg-do "run" }
// { dg-options "-w" }

signed short int x = 3;
signed short int y = 5;

struct { short int x; short int y; } S = { 3 , 5 };

short 
f( short x, short y )
{
  return x + y;
}

int 
main()
{
 int z = x + y;
 printf("%d %d %d\n",x,y,z);
 if (z != 8) abort();

 signed short int a = 3;
 signed short int b = 5;
 int c = a + b;
 printf("%d %d %d\n",a,b,c);
 if (c != 8) abort();

 c = S.x + S.y;
 printf("%d %d %d\n",S.x,S.y,c);
 if (c != 8) abort();

 printf("f(3,5)=%d\n",f(3,5));
 c = f ( 3, 5 );
 if (c != 8) abort();

 a = 3; b = 5;
 c = f ( a, b );
 printf("f(a,b)=%d\n",c);
 if (c != 8) abort();
 
}
