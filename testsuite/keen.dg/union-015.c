
/* { dg-do "run" } */
/* { dg-options "-w"  }  */

int
main()
{

 union a { int x; union b *p; } a_union;
 union b { int x; union c *p; } b_union;
 union c { int x; union d *p; } c_union;
 union d { int x; union a *p; } d_union;

 d_union.x = 80;

 a_union.p = &b_union;
 b_union.p = &c_union;
 c_union.p = &d_union;

 int x = a_union.p->p->p->x;

 printf("x=%d\n",x);

 if (x != 80) abort();

 return 0;

}

