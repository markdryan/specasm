VPATH=../../../src

.PHONY: all
all: specasm.tap

CC=zcc
CFLAGS=+zx -SO3 --opt-code-size --max-allocs-per-node200000 -Cs "--disable-warning 85" -clib=sdcc_iy -DSPECASM_TARGET_128
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

editor_banked.o: editor.c
	$(CC) $(CFLAGS) -DSPECASM_128_BANKED -o $@ --codesegBANK_6 --constsegBANK_6 --datasegBANK_6 -c $<

editor_extra_banked.o: editor_extra.c
	$(CC) $(CFLAGS) -DSPECASM_128_BANKED -o $@ --codesegBANK_6 --constsegBANK_6 --datasegBANK_6 -c $<

analysis_banked.o: analysis.c
	$(CC) $(CFLAGS) -DSPECASM_128_BANKED -o $@ --codesegBANK_6 --constsegBANK_6 --datasegBANK_6 -c $<

clipboard.o: clipboard.c
	$(CC) $(CFLAGS) -DSPECASM_128_BANKED -o $@ -c $<

include Make.include

SPECASM = \
	scratch.o \
	specasm.o \
	specasm_trampolines_128.c \
	line_common.o \
	clipboard.o \
	error.o \
	specasm_mainloop.o \
	state_base.o \
	editor_buffers.o \
	util_print_zx.o \
	peer_zx.o \
	line_parse_banked.o \
	state_parse_banked.o \
	line_parse_common_banked.o \
	line_dump_common_banked.o \
	line_dump_banked.o \
	state_dump_banked.o \
	ld_parse_banked.o \
	editor_banked.o \
	editor_extra_banked.o \
	analysis_banked.o

specasm_bare.tap: $(SPECASM)
	$(CC) $(CFLAGS) -zorg=32768 -m -startup=31 -o $@ $^ -pragma-include:zpragma.inc $(CZFLAGS)
	z88dk-appmake +zx -b specasm_bare_CODE.bin -o specasm_bare.tap --org 32768 --noloader

sald128/sald128: ../../../asm/sald128/sald128.s
	../../../buildbin.sh `realpath .` 128

specasm.tap: specasm_bare.tap sald128/sald128
	z88dk-appmake +zx -b specasm_bare_BANK_0.bin -o bank0.tap --org 49152 --noloader
	z88dk-appmake +zx -b specasm_bare_BANK_4.bin -o bank4.tap --org 49152 --noloader
	z88dk-appmake +zx -b specasm_bare_BANK_6.bin -o bank6.tap --org 49152 --noloader
	z88dk-appmake +zx -b sald128/sald128 -o sald128.tap --org 32768 --noloader
	cat ../../../bas/128/specld.tap sald128.tap bank0.tap bank4.tap bank6.tap specasm_bare.tap > specasm.tap

clean:
	- rm -rf specasm *.zip -rf unitzx sald128
	- rm *.X *.o *.bin *.tap
