/* pointer, side effect in pointer/array indexation
 * char (1 byte-long value) management
 */

/* { dg-do "run" } */
/* { dg-options "-w" } */

int 
main()
{
 char *p = "abcde";
 int sum = 0;
 int i = 0;
 while(i++,i-- /* this serves nothing, just to try */,p[i]) 
  { 
   int z = p[++i - 1] - 'a';
   sum += z; 
  }
 if (sum != 10) abort();
 return 0;
}


