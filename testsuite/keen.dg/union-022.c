
/* { dg-do "run" } */
/* { dg-options "-w" } */

union str { int z; int t; char u; };

union a { int x[10]; int y; union str sarray[2][2]; char string[12]; };

int
main ()
{

 union a a_union;
 union a b_union;

 int i,j;
 for(i=0;i<2;i++) for(j=0;j<2;j++) 
  {
   a_union.sarray[i][j].z = 5;
   a_union.sarray[i][j].t = 6;
   a_union.sarray[i][j].u = '7';
  }

 // union copy
 b_union = a_union;

 printf("sarray[1][1].z=%d,[0][0].t=%d,[0][1].u='%c'\n",
     b_union.sarray[1][1].z,
     b_union.sarray[0][0].t,
     b_union.sarray[0][1].u);

 if (b_union.sarray[1][1].z != 55 || b_union.sarray[0][0].t != 55 || b_union.sarray[0][1].u != '7') abort();

 return 0;
}

