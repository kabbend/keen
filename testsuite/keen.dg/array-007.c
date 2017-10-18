
/* { dg-do "run" } */
/* { dg-options "-w" } */

int f( int x ) { return x+1; }

int main()
{

 int array[3][3];
 int i,j;
 // zero table
 for (i=0;i<3;i++) for(j=0;j<3;j++) array[i][j] = 0;
 // set one cell
 array[f(0)+1][f(1)-1] = 3;
 // check that cell
 int x = array[2][1];
 printf("x=%d\n",x);
 if (x != 3) abort();
 for (i=0;i<3;i++) for(j=0;j<3;j++) printf("[%d][%d]=%d\n",i,j,array[i][j]);
 return 0;

}

