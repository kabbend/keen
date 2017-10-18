/* { dg-do compile } */
/* { dg-options "-w" } */


union { int x; int y; } S = { 3, 4, 5 }; // { dg-error "too many elements" }



