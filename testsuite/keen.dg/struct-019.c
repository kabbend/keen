
/* { dg-do "run" } */
/* { dg-options "-w"  }  */

int
main()
{

 struct a { int x; struct b *p; } a_struct;
 struct b { int x; struct c *p; } b_struct;
 struct c { int x; struct d *p; } c_struct;
 struct d { int x; struct a *p; } d_struct;

 a_struct.x = 77;
 b_struct.x = 78;
 c_struct.x = 79;
 d_struct.x = 80;

 a_struct.p = &b_struct;
 b_struct.p = &c_struct;
 c_struct.p = &d_struct;
 d_struct.p = &a_struct;

 int x = a_struct.p->p->p->p->x;

 printf("x=%d\n",x);

 if (x != 77) abort();

 return 0;

}

