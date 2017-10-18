
/* { dg-do "run" } */
/* { dg-options "-w" } */

int
foo ()
{
  int x;
  int jump_inside = 5;
  switch (jump_inside)
    {
    case 0:
      return 1;
    case 1:
      x = 2;
      break;
    default:
      x = 3;
      break;
    case 3:
      if (0) { case 5: x = 0; } else { x = 5; } break;       
    }
  return x;
}

int main()
{
  if (foo() != 0) abort ();
  exit (0);
}
