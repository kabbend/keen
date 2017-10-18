

/* { dg-do "run" } */
/* { dg-options "-w" } */

int f()
{
 a_label: /* same label */
 return 0;
}

int main()
{
 goto a_label;
 abort();
 a_label: 
 return f();
}





