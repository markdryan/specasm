VPATH=../../../src

.PHONY: all
all: specasm.tap salink.tap SAEXPORT SAIMPORT SAMAKE

CC=zcc
CFLAGS=+zx -SO3 --opt-code-size --max-allocs-per-node200000 -Cs "--disable-warning 85" -clib=sdcc_iy

include Make.include

SPECASM = \
	specasm.o \
	ld_parse.o \
	line_common.o \
	line_dump.o \
	line_dump_common.o \
	line_parse.o \
	line_parse_common.o \
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
	link_obj.o \
	map.o \
	state_base.o \
	peer_file_zx.o \
	util_print_zx.o \
	peer_zx.o

SAEXPORT = \
	error.o \
	line_common.o \
	state_base.o \
	line_dump.o \
	line_dump_common.o \
	state_dump.o \
	peer_file_zx.o \
	peer_zx.o \
	saexport.o

SAIMPORT = \
	error.o \
	state_base.o \
	ld_parse.o \
	line_common.o \
	line_parse.o \
	line_parse_common.o \
	state_parse.o \
	peer_file_zx.o \
	peer_zx.o \
	saimport.o

SAMAKE =\
	samake.o \
	error.o \
	state_base.o \
	peer_file_zx.o \
	peer_zx.o


specasm.tap: $(SPECASM)
	$(CC) $(CFLAGS) -zorg=24310 -startup=31 -o specasm-bare $^ -create-app
	cat ../../../bas/48/SPECLD.TAP specasm-bare.tap > specasm.tap

salink.tap: $(SALINK)
	$(CC) $(CFLAGS) -zorg=24000 -startup=31 -o salink-bare $^ -Cz"--noloader" -create-app
	cat ../../../bas/48/LINKLD.TAP salink-bare.tap > salink.tap

SAEXPORT: $(SAEXPORT)
	$(CC) $(CFLAGS) -startup=30 -o $@ $(SAEXPORT) -subtype=dotx -Cz"--clean" -create-app

SAIMPORT: $(SAIMPORT)
	$(CC) $(CFLAGS) -startup=30 -o $@ $(SAIMPORT) -subtype=dotx -Cz"--clean" -create-app

SAMAKE: $(SAMAKE)
	$(CC) $(CFLAGS) -startup=30 -o $@ $(SAMAKE) -subtype=dotx -Cz"--clean" -create-app

clean:
	- rm -rf specasm *.zip -rf unitzx
	- rm *.X *.o *.bin *.tap SAIMPORT SAEXPORT SAMAKE

.PHONY: release
release:
	- rm -rf release
	mkdir -p release/specasm
	cp specasm.tap salink.tap release/specasm
	cp ../../../COPYING release/specasm
	cp SAMAKE SAIMPORT SAEXPORT *.X release/specasm
	cp ../../../bas/48/INSTALL release/specasm
	cp ../../../bas/48/REMOVE release/specasm
	cd release && zip -r specasm48.zip specasm
