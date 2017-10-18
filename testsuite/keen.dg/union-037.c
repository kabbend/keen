// { dg-do "run" }
// { dg-options "-w" }

struct { int x; int y; union { struct { int Z; } s; int a; int b; } z; int t; } S1 = { 1 , 2 , { 3 } , 4 };
struct { int x; int y; union { struct { int Z; } s; int a; int b; } z; int t; } S2 = { 1 , 2 , 3 , 4 };

int
main()
{
  printf("S1.z.s.Z=%d\n",S1.z.s.Z);
  printf("S2.z.s.Z=%d\n",S2.z.s.Z);
  if (S1.z.s.Z != 3) abort();
  if (S1.z.s.Z != 3) abort();
  printf("S1.t=%d\n",S1.t);
  printf("S2.y=%d\n",S2.y);
  if (S1.t != 4) abort();
  if (S1.y != 2) abort();

}

