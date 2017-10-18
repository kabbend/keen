/* { dg-do "run" } */
/* { dg-options "-w" } */


union { int x; int y; } A;
union { int x; int y; } B;

int
main()
{

 A = B; // this should raise no error as union have same members and names. Surprisingly gcc does not allow this ?
 return 0;

}
