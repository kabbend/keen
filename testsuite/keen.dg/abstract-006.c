/* { dg-do compile } */
/* { dg-options "-w" } */


void f(int [], int);    /* { dg-error "" } */

int 
f (int x[], int y)	/* { dg-error "conflicting types" } */
{
 return x[y];
}

int
main()
{
 int ARRAY[3] = { 4, 8, 16 };
 printf ("ARRAY[1]=%d\n", f(ARRAY,1));
 if (f(ARRAY,1) != 8) abort(); 
}


