VPATH=../../src

CC=zcc
CFLAGS=+zx -SO2 --opt-code-size --max-allocs-per-node200000 -Cs "--disable-warning 85" -clib=sdcc_iy
all: specasm.tap salink.tap

include ../Make.include

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

specasm.tap: $(SPECASM)
	$(CC) $(CFLAGS) -zorg=24550 -startup=31 -o specasm $^ -create-app -Cz--merge=../../bas/SPECLD.TAP

salink.tap: $(SALINK)
	$(CC) $(CFLAGS) -zorg=24000 -startup=31 -o salink $^ -create-app

clean:
	- rm *.o *.bin *.tap



