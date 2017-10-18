
/* { dg-do "compile" } */

int main()
{

 struct a;
  
 struct a { 
   int *x,z; 
   unsigned int **y;
   int a[][2]; /* { dg-error "incomplete type" "" } */
 } t;

}

