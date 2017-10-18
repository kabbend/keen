
/* { dg-do "run" } */
/* { dg-options "-w" } */

struct a { int x; int y; };
struct b { struct a a_struct; } global;

int
main ()
{
 struct b local;
 global.a_struct.x = 8;
 global.a_struct.y = 10;
 local.a_struct.x = global.a_struct.x; 
 local.a_struct.y = global.a_struct.y; 
 printf("local.x=%d,local.y=%d\n",local.a_struct.x,local.a_struct.y);
 if (local.a_struct.x!=8 || local.a_struct.y != 10) abort();
 return 0;
}

