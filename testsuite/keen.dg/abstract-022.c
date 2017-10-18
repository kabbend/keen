/* { dg-do compile } */
/* { dg-options "-w" } */

// accept function with (void)

void f();

void f(void);

void f(void)
{
 return;
}


