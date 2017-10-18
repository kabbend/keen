
/* { dg-do "compile" } */

int main()
{

 struct a;
  
 struct a { 
   int *x,z; 
   unsigned int **y;
   int a[1][2]; 
   signed int z; /* { dg-error "is redeclared" "" } */
 } t;

}

