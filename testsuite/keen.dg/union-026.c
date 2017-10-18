
/* { dg-do "run" } */
/* { dg-options "-w" } */

union a { int x; int y; char c[3][7][9]; };

union a global_ret;

int g(union a param )
{
 // test copy param for union
 printf("param.x=%d,param.y=%d\n", param.x, param.y);
 if (param.x != 2) abort();
}

int
main ()
{
 union a z;
 z.x = 2;
 printf("z.x=%d,z.y=%d\n", z.x);
 g(z);
 
 return 0;
}


