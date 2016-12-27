AC_PREREQ(2.61)
AC_INIT([ptfs], [0.0.4.1], [])
AM_INIT_AUTOMAKE([1.9])

AC_PROG_CC

AC_PROG_OCAML
if test "$OCAMLC" = "no"; then
	AC_MSG_ERROR(["Can't find ocaml bytecode compiler"])	
fi

if test "OCAMLOPT" = "no"; then
	AC_MSG_ERROR(["Can't find ocaml native-code compiler"])
fi

if test "OCAMLMKLIB" = "no"; then
	AC_MSG_ERROR(["Can't find ocamlmklib"])
fi


PKG_CHECK_MODULES(DEPS, glib-2.0 fuse) 
AC_SUBST(DEPS_CFLAGS)
AC_SUBST(DEPS_LIBS)

AC_CONFIG_FILES([
	Makefile
	src/Makefile
	src/aurochs/Makefile
])

AC_OUTPUT
