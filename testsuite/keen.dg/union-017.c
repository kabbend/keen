/* { dg-do "compile" } */
/* { dg-options "-w" } */


union { long x; int y; } A;
union { int x; int y; } B;

int
main()
{

 A = B; // { dg-error "invalid" } as struct are not the same

}
