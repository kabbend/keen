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
 int y;
 int z;
 int t;
 x = f(a);
 y = f((a=2,a++,*p));
 z = f((--a,(*p)++));
 t = f(a);
 if (x + 10 * y + 100 * z + 1000 * t != 3233) abort();
 return 0;
}


