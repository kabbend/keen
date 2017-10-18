
/* { dg-do "run" } */
/* { dg-options "-w" } */

int
main ()
{

 enum ENUM { VALUE = 2 };

 {
 enum ENUM { VALUE = 1, TWO };

 printf("VALUE INSIDE COMPOUND=%d\n",VALUE);
 if (VALUE!=1) abort();

 }

 printf("VALUE OUTSIDE COMPOUND=%d\n",VALUE);
 if (VALUE!=2) abort();
 return 0;
}

