/* { dg-do "compile" } */
/* { dg-options "-w" } */


int f();

int f(int x);

int f(int x) { return 0; }

int g(int);

int g(int x);

int g(int x) { return 0; }
