// { dg-do "run" }
// { dg-options "-w" }

struct { int x; int y; union { struct { int Z; } s; int a; int b; } z; int t; } S1 = { 1 , 2 , { 3 } , 4 };
struct { int x; int y; union { struct { int Z; } s; int a; int b; } z; int t; } S2 = { 1 , 2 , 3 , 4 };
struct { int x; int y; union { struct { int Z; } s; int a; int b; } z; int t; } S3 = { 1 , 2 , { {3} } , 4 }; 	// allowed because scalar init 

int 
main()
{
 printf("%d\n",S3.z.s.Z);
 if (S3.z.s.Z != 3) abort();
}


