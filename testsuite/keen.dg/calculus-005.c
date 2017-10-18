/* { dg-do "run" } */
/* { dg-options "-w" } */


int x;

int 
main()
{

  x = 100;
  int y;
  printf("x=%d\n",x);
  y = x >> 1;
  printf("x=%d\n",x);
  printf("y=%d\n",y);
  x = y;
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
  y = 1;
  printf("x=%d\n",x);
  x = x >> y;
  printf("x=%d\n",x);
  x = x >> (y+1);
  printf("x=%d\n",x);
  if (x != 125) abort();

  x = 0x101010;
  y = 0x010101;
  x = x ^ y;
  printf("x=%d\n",x);
  if (x != 1118481) abort();
 
  x = 0x101010;
  y = 0x110101;
  x = x ^ y;
  printf("x=%d\n",x);
  if (x != 69905) abort();

  return 0; 
}
