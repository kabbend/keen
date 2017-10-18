// { dg-do run }

void abort();

union { int a, b, c; } U = { 1 };

int
main()
{
 if (U.b != 1) abort();
 return 0;
}

