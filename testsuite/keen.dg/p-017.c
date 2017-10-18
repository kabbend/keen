
/* { dg-do "run" } */
/* { dg-options "-w" } */

int
main ()
{

 int x = 3;

 {

 int x = 4;
 printf("VALUE INSIDE 1st COMPOUND=%d\n",x);

 if (x!=4) abort();
 else 
  {
    int x = 8;
    printf("VALUE INSIDE 2nd COMPOUND=%d\n",x);
    if (x!=8) abort();
  }

 printf("VALUE INSIDE 1st COMPOUND=%d\n",x);
 if (x!=4) abort();

 }

 printf("VALUE OUTSIDE COMPOUND=%d\n",x);
 if (x!=3) abort();
 return 0;
}

