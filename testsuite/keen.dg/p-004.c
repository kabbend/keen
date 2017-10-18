/* recursion, 
 * pointer and adress manipulation
 */

/* { dg-do "run" } */
/* { dg-options "-w" } */

int f(int a,int *y)
{
  int x = a;
  if (a==0) return *y;
  return f(a-1,&x);
}

int main()
{
  if (f (100, 0) != 1) abort ();
  exit (0);
}
