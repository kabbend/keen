
/* { dg-do "run" } */
/* { dg-options "-w" } */

int
main()
{

 char *string[3] = { "test1", "test2", "test3" };
 printf("%s\n",string[0]);
 printf("%s\n",string[1]);
 printf("%s\n",string[2]);
 if ( string[1][4] != '2' ) abort();
 return 0;

}

