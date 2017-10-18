
/* { dg-do "run" } */
/* { dg-options "-w" } */

int main()
{

 int *array[2][3];
 array[1][0] = malloc (4); // waiting for sizeof(int) !
 *array[1][0] = 1;
 int x = array[1][0][0];
 printf("x=%d\n",x);
 if (x != 1) abort();
 return 0;

}

