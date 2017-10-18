/* { dg-do compile } */

// forward declarations
int printf(char *, ...);
void abort();

struct a { int z; } A;


int
main(int argc, char **argv)
{
 (int) A; // { dg-error "cannot cast" }
}


