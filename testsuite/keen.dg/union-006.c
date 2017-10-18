
/* { dg-do "run" } */
/* { dg-options "-w" } */

int main()
{

 // forward declaration
 union b;

 union a { 
   int x; 
   int y;
   union b *z; /* pointer to incomplete type allowed */ 
 } r1, r2;

 union b {
   int x; /* same member name as in 'a' is allowed */
   union a complete; /* allowed, as 'a' is complete */
   union a *pointer;
 } r3;

 r1.z = &r3;
 r3.pointer = &r1;

 int sum = (*r3.pointer).y + (*r1.z).x;
 printf("sum %d\n",sum);

 return 0;

}

