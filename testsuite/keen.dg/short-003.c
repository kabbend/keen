// { dg-do "run" }
// { dg-options "-w" }

int
main()
{
 long int x = 0x10000001 ;	// low word is 0x1
 short y = *(&x); 	// take lower part of long int
 printf("%d\n",y);
 if (y != 1) abort();
 return 0;
}
