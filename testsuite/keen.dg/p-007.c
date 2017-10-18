/* pointer, side effect in pointer/array indexation
 * char (1 byte-long value) management
 * array affectation
 */

/* { dg-do "run" } */
/* { dg-options "-w" } */

int 
main()
{
 char *p = "abcde";

 p[2] = 'x'; // this leads to 'abxde'
             // this also requires that p memory is writable (no .rodata section)

 if (strlen(p) != 5) abort();
 return 0;
}


