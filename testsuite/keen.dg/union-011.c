
/* { dg-do "compile" } */

int
main()
{


 union a { int x; int y; } a_union, *pointer_to_union;

 a_union.x = 1;
 a_union.y = 2;
 pointer_to_union = &a_union;

 /* dereference pointer to union using -> syntax */ 
 //int z = pointer_to_union->y;
 int t = a_union->y; /* { dg-error "not a valid" "" } */
 
 printf("%d\n",z);
 if (z != 2) abort();

 return 0;

}

