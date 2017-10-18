/* pointer and adress, side effect within function call,
 * side effect on pointer-indirection
 * comma-separated expression
 */

/* { dg-do "run" } */
/* { dg-options "-w" } */

int f(int x) { return x; }
int main()
{
 int a = 1;
 int *p = &a;
 *p = 3;
 int x;
 x = f(a);
 x += 10 * f((a=2,a++,*p));
 x += 100 * f((--a,(*p)++));
 x += 1000 * f(a);
 if (x != 3233) abort();
 return 0;
}


