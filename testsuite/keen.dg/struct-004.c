
/* { dg-do "compile" } */

int main()
{

 struct a;

 struct a { 
   int x; 
   int y;
   struct a z; /* { dg-error "incomplete type" "" } */
 } t;

 return 0;

}

