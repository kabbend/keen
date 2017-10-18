
/* { dg-do "compile" } */


#define GET_IRCHILD(x)		GET_IRCHILD(x);


int
main()
{
GET_IRCHILD( 0 ) 	// { dg-warning "implicit declaration" }
return 1;
}

