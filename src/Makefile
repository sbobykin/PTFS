# please set these variebles according to you system envieroment.
OCAMLLIBS=/usr/lib/ocaml/3.10.2/

ptfs: ptfs.o compilewrap.o aurochs_lib.o
	gcc -g -o ptfs ptfs.o compilewrap.o aurochs_lib.o \
	    -L ../aurochs/_build/ -L $(OCAMLLIBS) \
	    -laurochs -lunix -lasmrun -ldl -lm `pkg-config fuse --cflags --libs`

ptfs.o: ptfs.c
	ocamlc.opt -ccopt -I../aurochs/include/ -ccopt -g -ccopt -D_FILE_OFFSET_BITS=64 -c ptfs.c

compilewrap.o: compilewrap.c
	ocamlc.opt -ccopt -I../aurochs/include/ -ccopt -g -c compilewrap.c

aurochs_lib.o: ../aurochs/_build/aurochs_pack.cmx
	ocamlopt.opt -output-obj -o aurochs_lib.o unix.cmxa ../aurochs/_build/aurochs_pack.cmx	

clean:
	rm *.o ptfs
