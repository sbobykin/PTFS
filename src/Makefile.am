
bin_PROGRAMS = ptfs

OCAMLC_CCOPT=-ccopt -I../aurochs/include/ -ccopt -g
DEPS_OCAMLC_CCOPT=`for i in $(DEPS_CFLAGS); do echo -n  " -ccopt $$i "; done;`

ptfs: ptfs.o compilewrap.o aurochs_lib.o
	gcc -g -o ptfs ptfs.o compilewrap.o aurochs_lib.o \
	    -L ../aurochs/_build/ -L $(OCAMLLIB) \
	    -laurochs -lunix -lasmrun -ldl -lm $(DEPS_LIBS)

ptfs.o: ptfs.c
	$(OCAMLC) $(OCAMLC_CCOPT) $(DEPS_OCAMLC_CCOPT) ptfs.c

compilewrap.o: compilewrap.c
	$(OCAMLC) $(OCAMLC_CCOPT) $(DEPS_OCAMLC_CCOPT) compilewrap.c

aurochs_lib.o: ../aurochs/_build/aurochs_pack.cmx
	ocamlopt.opt -output-obj -o aurochs_lib.o unix.cmxa ../aurochs/_build/aurochs_pack.cmx	

CLEANFILES = *.o $(bin_PROGRAMS) 

#clean:
#	rm *.o ptfs
