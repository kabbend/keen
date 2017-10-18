/* { dg-do "run" } */
/* { dg-options "-w" } */

struct A
{
  struct A *a;
  int b;
};

struct B
{
  struct A *c;
  unsigned int d;
};

struct A p; 
struct B q;

struct B *
foo ()
{
  return &q;
}

void
bar ()
{
  struct B *e = foo ();
  struct A *f = e->c;
  int g = f->b;

  if (++g == 0)
    {
      e->d++;
      e->c = f->a;
    }

  f->b = g;
}

int
main ()
{
  p.a = &p;
  p.b = -1;
  q.c = &p;
  q.d = 0;
  bar ();
  if (p.b != 0 || q.d != 1 || q.c != &p)
    abort ();
  exit (0);
}
