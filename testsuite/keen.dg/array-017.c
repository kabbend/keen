
/* { dg-do "run" } */
/* { dg-options "-w" } */

int
main()
{

 char string[6] = { 'h', 'e', 'l', 'l', 'o', 0 };
 printf("%s\n",string);
 if ( string[5] != 0 ) abort();
 return 0;

}

