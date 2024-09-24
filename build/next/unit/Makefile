VPATH=../../../src

.PHONY: all
all: UNITNEXT

util_print_acc_next.o: util_print_acc_next.asm
	$(CC) $(CFLAGS) -o $@ -c $<

CC=zcc
CFLAGS=+zxn -SO3 --opt-code-size --max-allocs-per-node200000 -Cs "--disable-warning 85" -clib=sdcc_iy -DSPECASM_TARGET_NEXT -DSPECASM_TARGET_NEXT_OPCODES -DUNITTESTS
CZFLAGS=-Cz="--clean --fullsize --main-fence 0xBFFE"

line_parse_banked.o: line_parse.c
	$(CC) $(CFLAGS) -DSPECASM_NEXT_BANKED -o $@ --codesegBANK_43_L --constsegBANK_43_L --datasegBANK_43_L -c $<

ld_parse_banked.o: ld_parse.c
	$(CC) $(CFLAGS) -DSPECASM_NEXT_BANKED -o $@ --codesegBANK_43_H --constsegBANK_43_H --datasegBANK_43_H -c $<

state_parse_banked.o: state_parse.c
	$(CC) $(CFLAGS) -DSPECASM_NEXT_BANKED -o $@ --codesegBANK_43_H --constsegBANK_43_H --datasegBANK_43_H -c $<

line_parse_common_banked.o: line_parse_common.c
	$(CC) $(CFLAGS) -DSPECASM_NEXT_BANKED -o $@ --codesegBANK_43_H --constsegBANK_43_H --datasegBANK_43_H -c $<

line_dump_banked.o: line_dump.c
	$(CC) $(CFLAGS) -DSPECASM_NEXT_BANKED -o $@ --codesegBANK_44_L --constsegBANK_44_L --datasegBANK_44_L -c $<

line_dump_common_banked.o: line_dump_common.c
	$(CC) $(CFLAGS) -DSPECASM_NEXT_BANKED -o $@ --codesegBANK_44_H --constsegBANK_44_H --datasegBANK_44_H -c $<

state_dump_banked.o: state_dump.c
	$(CC) $(CFLAGS) -DSPECASM_NEXT_BANKED -o $@ --codesegBANK_44_H --constsegBANK_44_H --datasegBANK_44_H -c $<

clipboard.o: clipboard.c
	$(CC) $(CFLAGS) -DSPECASM_NEXT_BANKED -o $@ -c $<

peer_unit.o: peer_unit.c
	$(CC) $(CFLAGS) -DSPECASM_NEXT_BANKED -o $@ -c $<

include Make.include

UNITNEXT = \
	specasm_trampolines_next.o \
	error.o \
	scratch.o \
	state_base.o \
	line_common.o \
	peer_file_next.o \
	unittests_zx.o \
	util_print_next.o \
	util_print_acc_next.o \
	peer_unit.o \
	clipboard.o \
	line_parse_banked.o \
	ld_parse_banked.o \
	line_parse_common_banked.o \
	state_parse_banked.o \
	line_dump_banked.o \
	line_dump_common_banked.o \
	state_dump_banked.o

UNITNEXT: $(UNITNEXT)
	$(CC) $(CFLAGS) -m -startup=31 -o $@ $^ -pragma-include:zpragma.inc -subtype=dotn $(CZFLAGS) -create-app

clean:
	- rm -rf unitzx
	- rm *.X *.o *.bin

.PHONY: tests
tests:
	- rm -rf tests
	mkdir tests
	cd ../../.. && make -f Makefile clean && CFLAGS="-DSPECASM_TARGET_NEXT -DSPECASM_TARGET_NEXT_OPCODES" make -f Makefile test_content_zx
	../../../test_content_zx tests
	cp unitnext tests
