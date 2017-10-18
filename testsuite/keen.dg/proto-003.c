/* { dg-do "compile" } */
/* { dg-options "-w" } */


int f();

int f(int x);

int f(int x) { return 0; }

int g(int, int);	// { dg-error "" }

int g(int x);	// { dg-error "conflicting types" }

int g(int x) { return 0; }
