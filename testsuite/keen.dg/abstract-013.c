/* { dg-do compile } */

// forward declarations
int printf(char *, ...);
void abort();

int
main(int argc, char **argv)
{
 int x = (int *) 3; // { dg-warning "" } 
 return 0;
}


