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

 p[2] = 'f'; // this leads to 'abfde' and total 13
             // this also requires that p memory is writable (no .rodata section)

 int sum = 0;
 int i = 0;
 while(i++,i-- /* this serves nothing, just to try */,p[i]) 
  { 
   int z = p[++i - 1] - 'a';
   sum += z; 
  }
 if (sum != 13) abort();
 return 0;
}


