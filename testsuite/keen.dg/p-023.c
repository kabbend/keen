/* { dg-do "run" } */
/* { dg-options "-w" } */

// forward declarations
int printf(char *, ...);
void abort();

int main()
{

  char string[] = "this is a test string";
  char *c = string;
  while(*c) { printf("%c",*c); c++; }
  printf("\n");

  int tab[] = { 1, 2, 3, 4, 0 };
  int *v = tab;
  while(*v) { printf("%d",*v); v = v+1; }
  printf("\n");
  // we test here arithmetic on pointers 
  if (*(v-1) != 4) abort();

  v = tab;
  while(*v) { printf("%d",*v); v++; }
  printf("\n");
  // we test here arithmetic on pointers with postfix/prefix operators
  if (*(--v) != 4) abort();

  return 0;
  
}


