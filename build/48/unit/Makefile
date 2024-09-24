VPATH=../../../src

.PHONY: all
all: unitzx.tap

CC=zcc
CFLAGS=+zx -SO3 --opt-code-size --max-allocs-per-node200000 -Cs "--disable-warning 85" -clib=sdcc_iy -DUNITTESTS

util_print_acc_zx.o: util_print_acc_zx.asm
	$(CC) $(CFLAGS) -o $@ -c $<

include Make.include

UNITZX = \
	error.o \
	state_base.o \
	ld_parse.o \
	line_common.o \
	line_dump.o \
	line_dump_common.o \
	line_parse.o \
	line_parse_common.o \
	scratch.o \
	state_dump.o \
	state_parse.o \
	peer_file_zx.o \
	util_print_zx.o \
	util_print_acc_zx.o \
	peer_unit.o \
	unittests_zx.o

unitzx.tap: $(UNITZX)
	$(CC) $(CFLAGS) -zorg=24000 -startup=31 -o unitzx $^ -create-app

clean:
	- rm -rf unitzx
	- rm *.X *.o *.bin *.tap

.PHONY: tests
tests:
	- rm -rf tests
	mkdir tests
	cd ../../.. && make -f Makefile clean && make -f Makefile test_content_zx
	../../../test_content_zx tests
	cp unitzx.tap tests
