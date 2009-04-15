
bin_PROGRAMS = ptfs

OCAMLC_CCOPT=-ccopt -I../aurochs/include/ -ccopt -g -ccopt -D_FILE_OFFSET_BITS=64

ptfs: ptfs.o compilewrap.o aurochs_lib.o
	gcc -g -o ptfs ptfs.o compilewrap.o aurochs_lib.o \
	    -L ../aurochs/_build/ -L $(OCAMLLIB) \
	    -laurochs -lunix -lasmrun -ldl -lm `pkg-config fuse --cflags --libs`

ptfs.o: ptfs.c
	$(OCAMLC) $(OCAMLC_CCOPT) -c ptfs.c

compilewrap.o: compilewrap.c
	$(OCAMLC) $(OCAMLC_CCOPT) -c compilewrap.c

aurochs_lib.o: ../aurochs/_build/aurochs_pack.cmx
	ocamlopt.opt -output-obj -o aurochs_lib.o unix.cmxa ../aurochs/_build/aurochs_pack.cmx	

CLEANFILES = *.o $(bin_PROGRAMS) 

#clean:
#	rm *.o ptfs
