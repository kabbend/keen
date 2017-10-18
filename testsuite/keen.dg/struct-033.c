/* { dg-do run } */
/* { dg-options "-w" } */


struct { int x; int z; int y; } S = { 3, 9, 6 }; 

int
main()
{
  printf("S.x = %d\nS.y = %d\n",S.x, S.y);
  if (S.x != 3 || S.y != 6) abort();
  return 0;  
}



