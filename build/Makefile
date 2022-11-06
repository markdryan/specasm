VPATH=../src

.PHONY: all
all: specasm.tap salink.tap unitzx.tap SAEXPORT SAIMPORT

CC=zcc
CFLAGS=+zx -SO2 --opt-code-size --max-allocs-per-node200000 -Cs "--disable-warning 85" -clib=sdcc_iy

include Make.include

SPECASM = \
	specasm.o \
	line.o \
	error.o \
	state_base.o \
	state.o \
	editor.o \
	peer_zx.o

SALINK = \
       salink.o \
       error.o \
       state_base.o \
       peer_file_zx.o \
       peer_zx.o

SAEXPORT = \
	 error.o \
	 state_base.o \
	 line.o \
	 state.o \
	 peer_file_zx.o \
	 peer_zx.o \
	 saexport.o

SAIMPORT = \
	 error.o \
	 state_base.o \
	 line.o \
	 state.o \
	 peer_file_zx.o \
	 peer_zx.o \
	 saimport.o

UNITZX = \
	 error.o \
	 state_base.o \
	 line.o \
	 state.o \
	 peer_file_zx.o \
	 peer_zx.o \
	 unittests_zx.o


specasm.tap: $(SPECASM)
	$(CC) $(CFLAGS) -zorg=24550 -startup=31 -o specasm $^ -create-app -Cz--merge=../bas/SPECLD.TAP

salink.tap: $(SALINK)
	$(CC) $(CFLAGS) -zorg=24000 -startup=31 -o salink $^ -create-app

unitzx.tap: $(UNITZX)
	$(CC) $(CFLAGS) -zorg=24000 -startup=31 -o unitzx $^ -create-app

SAEXPORT: $(SAEXPORT)
	$(CC) $(CFLAGS) -startup=30 -o $@ $(SAEXPORT) -subtype=dotx -Cz"--clean" -create-app

SAIMPORT: $(SAIMPORT)
	$(CC) $(CFLAGS) -startup=30 -o $@ $(SAIMPORT) -subtype=dotx -Cz"--clean" -create-app

clean:
	- rm -rf specasm *.zip -rf unitzx
	- rm *.X *.o *.bin *.tap SAIMPORT SAEXPORT

.PHONY: release
release:
	- rm -rf release
	mkdir -p release/specasm
	cp *.tap release/specasm
	cp SAIMPORT SAEXPORT *.X release/specasm
	cp ../bas/INSTALL release/specasm
	cp ../bas/REMOVE release/specasm
	cd release && zip -r specasm.zip specasm

.PHONY: tests
tests:
	- rm -rf unitzx
	mkdir unitzx
	cd .. && make -f Makefile test_content_zx
	../test_content_zx unitzx
	cp unitzx.tap unitzx
