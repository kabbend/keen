
/* { dg-do "run" } */
/* { dg-options "-w" } */

int
main()
{
 int x[2] = { 5, 6 };
 int y[2][2][3] = { { {0,0,0}, {1,1,1} }, {{2,2,2},{3,3,3} } };
 int ret = y[1][0][2];
 printf("%d\n",ret);
 if (ret != 2) abort();
 return 0;
}

