
/* { dg-do "run" } */
/* { dg-options "-w" } */

union a { int x; int y; };

union a f(int x)
{
 union a ret;
 ret.y = x + 1;
 return ret; 
}

int
main ()
{
 union a z1 = f(3);
 printf("z1.x=%d,z1.y=%d\n", z1.x, z1.y);
 union a z2 = f(4);
 printf("z2.x=%d,z2.y=%d\n", z2.x, z2.y);
 if (z1.x + 1 != z2.x ) abort();
 if (z1.y + 1 != z2.y ) abort();
 return 0;
}

