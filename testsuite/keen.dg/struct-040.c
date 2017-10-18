
/* { dg-do "run" } */
/* { dg-options "-w" } */


#define m8	8
#define m16	16	
#define m32	32	
#define NOREG	-1
#define REGCLASS_GENERAL	0	/* general register */
#define REGCLASS_FLOAT		0	/* floating point */
#define REGCLASS_SPECIAL	0	/* special regs */
#define REGCLASS_RV		0	/* return value */
#define REGCLASS_FP		0	/* frame pointer */
#define REGCLASS_SP		0	/* stack pointer */
#define REGCLASS_ZERO		0	/* zero reg */
#define REGCLASS_ARG		0	/* pass arg to function */
#define REGCLASS_CALLEE		0	/* callee save */
#define REGCLASS_CALLER		0	/* caller save */

typedef struct xreg
{
  int id;
  char *name;
  int class;
  int mode;
  int subreg_of;
  int *subregs;
  void *temps;
} reg;

reg hardregset[20] = {
  {0,  "eax", REGCLASS_GENERAL, m32, NOREG, 0, 0} ,
  {1,  "ecx", REGCLASS_GENERAL, m32, NOREG, 0, 0} ,
  {2,  "edx", REGCLASS_GENERAL, m32, NOREG, 0, 0} ,
  {3,  "ebx", REGCLASS_GENERAL, m32, NOREG, 0, 0} ,
  {4,  "edi", REGCLASS_GENERAL, m32, NOREG, 0, 0} ,
  {5,  "esi", REGCLASS_GENERAL, m32, NOREG, 0, 0} ,
  {6,  "ebp", REGCLASS_FP,      m32, NOREG, 0, 0} ,
  {7,  "ax",  REGCLASS_GENERAL, m16, 0, 0, 0} ,
  {8,  "ah",  REGCLASS_GENERAL, m8,  7, 0, 0} ,
  {9,  "al",  REGCLASS_GENERAL, m8,  7, 0, 0} ,
  {10, "bx",  REGCLASS_GENERAL, m16, 3, 0, 0} ,
  {11, "bh",  REGCLASS_GENERAL, m8,  10, 0, 0} ,
  {12, "bl",  REGCLASS_GENERAL, m8,  10, 0, 0} ,
  {13, "cx",  REGCLASS_GENERAL, m16, 1, 0, 0} ,
  {14, "ch",  REGCLASS_GENERAL, m8,  13, 0, 0} ,
  {15, "cl",  REGCLASS_GENERAL, m8,  13, 0, 0} ,
  {16, "dx",  REGCLASS_GENERAL, m16, 2, 0, 0} ,
  {17, "dh",  REGCLASS_GENERAL, m8,  16, 0, 0} ,
  {18, "dl",  REGCLASS_GENERAL, m8,  16, 0, 0} ,
  {19, "esp", REGCLASS_FP,      m32, NOREG, 0, 0}
};

int
main()
{
  printf("%s\n",hardregset[19].name);
  if (hardregset[18].subreg_of != 16) abort();
  return 0;
}

