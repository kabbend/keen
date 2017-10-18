
/* { dg-do "run" } */
/* { dg-options "-w" } */

int main()
{

 union { 
   int x; 
   int y;
 } an_anonymous_union;

 union { 
   int x; 
   int y;
 } another_anonymous_union;

 an_anonymous_union.x = 3;
 another_anonymous_union.y = 5;

 int x = an_anonymous_union.y +  another_anonymous_union.x;

 printf("sum=%d\n", x);

 if (x!=8) abort();

 return 0;

}

