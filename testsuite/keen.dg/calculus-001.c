
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

  check( x/y, 7 );
  check( x%y, 2 );
  check( x+y, 34 );
  check( x&&y, 1 );
  check( x&&(!y), 0 );
  check( x||y, 1);
  check( 0x11 & 0x10, 16);
  check( 0x01 | 0x10, 17);

}

