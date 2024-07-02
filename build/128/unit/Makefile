VPATH=../../../src

.PHONY: all
all: unit128.tap

CC=zcc
CFLAGS=+zx -SO3 --opt-code-size --max-allocs-per-node200000 -Cs "--disable-warning 85" -clib=sdcc_iy -DSPECASM_TARGET_128 -DUNITTESTS
CZFLAGS=-Cz="--clean --fullsize --main-fence 0xBFFE"

line_parse_banked.o: line_parse.c
	$(CC) $(CFLAGS) -DSPECASM_128_BANKED -o $@ --codesegBANK_0 --constsegBANK_0 --datasegBANK_0 -c $<

ld_parse_banked.o: ld_parse.c
	$(CC) $(CFLAGS) -DSPECASM_128_BANKED -o $@ --codesegBANK_0 --constsegBANK_0 --datasegBANK_0 -c $<

state_parse_banked.o: state_parse.c
	$(CC) $(CFLAGS) -DSPECASM_128_BANKED -o $@ --codesegBANK_0 --constsegBANK_0 --datasegBANK_0 -c $<

line_parse_common_banked.o: line_parse_common.c
	$(CC) $(CFLAGS) -DSPECASM_128_BANKED -o $@ --codesegBANK_0 --constsegBANK_0 --datasegBANK_0 -c $<

line_dump_common_banked.o: line_dump_common.c
	$(CC) $(CFLAGS) -DSPECASM_128_BANKED -o $@ --codesegBANK_4 --constsegBANK_4 --datasegBANK_4 -c $<

line_dump_banked.o: line_dump.c
	$(CC) $(CFLAGS) -DSPECASM_128_BANKED -o $@ --codesegBANK_4 --constsegBANK_4 --datasegBANK_4 -c $<

state_dump_banked.o: state_dump.c
	$(CC) $(CFLAGS) -DSPECASM_128_BANKED -o $@ --codesegBANK_4 --constsegBANK_4 --datasegBANK_4 -c $<

clipboard_banked.o: clipboard.c
	$(CC) $(CFLAGS) -DSPECASM_128_BANKED -o $@ --codesegBANK_1 --constsegBANK_1 --datasegBANK_1 -c $<

peer_unit.o: peer_unit.c
	$(CC) $(CFLAGS) -DSPECASM_128_BANKED -o $@ -c $<

include Make.include

UNIT128 = \
	specasm_trampolines_128.o \
	error.o \
	scratch.o \
	state_base.o \
	line_common.o \
	peer_file_zx.o \
	unittests_zx.o \
	util_print_zx.o \
	peer_unit.o \
	line_parse_banked.o \
	ld_parse_banked.o \
	line_parse_common_banked.o \
	state_parse_banked.o \
	clipboard_banked.o \
	line_dump_banked.o \
	line_dump_common_banked.o \
	state_dump_banked.o

unit128_bare.tap: $(UNIT128)
	$(CC) $(CFLAGS) -zorg=32768 -m -startup=31 -o $@ $^ -pragma-include:zpragma.inc $(CZFLAGS)
	z88dk-appmake +zx -b unit128_bare_CODE.bin -o unit128_bare.tap --org 32768 --noloader

unit128.tap: unit128_bare.tap
	z88dk-appmake +zx -b unit128_bare_BANK_0.bin -o bank0.tap --org 49152 --noloader
	z88dk-appmake +zx -b unit128_bare_BANK_1.bin -o bank1.tap --org 49152 --noloader
	z88dk-appmake +zx -b unit128_bare_BANK_4.bin -o bank4.tap --org 49152 --noloader
	cat ../../../bas/128/u128ld.tap bank0.tap bank1.tap bank4.tap unit128_bare.tap > unit128.tap


clean:
	- rm -rf unit128
	- rm *.X *.o *.bin *.tap

.PHONY: tests
tests:
	- rm -rf tests
	mkdir tests
	cd ../../.. && make -f Makefile clean && make -f Makefile test_content_zx
	../../../test_content_zx tests
	cp unit128.tap tests
