// { dg-do "run" }
// { dg-options "-w" }

long int x = 0x10000001 ;	// low word is 0x1

int
main()
{
 short y = *(&x); 	// take lower part of long int
 printf("%d\n",y);
 if (y != 1) abort();
 return 0;
}
