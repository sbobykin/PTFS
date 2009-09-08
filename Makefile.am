#make -C aurochs/
all:
	if test ! -e 'src/aurochs/.deps'; then echo > src/aurochs/.deps; fi
	make -C src/aurochs genmachine
	make -C src/aurochs deps
	make -C src/aurochs
	make -C src/

clean:
	make -C src/ clean
	make -C src/aurochs/ clean

install:
	make -C src/ install
