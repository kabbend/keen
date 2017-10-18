/* { dg-do "run" } */
/* { dg-options "-w" } */


int x;

int 
main()
{

  x = 100;
  printf("x=%d\n",x);
  x = x >> 1;
  printf("x=%d\n",x);
  x = x >> 1;
  printf("x=%d\n",x);
  x = x >> 1;
  printf("x=%d\n",x);
  if (x != 12) abort();

  x = -100;
  printf("x=%d\n",x);
  x = x >> 1;
  printf("x=%d\n",x);
  x = x >> 1;
  printf("x=%d\n",x);
  x = x >> 1;
  printf("x=%d\n",x);
  if (x != -13) abort();

  x = 1000;
  int y = 1;
  printf("x=%d\n",x);
  x = x >> y;
  printf("x=%d\n",x);
  x = x >> (y+1);
  printf("x=%d\n",x);
  if (x != 125) abort();

  return 0; 
}
