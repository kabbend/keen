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

 char string[11];
 int i;
 int r;
 for(i=0;i<10;i++) string[i] = 'a' + i;
 string[10] = '\0';
 for(i=0;i<10;i++) printf("%c",r = f(string,i));
 printf("\n");
 printf("%s\n",string);
 if (r != 'j') abort();
 if (strlen(string)!=10) abort();
 return 0;

}

