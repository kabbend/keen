

SUBDIRS = kburg $(TARGET_ARCH) 

BUILT_SOURCES = irmacro.h y.tab.h

bin_PROGRAMS = keen
keen_SOURCES = 	y.tab.c lexer.l \
		error.c  insn.c list.c  reg.c translate.c xmalloc.c \
		bind.c frame.c ir.c symbol.c type.c \
		canon.c grammar.c keen.c prep.c temp.c version.c \
		flow.c regalloc.c link.c \
		pp.y.tab.c hash.c

if X86 
  keen_SOURCES += x86/frame_x86.c x86/gen_x86.c x86/reg_x86.c x86/x86.c 
endif

CFLAGS += -DKEEN_VERSION="\"$(PACKAGE_VERSION).$(shell if [ -f keenbuild ]; then cat keenbuild; else echo 0; fi)\""

AUTOMAKE_OPTIONS = dejagnu
EXTRA_DIST = testsuite/keen.dg/keen.exp testsuite/config/unix.exp 
RUNTESTFLAGS = --tool keen --srcdir ./testsuite

irmacro.h: irmdef.h
	$(CPP) $(CPPFLAGS) irmdef.h > irmacro.h
	sed -e's/__replace_me__/#/' -i irmacro.h

y.tab.h: grammar.y
	$(YACC) -t --defines=y.tab.h grammar.y

pp.y.tab.c: ppgrammar.y
	$(YACC) -o pp.y.tab.c -p pp_yy ppgrammar.y

clean-local:
	rm -f lexer.c y.tab.* pp.y.tab.* irmacro.h

build:
	if [ -f keenbuild ]; then echo `expr \`cat keenbuild \` + 1` > keenbuild; fi
	$(MAKE) clean all

