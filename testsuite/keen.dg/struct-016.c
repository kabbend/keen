
/* { dg-do "run" } */
/* { dg-options "-w"  }  */

int
main()
{


 struct a {
  int x;
  int y[3];
 } a_struct, *pointer_to_struct;

 a_struct.x = 99;
 a_struct.y[0] = 100;
 a_struct.y[1] = 101;
 a_struct.y[2] = 102;

 pointer_to_struct = &a_struct;

 /* dereference pointer to struct using -> syntax */ 
 int z = pointer_to_struct->x + pointer_to_struct->y[1];
 
 printf("%d\n",z);
 if (z != 200) abort();

 return 0;

}

