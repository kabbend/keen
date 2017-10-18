/* for loop, break */
/* { dg-do "run" } */
/* { dg-options "-w" } */

int main()
{
  int i=0;
  for(;;i++) 
    {
      if (i==10) break;
    }
  if (i!=10) abort();
  return 0;
}

