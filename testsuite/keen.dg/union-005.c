
/* { dg-do "run" } */
/* { dg-options "-w" } */

int main()
{

 union a { 
   int x; 
   int y;
   union a *z; /* recursive pointer is allowed */
 } r1, r2;

 r1.z = &r2;
 r2.y = 4;
 int x = (*r1.z).y;
 int y = (*r1.z).x;

 printf("sum %d\n",x+y);

 if (x+y != 8) abort();

 return 0;

}

