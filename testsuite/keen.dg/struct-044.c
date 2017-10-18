/* { dg-do "compile" } */
/* { dg-options "-w" } */


struct { int x; int z; } A;
struct { int x; int y; } B;

int
main()
{

 A = B; // { dg-error "invalid" } as struct have not same member names 

}
