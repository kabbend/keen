
/* { dg-do "run" } */
/* { dg-options "-w" } */

int 
main()
{

 char string[13];
 strcpy(string,"hello world\n");
 printf("%s",string);
 char *p = string + 2;
 if (strlen(p) != 10) abort();
 return 0;

}


