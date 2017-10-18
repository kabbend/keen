
/* { dg-do "run" } */
/* { dg-options "-w" } */

void 
check(int value, int expect)
{
 printf("value=%d\n",value);
 if (value != expect) abort(); 
}

int 
main()
{
  int x = 30;
  int y = 4;
  int z = 2;

  z = ~y + x * ~z;
  check( z, -95 );
 
}

