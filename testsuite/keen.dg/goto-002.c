
/* { dg-do "compile" } */

int f()
{
 a_label: /* same label */
 return 0;
}

int main()
{
 goto a_label;
 goto a_label;
 a_label: 
 a_label:  /* { dg-error "already defined" "" } */
 return 0;
}





