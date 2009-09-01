# This file contains reuse notes. See the file 'REUSE' for details.
#ocamlopt.opt -output-obj -o aurochs_lib.o unix.cmxa ../aurochs/_build/aurochs_pack.cmx	

OCAMLC_CCOPT=-ccopt -I../aurochs/include/ -ccopt -g
DEPS_OCAMLC_CCOPT=`for i in $(DEPS_CFLAGS); do echo -n  " -ccopt $$i "; done;`

bin_PROGRAMS = ptfs
ptfs_SOURCES = ptfs.c ptfs.h parse.c get_node.c compilewrap.c fuse_ops.c rw.c
ptfs_LDADD=aurochs_lib.o
LIBS = -L aurochs/ -L $(OCAMLLIB) -laurochs -lunix -lasmrun -ldl -lm $(DEPS_LIBS)
#ptfs_DEPENDENCIES=$(ptfs_LDADD)
COMPILE=$(CPP)

#ptfs: ptfs.o compilewrap.o parse.o aurochs_lib.o get_node.o
#	gcc -g -o ptfs ptfs.o compilewrap.o aurochs_lib.o parse.o get_node.o\
#	    -L ../aurochs/_build/ -L $(OCAMLLIB) \
#	    -laurochs -lunix -lasmrun -ldl -lm $(DEPS_LIBS)

.c.o:
	$(CPP) -M -MF $(DEPDIR)/$*.Tpo $(DEPS_CFLAGS) -I../aurochs/include/  $<
	$(OCAMLC) $(OCAMLC_CCOPT) $(DEPS_OCAMLC_CCOPT) $<
	mv -f $(DEPDIR)/$*.Tpo $(DEPDIR)/$*.Po

aurochs_lib.o: aurochs/aurochs_pack.cmx
	ocamlopt.opt -output-obj -o aurochs_lib.o unix.cmxa aurochs/aurochs_pack.cmx	

CLEANFILES = *.o $(bin_PROGRAMS)

#dist-clean:
#	rm *.o ptfs
#	rm -r .deps
