/* { dg-do "run" } */
/* { dg-options "-w" } */

char
f( char *p, int index )
{
 return p[index];
}

int
main()
{

 char strings[3][11] = { "abcdefghij", "bcdefghijk", "cdefghijkl" };
 int i;
 int r;
 for(i=0;i<10;i++) printf("%c",r = f(strings[0],i));
 printf("\n");
 for(i=0;i<10;i++) printf("%c",r = f(strings[1],i));
 printf("\n");
 for(i=0;i<10;i++) printf("%c",r = f(strings[2],i));
 printf("\n");
 printf("%s\n",strings[0]);
 if (r != 'l') abort();
 if (strlen(strings[0])!=10) abort();

 return 0;

}

