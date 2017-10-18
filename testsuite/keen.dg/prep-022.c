
/* { dg-do "run" } */
/* { dg-options "-w" } */


#define GET_IRCHILD(x)		x


int
main()
{
GET_IRCHILD(return 0);
return 1;
}

