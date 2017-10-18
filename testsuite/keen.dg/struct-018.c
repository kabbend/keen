
/* { dg-do "run" } */
/* { dg-options "-w"  }  */

int
main()
{

 struct a { int x; struct b *p; } a_struct;
 struct b { int x; struct a *p; } b_struct;

 a_struct.x = 100;
 b_struct.x = 200;

 printf("a.x=%d b.x=%d\n",a_struct.x, b_struct.x);

 int save_1 = a_struct.x;
 int save_2 = b_struct.x;

 a_struct.p = &b_struct;
 b_struct.p = &a_struct;

 a_struct.p->x = 100;
 b_struct.p->x = 200;

 int save_3 = a_struct.x;
 int save_4 = b_struct.x;

 printf("a.x=%d b.x=%d\n",a_struct.x, b_struct.x);

 if (save_1 != save_4 || save_2 != save_3) abort();

 return 0;

}

