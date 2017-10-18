
/* { dg-do "run" } */
/* { dg-options "-w" } */

int 
main()
{

 char string[] = "hello world";
 printf("%s",string);
 char *p = string + 2;
 if (strlen(p) != 9) abort();
 return 0;

}


