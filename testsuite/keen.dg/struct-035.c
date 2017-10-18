/* { dg-do run } */
/* { dg-options "-w" } */


struct internal { int X; int Y; };

int
main()
{
  struct { int x; int z; int y; struct internal t; int w; } S = { 3, 9, 6, { 8, 12 }, 1 }; 

  printf("S.x = %d\nS.w = %d\n",S.x, S.w);
  if (S.x != 3 || S.w != 1) abort();
  printf("S.t.X = %d\nS.t.Y = %d\n",S.t.X, S.t.Y);
  if (S.t.X != 8 || S.t.Y != 12) abort();
  return 0;  
}



