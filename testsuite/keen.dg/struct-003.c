
/* { dg-do "run" } */
/* { dg-options "-w" } */

int main()
{

 struct a { 
   int x,z; 
   unsigned int **y;
   int a[1][2];
 } t;

 t.x = 1;
 t.z = 3;
 t.a[0][0] = 5;
 t.a[0][1] = 6;

 printf("%d,%d,%d,%d\n", t.x, t.z, t.a[0][0],t.a[0][1] );

 if (t.x != 1 || t.z != 3 || t.a[0][0] != 5 || t.a[0][1] != 6) abort();
 return 0;

}

