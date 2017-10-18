
/* { dg-do "run" } */
/* { dg-options "-w" } */

int x = 3, y;
int z[2][3] = { {1,2,3}, {6,7,8} };
char t[3] = { 'a', 'b', 'c' };
char string1[] = "foobarbaz";

char string3[20];
int array_of_int[10][3];

int
main()
{

 // check that un-initialized global array is zeroed
 int i,j,total=0;
 for(i=0;i<10;i++) for(j=0;j<3;j++) total += array_of_int[i][j];
 printf("array sum = %d\n",total);
 if (total) abort();

 // affectation of a global un-initialized string
 strcpy(string3,"foobarbaz");

 // local string
 char string2[] = "foobarbaz";

 printf("%s=%s=%s\n",string1,string2,string3);
 if (strcmp(string1,string2)!=0 || strcmp(string1,string3)!=0) abort();

 printf("x=%d,y=%d\n",x,y);
 if (x != 3 || y != 0) abort();
 printf("t[2]=%c,z[1][1]=%d\n",t[2],z[1][1]);
 if (t[2] != 'c' || z[1][1] != 7) abort();
 return 0;

}

