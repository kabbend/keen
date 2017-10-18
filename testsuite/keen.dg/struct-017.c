
/* { dg-do "run" } */
/* { dg-options "-w"  }  */

int
main()
{


 struct a { int x; int y[3]; } a_struct, **pointer_to_pointer_to_struct, *pointer_to_struct;

 a_struct.x = 99;
 a_struct.y[0] = 100;
 a_struct.y[1] = 101;
 a_struct.y[2] = 102;

 pointer_to_struct = &a_struct;

 int z1= pointer_to_struct->y[2];

 pointer_to_pointer_to_struct = &pointer_to_struct;

 int z2 = (*pointer_to_pointer_to_struct)->y[2];
 
 printf("%d %d\n",z1,z2);
 if (z1 != z2 && z1 != 102) abort();

 return 0;

}

