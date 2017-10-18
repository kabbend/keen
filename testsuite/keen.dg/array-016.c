
/* { dg-do "run" } */
/* { dg-options "-w" } */

int f(int x) { return 2*x; }

int
main()
{
 int y[2][2][3] = { { {f(0),f(0),f(0)}, {f(1),f(1),f(1)} }, {{f(2),f(2),f(2)},{f(3),f(3),3} } };
 int ret = y[1][0][2];
 printf("%d\n",ret);
 if (ret != 4) abort();
 return 0;
}

