
/* { dg-do "compile" } */

struct a { int x; };

struct a f()
{
 struct a ret;
 ret.x = 1;
 return ret;
}

int
main ()
{

 struct a a_struct;
 a_struct += f(); /* { dg-error "invalid assignment" "" } */

}

