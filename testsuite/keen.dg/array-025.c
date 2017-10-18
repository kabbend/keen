
/* { dg-do "run" } */
/* { dg-options "-w" } */

int
main()
{

 // test array[] init with string, and no length
 char string[] =  "hello world";
 int len = strlen(string);
 printf("%s\n",string);
 if (len != 11) abort();
 return 0;

}

