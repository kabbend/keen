// { dg-do "run" }
// { dg-options "-w" }

struct { long int x; int y; } S = { 0x10000101, 0x10000101 };

int
main()
{
 char y = *(&S.x); 	// take lower part of long int
 printf("%d\n",y);
 if (y != 1) abort();
 return 0;
}
