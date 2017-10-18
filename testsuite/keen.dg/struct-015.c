
/* { dg-do "run" } */
/* { dg-options "-w"  }  */

int
main()
{


 struct a {
  int x;
 } a_struct, *pointer_to_struct;

 a_struct.x = 1;
 pointer_to_struct = &a_struct;

 /* dereference pointer to struct using -> syntax */ 
 int z = pointer_to_struct->x;
 
 printf("%d\n",z);
 if (z != 1) abort();

 return 0;

}

