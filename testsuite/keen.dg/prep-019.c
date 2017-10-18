/* { dg-do run } */
/* { dg-options "-w" } */


int
main()
{
  char s[] = "ABC"  "abc" /* include a comment between strings */ 
	"xyz" // include another comment 
    ;
  printf("%s\n",s);
  if (strlen(s) != 9) abort();
  return 0;
}
	





