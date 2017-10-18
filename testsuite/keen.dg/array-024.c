
/* { dg-do "run" } */
/* { dg-options "-w" } */

int
main()
{

 char strings[3][6] =  { "abc" , {'d', 'e', '\0'}, "fghi" } ;
 printf("%s\n",strings[0]);
 printf("%s\n",strings[1]);
 printf("%s\n",strings[2]);
 int len = strlen(strings[2]);
 if (len != 4) abort();
 return 0;

}

