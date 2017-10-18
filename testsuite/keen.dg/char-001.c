// { dg-do "run" }
// { dg-options "-w" }

char x = 3;
char y = 5;

struct { char x; char y; } S = { 3 , 5 };

int 
main()
{
 int z = x + y;
 printf("%d %d %d\n",x,y,z);
 if (z != 8) abort();

 char a = 3;
 char b = 5;
 int c = a + b;
 printf("%d %d %d\n",a,b,c);
 if (c != 8) abort();

 c = S.x + S.y;
 printf("%d %d %d\n",S.x,S.y,c);
 if (c != 8) abort();
 
}
