/* { dg-do run } */
/* { dg-options "-w" } */


struct internal { char X; char Y; };
struct { char x; char z; char y; struct internal t; int w; } S[2] = { 
	{ 3,  9, 6, { 8, 12 }, 1 } ,
	{ 4, 10, 7, { 9, 13 }, 2 } 
  };


int
main()
{

  printf("S[0].x = %d\nS[0].w = %d\n",S[0].x, S[0].w);
  if (S[0].x != 3 || S[0].w != 1) abort();
  printf("S[1].t.X = %d\nS[1].t.Y = %d\n",S[1].t.X, S[1].t.Y);
  if (S[1].t.X != 9 || S[1].t.Y != 13) abort();
  return 0;  
}



