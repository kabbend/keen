/* { dg-do run } */
/* { dg-options "-w" } */


struct internal { int X; int Y; };

int
main()
{

  union { struct internal t; int x; int z; int y; int w; } S[2] = { 
	{ { 8, 12 } } ,
	{ { 9, 13 } } 
  };

  printf("S[1].t.X = %d\nS[1].t.Y = %d\n",S[1].t.X, S[1].t.Y);
  if (S[1].t.X != 9 || S[1].t.Y != 13) abort();
  return 0;  
}



