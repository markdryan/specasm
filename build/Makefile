VPATH=../src

.PHONY: all
all: specasm.tap salink.tap SAEXPORT SAIMPORT

CC=zcc
CFLAGS=+zx -SO3 --opt-code-size --max-allocs-per-node200000 -Cs "--disable-warning 85" -clib=sdcc_iy

include Make.include

SPECASM = \
	specasm.o \
	line_common.o \
	line_dump.o \
	line_parse.o \
	error.o \
	state_base.o \
	state_dump.o \
	state_parse.o \
	editor.o \
	util_print_zx.o \
	peer_zx.o

SALINK = \
	salink.o \
	error.o \
	expression.o \
	state_base.o \
	peer_file_zx.o \
	util_print_zx.o \
	peer_zx.o

SAEXPORT = \
	error.o \
	line_common.o \
	state_base.o \
	line_dump.o \
	state_dump.o \
	peer_file_zx.o \
	util_print_zx.o \
	peer_zx.o \
	saexport.o

SAIMPORT = \
	error.o \
	state_base.o \
	line_common.o \
	line_parse.o \
	state_parse.o \
	peer_file_zx.o \
	util_print_zx.o \
	peer_zx.o \
	saimport.o

specasm.tap: $(SPECASM)
	$(CC) $(CFLAGS) -zorg=24310 -startup=31 -o specasm-bare $^ -create-app
	cat ../bas/SPECLD.TAP specasm-bare.tap > specasm.tap

salink.tap: $(SALINK)
	$(CC) $(CFLAGS) -zorg=24000 -startup=31 -o salink $^ -create-app

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
	cp specasm.tap salink.tap release/specasm
	cp SAIMPORT SAEXPORT *.X release/specasm
	cp ../bas/INSTALL release/specasm
	cp ../bas/REMOVE release/specasm
	cd release && zip -r specasm.zip specasm
