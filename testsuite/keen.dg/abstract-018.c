/* { dg-do compile } */

// forward declarations
int printf(char *, ...);
void abort();


int
main(int argc, char **argv)
{
 int x = (unsigned signed int) 3; // { dg-error "both signed and unsigned" }
 return 0;
}


