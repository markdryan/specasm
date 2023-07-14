.PHONY: all
all: sbcfmt

CC=zcc
CFLAGS=+zx -SO3 --opt-code-size --max-allocs-per-node200000 -Cs "--disable-warning 85" --math32 -clib=sdcc_iy -I../src

include Make.include

SBCFMT = \
	sbc_error.o \
	sbc_lexer.o \
	sbc_overlay.o \
	sbc_parser.o \
	sbc_exp.o \
	sbc_fmt.o \
	sbc_fmt_utils.o \
	../src/peer_file_zx.o

sbcfmt: $(SBCFMT)
	$(CC) $(CFLAGS) -startup=4 -o $@ $(SBCFMT) -Cz"--clean" -create-app -pragma-define:CLIB_OPT_PRINTF=0x4000201


clean:
	- rm *.X *.o *.bin *.tap sbcfmt
