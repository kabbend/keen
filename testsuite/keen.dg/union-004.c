
/* { dg-do "compile" } */

int main()
{

 union a;

 struct a { 
   int x; 
   int y;
   union a z; /* { dg-error "incomplete type" "" } */
 } t;

 return 0;

}

