
/* { dg-do "run" } */
/* { dg-options "-w"  }  */

int
main()
{


 union a {
  int x;
 } a_union, *pointer_to_union;

 a_union.x = 1;
 pointer_to_union = &a_union;

 /* dereference pointer to union using -> syntax */ 
 int z = pointer_to_union->x;
 
 printf("%d\n",z);
 if (z != 1) abort();

 return 0;

}

