/* { dg-do run } */
/* { dg-options "-w" } */


int x[] = { 0x001 & 0x010 , 0x1100 | 0x0011 , !1 };

int
main()
{
  printf("x[0]=%d, x[1]=%d, x[2]=%d\n", x[0], x[1], x[2] );
  if (x[0] != 0) abort(); 
  if (x[1] != 4369) abort(); 
  if (x[2] != 0) abort(); 
  return 0;
}	





