
/* { dg-do "run" } */
/* { dg-options "-w"  }  */

int
main()
{


 union a { int x; int y[3]; } a_union, **pointer_to_pointer_to_union, *pointer_to_union;

 a_union.x = 99;
 a_union.y[0] = 100;
 a_union.y[1] = 101;
 a_union.y[2] = 102;

 pointer_to_union = &a_union;

 int z1= pointer_to_union->y[2];

 pointer_to_pointer_to_union = &pointer_to_union;

 int z2 = (*pointer_to_pointer_to_union)->y[2];
 
 printf("%d %d\n",z1,z2);
 if (z1 != z2 && z1 != 102) abort();

 return 0;

}

