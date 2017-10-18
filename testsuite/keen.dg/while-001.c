
/* while loop, nested loops, break, pointer via function calls */

/* { dg-do "run" } */
/* { dg-options "-w" } */

char *
f (int size)
{
  char *s = malloc(size); // no cast. -w removes warning
  if ( s == 0 )
    {
      // probably not a fatal memory error here, but some bug !
      exit(1);
    }
  return s;
}

int
main ()
{
  int i = 1;
  int j = 0;
  while(i<10)
    {
      char *addr = f(i);
      i=i+1;
      if (i==5) 
        {
        j = 0;
        while(j<10)
          {
            if (j++==i) break;
          }
      }
    }
  if (!(i == 5 && j == 6)) abort();
  return 0;
}

