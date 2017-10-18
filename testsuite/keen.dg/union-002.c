
/* { dg-do "compile" } */

int main()
{

 union a;
  
 union a { 
   int *x,z; 
   unsigned int **y;
   int a[1][2]; 
   signed int z; /* { dg-error "is redeclared" "" } */
 } t;

}

