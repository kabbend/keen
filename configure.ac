#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)

AC_INIT([keen], [0.1], [kabend@free.fr])

AC_CANONICAL_TARGET
case $target in
       i?86-*-linux*)   TARGET_ARCH=x86
			AC_DEFINE([TARGET_ARCH_X86],[x86],[determine target assembly instruction set])
			AC_DEFINE([TARGET_LINK_LDLINUX],[ld-linux],[determine target linking flavor])
			AM_CONDITIONAL([X86],[test "$TARGET_ARCH" = x86]) ;;
       i?86-*-cygwin*)  TARGET_ARCH=x86
			AC_DEFINE([TARGET_ARCH_X86],[x86],[determine target assembly instruction set])
			AC_DEFINE([TARGET_LINK_CYGWIN],[cygwin],[determine target linking flavor])
			AM_CONDITIONAL([X86],[test "$TARGET_ARCH" = x86]) ;;
       #*)          	AC_MSG_ERROR([sorry. Only linux/cygwin on x86 is supported today]) ;;
esac
AC_SUBST([TARGET_ARCH])

AM_INIT_AUTOMAKE([-Wall -Werror foreign])

AC_CONFIG_SRCDIR([ir.c])
AC_CONFIG_HEADER([config.h])

# Checks for programs.
AC_PROG_YACC
AC_PROG_CC
AC_PROG_LEX

# Checks for libraries.

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([fcntl.h stdlib.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_RESTRICT
AC_TYPE_SIZE_T
AC_C_VOLATILE

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([memmove memset strchr strrchr])

AC_CONFIG_FILES([kburg/Makefile])
AC_CONFIG_FILES([x86/Makefile])
AC_CONFIG_FILES([Makefile])

AC_OUTPUT
