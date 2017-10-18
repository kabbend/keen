
/* { dg-do "run" } */
/* { dg-options "-w" } */

int
main()
{

 char string[20] =  "hello world";
 int len = strlen(string);
 printf("%s\n",string);
 if (len != 11) abort();
 return 0;

}

