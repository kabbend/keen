
/* { dg-do "run" } */
/* { dg-options "-w" } */

union a { int x; int y; };
union b { union a a_union; } global;

int
main ()
{
 union b local;
 global.a_union.y = 10;
 local.a_union.y = global.a_union.y; 
 if (local.a_union.y != 10) abort();
 return 0;
}

