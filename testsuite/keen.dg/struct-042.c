/* { dg-do "run" } */
/* { dg-options "-w" } */


struct { int x; int y; } A;
struct { int x; int y; } B;

int
main()
{

 A = B; // this should raise no error as struct have same members and names. Surprisingly gcc does not allow this ?
 return 0;

}
