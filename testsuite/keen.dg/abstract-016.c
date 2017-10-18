/* { dg-do compile } */

// forward declarations
int printf(char *, ...);
void abort();

int
main(int argc, char **argv)
{
 int x;
 (int) x = 2; // { dg-error "not an lvalue" } 
 return 0;
}


