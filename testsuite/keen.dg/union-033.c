/* { dg-do run } */
/* { dg-options "-w" } */


struct internal { int X; int Y; };

int
main()
{
  union { struct internal t; int w; } S = { { 8, 12 } }; 
  if (S.t.X != 8 || S.t.Y != 12) abort();
  return 0;  
}



