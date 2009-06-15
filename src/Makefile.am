# This file contains reuse notes. See the file 'REUSE' for details.

OCAMLC_CCOPT=-ccopt -I../aurochs/include/ -ccopt -g
DEPS_OCAMLC_CCOPT=`for i in $(DEPS_CFLAGS); do echo -n  " -ccopt $$i "; done;`

bin_PROGRAMS = ptfs
ptfs_SOURCES = ptfs.c ptfs.h parse.c compilewrap.c
COMPILE=$(CPP)

ptfs: ptfs.o compilewrap.o parse.o aurochs_lib.o
	gcc -g -o ptfs ptfs.o compilewrap.o aurochs_lib.o parse.o \
	    -L ../aurochs/_build/ -L $(OCAMLLIB) \
	    -laurochs -lunix -lasmrun -ldl -lm $(DEPS_LIBS)

.c.o:
	$(CPP) -M -MF $(DEPDIR)/$*.Tpo $(DEPS_CFLAGS) -I../aurochs/include/  $<
	$(OCAMLC) $(OCAMLC_CCOPT) $(DEPS_OCAMLC_CCOPT) $<
	mv -f $(DEPDIR)/$*.Tpo $(DEPDIR)/$*.Po

aurochs_lib.o: ../aurochs/_build/aurochs_pack.cmx
	ocamlopt.opt -output-obj -o aurochs_lib.o unix.cmxa ../aurochs/_build/aurochs_pack.cmx	

CLEANFILES = *.o $(bin_PROGRAMS) 

#clean:
#	rm *.o ptfs
