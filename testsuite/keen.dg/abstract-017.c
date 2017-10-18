/* { dg-do compile } */

// forward declarations
int printf(char *, ...);
void abort();

int
main(int argc, char **argv)
{
 int ARRAY[3];
 int x[] = (signed int) ARRAY; // { dg-error "cannot cast" }
 return 0;
}


