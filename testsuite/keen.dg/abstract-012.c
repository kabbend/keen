/* { dg-do compile } */

// forward declarations
int printf(char *, ...);
void abort();

int
main(int argc, char **argv)
{
 int x = (int) (int *) 3; // no warning
 return 0;
}


