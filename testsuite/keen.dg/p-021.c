/* { dg-do compile } */

// forward declarations
int printf(char *, ...);
void abort();

unsigned signed int x;  // { dg-error "both signed and unsigned" }

int
main(int argc, char **argv)
{
 int ARRAY[3][2];
 int x[] = (unsigned signed int) ARRAY;
 return 0;
}


