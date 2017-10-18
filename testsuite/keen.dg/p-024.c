// { dg-do "compile" }
// { dg-options "-w" }

/* IR modes */
enum {
 NUL_mode = 0,  /* no or unknown mode */

 SBY_mode = 1,  /* Single Byte mode */
 m8       = 1,  

 TBY_mode = 2,  /* Two Bytes mode */
 m16      = 2,

 FBY_mode = 4,  /* Four Bytes mode */
 m32      = 4,

 EBY_mode = 8,  /* Eight Bytes mode */
 m64      = 8,

 BIT_mode = 10, /* bit mode */

 STR_mode = 11, /* string mode */

 CC_mode  = 12  /* condition code mode */
};


#define NOREG	-1

#define REGCLASS_GENERAL	0x0000000	/* general register */
#define REGCLASS_FLOAT		0x0000001	/* floating point */
#define REGCLASS_SPECIAL	0x0000002	/* special regs */

#define REGCLASS_RV		0xF000000	/* return value */
#define REGCLASS_FP		0xF100000	/* frame pointer */
#define REGCLASS_SP		0xF200000	/* stack pointer */
#define REGCLASS_ZERO		0xF400000	/* zero reg */
#define REGCLASS_ARG		0xF800000	/* pass arg to function */
#define REGCLASS_CALLEE		0xF010000	/* callee save */
#define REGCLASS_CALLER		0xF020000	/* caller save */


typedef struct _List *List;
typedef struct _ListElem *ListElem;

struct _ListElem
{
  void *object;
  struct _ListElem *next;
  struct _ListElem *prev;
};

struct _List
{
  ListElem l;
	/* first element of the list */

  ListElem last;
	/* last element of the list */

  int size;
	/* number of elements in the list */

  ListElem *t;
	/* array of pointers to elements, to ease get_ith() retrieval function */

  int maxt;
	/* maximum number of elements in previous array (0 at startup, is increased when necessary) */

};

#define NULL	0

typedef struct xreg
{
  int id;
  char *name;
  int class;
  int mode;
  int subreg_of;
  int *subregs;
  List temps;
} reg;

reg hardregset[] = {
  {0,  "eax", REGCLASS_GENERAL, m32, NOREG, NULL, NULL} ,
  {1,  "ecx", REGCLASS_GENERAL, m32, NOREG, NULL, NULL} ,
  {2,  "edx", REGCLASS_GENERAL, m32, NOREG, NULL, NULL} ,
  {3,  "ebx", REGCLASS_GENERAL, m32, NOREG, NULL, NULL} ,
  {4,  "edi", REGCLASS_GENERAL, m32, NOREG, NULL, NULL} ,
  {5,  "esi", REGCLASS_GENERAL, m32, NOREG, NULL, NULL} ,
  {6,  "ebp", REGCLASS_FP,      m32, NOREG, NULL, NULL} ,
  {7,  "ax",  REGCLASS_GENERAL, m16, 0, NULL, NULL} ,
  {8,  "ah",  REGCLASS_GENERAL, m8,  7, NULL, NULL} ,
  {9,  "al",  REGCLASS_GENERAL, m8,  7, NULL, NULL} ,
  {10, "bx",  REGCLASS_GENERAL, m16, 3, NULL, NULL} ,
  {11, "bh",  REGCLASS_GENERAL, m8,  10, NULL, NULL} ,
  {12, "bl",  REGCLASS_GENERAL, m8,  10, NULL, NULL} ,
  {13, "cx",  REGCLASS_GENERAL, m16, 1, NULL, NULL} ,
  {14, "ch",  REGCLASS_GENERAL, m8,  13, NULL, NULL} ,
  {15, "cl",  REGCLASS_GENERAL, m8,  13, NULL, NULL} ,
  {16, "dx",  REGCLASS_GENERAL, m16, 2, NULL, NULL} ,
  {17, "dh",  REGCLASS_GENERAL, m8,  16, NULL, NULL} ,
  {18, "dl",  REGCLASS_GENERAL, m8,  16, NULL, NULL} ,
  /* special registers */
  {19, "esp", REGCLASS_FP,      m32, NOREG, NULL, NULL}
};
