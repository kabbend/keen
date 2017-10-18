
/* { dg-do "run" } */
/* { dg-options "-w" } */

struct str { int z; int t; char u; };

struct a { int x[10]; int y; struct str sarray[2][2]; char string[12]; };

int
main ()
{

 struct a a_struct;
 struct a b_struct;

 int i,j;
 for(i=0;i<10;i++) a_struct.x[i] = i + 10;
 a_struct.y = -1;
 strcpy( a_struct.string, "0123456789a" );
 for(i=0;i<2;i++) for(j=0;j<2;j++) 
  {
   a_struct.sarray[i][j].z = 5;
   a_struct.sarray[i][j].t = 6;
   a_struct.sarray[i][j].u = '7';
  }

 // struct copy
 b_struct = a_struct;

 printf("b_struct.x[3]=%d, .x[9]=%d\n", b_struct.x[3], b_struct.x[9] );
 if (b_struct.x[3] != 13 || b_struct.x[9] != 19) abort();

 printf("b_struct.y = %d\n", b_struct.y );
 if (b_struct.y != -1) abort();

 printf("b_struct.string[]=%s\n",b_struct.string );
 if (strcmp("0123456789a",b_struct.string) != 0) abort();

 printf("sarray[1][1].z=%d,[0][0].t=%d,[0][1].u='%c'\n",
     b_struct.sarray[1][1].z,
     b_struct.sarray[0][0].t,
     b_struct.sarray[0][1].u);

 if (b_struct.sarray[1][1].z != 5 || b_struct.sarray[0][0].t != 6 || b_struct.sarray[0][1].u != '7') abort();

 return 0;
}

