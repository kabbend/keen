/* { dg-do "run" } */
/* { dg-options "-w" } */

struct foos { int l; }; 
int foo;
struct foos *getfoo();
int main ()
{
  struct foos *f = getfoo();
  f->l = 1;
  foo = 2;
  if (f->l == 1)
    abort();
  exit(0);
}
struct foos *getfoo() 
{ return &foo; }
