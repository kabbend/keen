
/* { dg-do "run" } */
/* { dg-options "-w" } */

int main()
{

 // forward declaration
 struct b;

 struct a { 
   int x; 
   int y;
   struct b *z[2]; /* array */ 
 } an_struct_a;

 struct b {
   int b; /* same member name as struct itself is allowed */
   struct a complete[2][2]; /* allowed, as 'a' is complete */
   struct a *pointer[2];
 } array_of_struct_b[2];

 an_struct_a.x = 10;
 an_struct_a.y = 20;
 an_struct_a.z[0] = &array_of_struct_b[1];
 an_struct_a.z[1] = &array_of_struct_b[0];

 (*an_struct_a.z[0]).b = 100;
 (*an_struct_a.z[1]).b = 200;

 printf("array_of_struct_b[0].b=%d\n", array_of_struct_b[0].b);

 if ( array_of_struct_b[0].b != 200) abort();

 return 0;

}

