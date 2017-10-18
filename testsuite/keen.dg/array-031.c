/* { dg-do "compile" } */

// forward declarations
int printf(char *, ...);
void abort();

int
main(int argc, char **argv)
{
  int x;
  x = sizeof (int []);	// { dg-error "incomplete" }
}


