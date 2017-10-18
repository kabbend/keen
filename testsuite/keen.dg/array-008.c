
/* { dg-do "run" } */
/* { dg-options "-w" } */

int f( int x ) { return x+1; }

int main()
{

 int array[3][3];
 int i,j;
 // zero table
 for (i=0;i<3;i++) for(j=0;j<3;j++) array[i][j] = f(i)*f(j);
 // set one cell
 array[f(-1)+1][f(1)*f(1)-2] = 1;
 // check that cell
 for (i=0;i<3;i++) for(j=0;j<3;j++) printf("[%d][%d]=%d\n",i,j,array[i][j]);
 int x = array[1][2];
 printf("x=%d\n",x);
 if (x != 1) abort();
 return 0;

}

