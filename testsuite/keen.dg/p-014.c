
/* { dg-do "run" } */
/* { dg-options "-w" } */

int x = 3, y;
int z[2][3] = { {1,2,3}, {6,7,8} };
char t[3] = { 'a', 'b', 'c' };

int
main()
{

 printf("x=%d,y=%d\n",x,y);
 if (x != 3 || y != 0) abort();
 printf("t[2]=%c,z[1][1]=%d\n",t[2],z[1][1]);
 if (t[2] != 'c' || z[1][1] != 7) abort();
 return 0;

}

