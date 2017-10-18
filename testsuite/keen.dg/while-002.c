
/* while loop, side effects, continue */

/* { dg-do "run" } */
/* { dg-options "-w" } */

int
main ()
{
  int i = 0;
  int sum = 0;
  while(i<10)
    {
      i++;
      if (i == 2 || i == 4 || i == 6 || i == 8) 
        {
         sum+=i;
         continue;
         abort(); // should not get here !
        }
    }
  if (sum != 20) abort();
  return 0;
}

