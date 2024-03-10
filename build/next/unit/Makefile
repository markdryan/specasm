VPATH=../../../src

.PHONY: all
all: UNITNEXT

CC=zcc
CFLAGS=+zxn -SO3 --opt-code-size --max-allocs-per-node200000 -Cs "--disable-warning 85" -clib=sdcc_iy -DSPECASM_TARGET_NEXT -DSPECASM_TARGET_NEXT_OPCODES -DUNITTESTS
CZFLAGS=-Cz="--clean --fullsize --main-fence 0xDFE0"

line_parse_banked.o: line_parse.c
	$(CC) $(CFLAGS) -DSPECASM_NEXT_BANKED -o $@ --codesegBANK_43_H --constsegBANK_43_H --datasegBANK_43_H -c $<

line_dump_banked.o: line_dump.c
	$(CC) $(CFLAGS) -DSPECASM_NEXT_BANKED -o $@ --codesegBANK_44_H --constsegBANK_44_H --datasegBANK_44_H -c $<

ld_parse_banked.o: ld_parse.c
	$(CC) $(CFLAGS) -DSPECASM_NEXT_BANKED -o $@ --codesegBANK_45_H --constsegBANK_45_H --datasegBANK_45_H -c $<

clipboard_banked.o: clipboard.c
	$(CC) $(CFLAGS) -DSPECASM_NEXT_BANKED -o $@ --codesegBANK_46_H --constsegBANK_46_H --datasegBANK_46_H -c $<

include Make.include

UNITNEXT = \
	error.o \
	state_base.o \
	specasm_trampolines.o \
	line_common.o \
	clipboard_banked.o \
	ld_parse_banked.o \
	line_dump_banked.o \
	line_dump_common.o \
	line_parse_banked.o \
	line_parse_common.o \
	state_dump.o \
	state_parse.o \
	peer_file_next.o \
	util_print_next.o \
	peer_unit.o \
	unittests_zx.o

UNITNEXT: $(UNITNEXT)
	$(CC) $(CFLAGS) -m -startup=31 -o $@ $^ -pragma-include:zpragma.inc -subtype=dotn $(CZFLAGS) -create-app

clean:
	- rm -rf unitzx
	- rm *.X *.o *.bin

.PHONY: tests
tests:
	- rm -rf tests
	mkdir tests
	cd ../../.. && CFLAGS=-DSPECASM_TARGET_NEXT make -f Makefile test_content_zx
	../../../test_content_zx tests
	cp unitnext tests
