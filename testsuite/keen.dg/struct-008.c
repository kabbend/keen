
/* { dg-do "run" } */
/* { dg-options "-w" } */

int main()
{

 struct { 
   int x; 
   int y;
 } an_anonymous_struct;

 struct { 
   int x; 
   int y;
 } another_anonymous_struct;

 an_anonymous_struct.x = 3;
 an_anonymous_struct.y = 4;
 another_anonymous_struct.y = 5;
 another_anonymous_struct.y = 6;

 int x = an_anonymous_struct.x +  another_anonymous_struct.y;

 printf("sum=%d\n", x);

 if (x!=9) abort();

 return 0;

}

