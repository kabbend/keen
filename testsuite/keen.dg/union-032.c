/* { dg-do run } */
/* { dg-options "-w" } */


struct internal { int X; int Y; };

union { struct internal t; int x; int z; int y; int w; } S = { { 8, 12 } }; 

int
main()
{
  if (S.t.X != 8 || S.t.Y != 12) abort();
  return 0;  
}



