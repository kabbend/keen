/* { dg-do "run" } */
/* { dg-options "-w" } */

int
f( int *p, int index )
{
 return p[index];
}

int
main()
{

 int a[10];
 int i;
 int r;
 for(i=0;i<10;i++) a[i] = i;
 for(i=0;i<10;i++) printf("%d \n",r = f(a,i));
 if (r != 9) abort();
 return 0;

}

