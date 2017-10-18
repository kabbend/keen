
/* { dg-do "run" } */
/* { dg-options "-w"  }  */

int
main()
{


 union a {
  int x;
  int y[3];
 } a_union, *pointer_to_union;

 a_union.x = 99;
 a_union.y[0] = 100;
 a_union.y[1] = 101;
 a_union.y[2] = 102;

 pointer_to_union = &a_union;

 /* dereference pointer to union using -> syntax */ 
 int z = pointer_to_union->x + pointer_to_union->y[1];
 
 printf("%d\n",z);
 if (z != 201) abort();

 return 0;

}

