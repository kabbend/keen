
/* { dg-do "run" } */
/* { dg-options "-w" } */

int f( int x ) { return x+1; }
int g( int x ) { return x; }
int main()
{

 int array[3][3];
 int i,j;
 // zero table
 for (i=0;i<3;i++) for(j=0;j<3;j++) array[i][j] = f(i)*f(j); 
 for (i=0;i<3;i++) for(j=0;j<3;j++) printf("[%d][%d]=%d\n",i,j,array[i][j]);
 // test assign-plus affectation, on some strange affectation
 for (i=0;i<3;i++) for(j=0;j<3;j++) array[g(i)][g(j)] += array[g(i)][g(j)] + array[0][j]; 
 for (i=0;i<3;i++) for(j=0;j<3;j++) printf("[%d][%d]=%d\n",i,j,array[i][j]);
 // check last cell
 int x = array[2][2];
 printf("x=%d\n",x);
 if (x != 27) abort();
 return 0;

}

