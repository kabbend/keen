/* { dg-do "compile" } */
/* { dg-options "-w" } */


union { int x; int z; } A;
union { int x; int y; } B;

int
main()
{

 A = B; // { dg-error "invalid" } as struct have not same member names 

}
