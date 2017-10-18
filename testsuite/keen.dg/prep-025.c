/* { dg-do "run" } */
/* { dg-options "-w" } */


#define MACRO(a,b,c)	a   b##c	;


int
main()
{
MACRO ( return ,  0x ,  0 )
return 1;
}

