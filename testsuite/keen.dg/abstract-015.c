/* { dg-do compile } */

// forward declarations
int printf(char *, ...);
void abort();

int
main(int argc, char **argv)
{
 int ARRAY[2];
 int x = (int []) ARRAY; // { dg-error "cannot cast" } 
 return 0;
}


