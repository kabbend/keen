/* { dg-do compile } */
/* { dg-options "-w" } */

#define A(y, ... , x) 	__VA_ARGS__ 	// { dg-error "" }

	





