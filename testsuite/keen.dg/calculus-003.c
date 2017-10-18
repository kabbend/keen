/* { dg-do "run" } */
/* { dg-options "-w" } */


int x;

int 
main()
{

  x = 10;
  printf("x=%d\n",x);
  x = x << 1;
  printf("x=%d\n",x);
  x = x << 1;
  printf("x=%d\n",x);
  x = x << 1;
  printf("x=%d\n",x);
  if (x != 80) abort();

  x = -10;
  printf("x=%d\n",x);
  x = x << 1;
  printf("x=%d\n",x);
  x = x << 1;
  printf("x=%d\n",x);
  x = x << 1;
  printf("x=%d\n",x);
  if (x != -80) abort();

  x = 1000;
  int y = 1;
  printf("x=%d\n",x);
  x = x << y;
  printf("x=%d\n",x);
  x = x << (y+1);
  printf("x=%d\n",x);
  if (x != 8000) abort();

  return 0; 
}
