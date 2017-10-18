/* { dg-do "compile" } */
/* { dg-options "-w" } */


struct { long x; int y; } A;
struct { int x; int y; } B;

int
main()
{

 A = B; // { dg-error "invalid" } as struct are not the same

}
