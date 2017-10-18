
#ifndef _IR_MACRODEF_H_
#define _IR_MACRODEF_H_

#undef  DEF_CLASS
#define DEF_CLASS( a )
#undef  DEF_SUBCLASS
#define DEF_SUBCLASS( a, b )
#undef  DEF_NODE0
#undef  DEF_NODE1
#undef  DEF_NODE2
#undef  DEF_NODE3
#undef  DEF_NODE4
#undef  DEF_NODE5
#undef  DEF_NODE6
#undef  DEF_NODE7
#define DEF_NODE0( t, c, s, arg ) __replace_me__define n##t()           IR(n_##t)
#define DEF_NODE1( t, c, s, arg ) __replace_me__define n##t(x)          IR(n_##t,x)
#define DEF_NODE2( t, c, s, arg ) __replace_me__define n##t(x,y)        IR(n_##t,x,y)
#define DEF_NODE3( t, c, s, arg ) __replace_me__define n##t(x,y,z)      IR(n_##t,x,y,z)
#define DEF_NODE4( t, c, s, arg ) __replace_me__define n##t(x,y,z,a)    IR(n_##t,x,y,z,a)
#define DEF_NODE5( t, c, s, arg ) __replace_me__define n##t(x,y,z,a,b)  IR(n_##t,x,y,z,a,b)
#define DEF_NODE6( t, c, s, arg ) __replace_me__define n##t(x,y,z,a,b,d)  IR(n_##t,x,y,z,a,b,d)
#define DEF_NODE7( t, c, s, arg ) __replace_me__define n##t(x,y,z,a,b,d,e)  IR(n_##t,x,y,z,a,b,d,e)
#include "ir.def"

#undef  DEF_NODE0
#undef  DEF_NODE1
#undef  DEF_NODE2
#undef  DEF_NODE3
#undef  DEF_NODE4
#undef  DEF_NODE5
#undef  DEF_NODE6
#undef  DEF_NODE7
#define DEF_NODE0( t, c, s, arg ) __replace_me__define n##t##m(mode)           IRm(n_##t,mode)
#define DEF_NODE1( t, c, s, arg ) __replace_me__define n##t##m(mode,x)         IRm(n_##t,mode,x)
#define DEF_NODE2( t, c, s, arg ) __replace_me__define n##t##m(mode,x,y)       IRm(n_##t,mode,x,y)
#define DEF_NODE3( t, c, s, arg ) __replace_me__define n##t##m(mode,x,y,z)     IRm(n_##t,mode,x,y,z)
#define DEF_NODE4( t, c, s, arg ) __replace_me__define n##t##m(mode,x,y,z,a)   IRm(n_##t,mode,x,y,z,a)
#define DEF_NODE5( t, c, s, arg ) __replace_me__define n##t##m(mode,x,y,z,a,b) IRm(n_##t,mode,x,y,z,a,b)
#define DEF_NODE6( t, c, s, arg ) __replace_me__define n##t##m(mode,x,y,z,a,b,d) IRm(n_##t,mode,x,y,z,a,b,d)
#define DEF_NODE7( t, c, s, arg ) __replace_me__define n##t##m(mode,x,y,z,a,b,d,e) IRm(n_##t,mode,x,y,z,a,b,d,e)
#include "ir.def"

#endif
