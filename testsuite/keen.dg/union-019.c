// { dg-do compile }

void abort();

union { int a, b, c; } U = { 1, 2, 3 }; // { dg-error "too many" }

int
main()
{
 if (U.b != 1) abort();
 return 0;
}

