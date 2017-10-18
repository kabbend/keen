/* { dg-do run } */
/* { dg-options "-w" } */



int
main()
{
  int x[ 1 + 1 + 1 ][ 1 + 1 + 1] = { { 2, 5, 7 }, {1, 3, 5 }, { 0, 0, 0 } };
  printf("x[0][0]=%d, x[1][0]=%d, x[2][2]=%d\n", x[0][0], x[1][0], x[2][2] );
  if (x[0][1-1] != 2) abort(); 
  if (x[1*1][0] != 1) abort(); 
  if (x[2][2+0] != 0) abort(); 
  return 0;
}	





