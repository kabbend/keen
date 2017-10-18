
/* { dg-do "run" } */
/* { dg-options "-w" } */

int which_alternative = 3;

char *foo () { return "foo"; }

char *
bar ()
{
  switch (which_alternative)
    {
    case 0:
      return "foo";
    case 1:
      return foo ();
    default:
      return "foo";
    case 2:
      return "foo";
    case 3:
      return "baz";      
    }
}

int main()
{
  char *s = bar () ;
  if (s[0] != 'b')
    abort ();
  exit (0);
}
