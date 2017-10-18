/* { dg-do run } */
/* { dg-options "-w" } */


int f(int *, int *[]);   

int 
f (int x[], int **y)
{
 return x[**y];
}

int
main()
{
 int ARRAY[3] = { 4, 8, 16 };
 int *index = malloc(4);
 *index = 2;
 printf ("ARRAY[1]=%d\n", f(ARRAY,&index));
 if (f(ARRAY,&index) != 16) abort(); 
}


