/* { dg-do run } */
/* { dg-options "-w" } */


int x[] = { 1 + 20 - 3, 2 * 3 * 4, !3 + 5 };

int
main()
{
  printf("x[0]=%d, x[1]=%d, x[2]=%d\n", x[0], x[1], x[2] );
  if (x[0] != 18) abort(); 
  if (x[1] != 24) abort(); 
  if (x[2] != 5) abort(); 
  return 0;
}	





