
/* { dg-do "compile" } */

int main()
{

 union a;

 union a;

 union a { 
   int *x,z; 
   unsigned int **y;
   int a[1][2];
 } t;

 union a;

}

