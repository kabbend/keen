
/* { dg-do "run" } */
/* { dg-options "-w" } */

union a { int x; int y; };

union a global_ret;

union a g()
{
 union a ret;
 ret.x = 6;
 return ret; 
}

union a f(int x, int y)
{
 union a ret;
 ret.y = x+y;
 return ret; 
}

int
main ()
{
 union a z1 = f(3,2);
 printf("z1.x=%d,z1.y=%d\n", z1.x, z1.y);
 global_ret = g();
 printf("global_ret.x=%d,global_ret.y=%d\n", global_ret.x, global_ret.y);
 if (z1.x + 1 != global_ret.x ) abort();
 if (z1.y + 1 != global_ret.y ) abort();
 return 0;
}

