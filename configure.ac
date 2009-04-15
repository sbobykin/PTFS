AC_PREREQ(2.61)
AC_INIT([ptfs], [0.0.2.1], [http://ptfs.berlios.de])
AM_INIT_AUTOMAKE([1.9])

AC_PROG_CC

AC_PROG_OCAML
if test "$OCAMLC" = "no"; then
	AC_MSG_ERROR(["Can't find ocaml bytecode compiler"])	
fi

if test "OCAMLOPT" = "no"; then
	AC_MSG_ERROR(["Can't find ocaml native-code compiler"])
fi

AC_CONFIG_FILES([
	Makefile
	src/Makefile
])

AC_OUTPUT
