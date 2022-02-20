VPATH=../../src

CC=zcc
CFLAGS=+zx -SO2 -clib=sdcc_ix --reserve-regs-iy --opt-code-size --max-allocs-per-node200000 -Cs "--disable-warning 85"

all: SAEXPORT SAIMPORT

include ../Make.include

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


SAEXPORT: $(SAEXPORT)
	$(CC) $(CFLAGS) -startup=30 -o $@ $(SAEXPORT) -subtype=dotx -Cz"--clean" -create-app

SAIMPORT: $(SAIMPORT)
	$(CC) $(CFLAGS) -startup=30 -o $@ $(SAIMPORT) -subtype=dotx -Cz"--clean" -create-app

clean:
	- rm *.o *.bin *.X SAIMPORT SAEXPORT



