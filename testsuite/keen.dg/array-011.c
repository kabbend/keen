
/* { dg-do "run" } */
/* { dg-options "-w" } */

int
main()
{
 int y = 0;
 int x[2][4] = { {20,21,22,23} , { 100, 101, 102, 103} };
 for(;y<4;y++) printf("x[0][%d]=%d\n",y,x[0][y]); 
 for(y=0;y<4;y++) printf("x[1][%d]=%d\n",y,x[1][y]); 
 if ( x[1][1] + x[0][3] != 124 ) abort();
}

