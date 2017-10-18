
/* { dg-do "run" } */
/* { dg-options "-w"  }  */

int
main()
{

 union a {
  int x[2];
  int y[2];
 };

 union a an_union;

 an_union.x[0] = 0;
 an_union.x[1] = 3;

 int z = an_union.y[1];

 printf("%d\n",z);

 if (z != 3) abort();

 return 0;

}

