/* { dg-do run } */
/* { dg-options "-w" } */


int
main()
{
  char s[] = "ABC"  "abc" "xyz";
  printf("%s\n",s);
  if (strlen(s) != 9) abort();
  return 0;
}
	





