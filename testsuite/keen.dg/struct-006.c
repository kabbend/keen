
/* { dg-do "run" } */
/* { dg-options "-w" } */

int main()
{

 // forward declaration
 struct b;

 struct a { 
   int x; 
   int y;
   struct b *z; /* pointer to incomplete type allowed */ 
 } r1, r2;

 struct b {
   int x; /* same member name as in 'a' is allowed */
   struct a complete; /* allowed, as 'a' is complete */
   struct a *pointer;
 } r3;

 r1.z = &r3;
 r3.pointer = &r1;
 
 r1.y = 1000;
 r3.x = 2000;

 int sum = (*r3.pointer).y + (*r1.z).x;

 printf("sum %d\n",sum);

 if (sum != 3000) abort();

 return 0;

}

