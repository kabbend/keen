/* { dg-do run } */
/* { dg-options "-w" } */

#define A(...)	__VA_ARGS__

int
main()
{
 A(return 0);
 return 1;
}
	





