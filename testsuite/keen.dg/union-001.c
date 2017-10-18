
/* { dg-do "compile" } */

int main()
{

 union a;
  
 union a { 
   int *x,z; 
   unsigned int **y;
   int a[][2]; /* { dg-error "incomplete type" "" } */
 } t;

}

