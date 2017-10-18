// { dg-do "run" }
// { dg-options "-w" }

int A[] = 
{
 3, 
 4, 
 5, // note the trailing comma
};

int
main()
{
 if (A[2] != 5) abort();
 return 0;
}

