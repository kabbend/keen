/* for loop, continue */
/* { dg-do "run" } */
/* { dg-options "-w" } */

int main()
{
  int i,j;
  for(i=0,j=2;i<10&&j!=7;i++,j++) 
    {
      if (j == 4) { continue; }
    }
  if (i!=5) abort();
  return 0;
}

