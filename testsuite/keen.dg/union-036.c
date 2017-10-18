/* { dg-do "compile" } */
/* { dg-options "-w" } */


union { int x; int y[2]; } U = { { 2, 3 } } ; // { dg-error "" } 
