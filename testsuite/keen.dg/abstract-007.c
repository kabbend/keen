/* { dg-do run } */
/* { dg-options "-w" } */


int f(int *, int);   

int 
f (int x[], int y)
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


