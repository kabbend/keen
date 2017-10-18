
/* { dg-do "run" } */
/* { dg-options "-w" } */

int main()
{

 union a { 
   int x,z; 
   unsigned int **y;
   int a[1][2];
 } t;

 t.x = 1;
 printf("t.x = %d\n", t.x);

 t.z = 3;
 printf("t.x = %d\n", t.x);
 printf("t.z = %d\n", t.z);
 if (t.x != 3 || t.z != 3) abort();

 t.a[0][0] = 5;

 if (t.x != 5) abort();
 return 0;

}

