
/* { dg-do "compile" } */

int
main()
{


 struct a { int x; int y; } a_struct, *pointer_to_struct;

 a_struct.x = 1;
 a_struct.y = 2;
 pointer_to_struct = &a_struct;

 /* dereference pointer to struct using -> syntax */ 
 //int z = pointer_to_struct->y;
 int t = a_struct->y; /* { dg-error "not a valid" "" } */
 
 printf("%d\n",z);
 if (z != 2) abort();

 return 0;

}

