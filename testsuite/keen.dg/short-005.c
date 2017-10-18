// { dg-do "run" }
// { dg-options "-w" }

struct { long int x; int y; } S = { 0x10000001, 0x10000001 };

int
main()
{
 short y = *(&S.x); 	// take lower part of long int
 printf("%d\n",y);
 if (y != 1) abort();
 return 0;
}
