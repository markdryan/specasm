VPATH=../../../src

.PHONY: all
all: SPECASM SALINK SAEXPORT SAIMPORT SAMAKE

CC=zcc
CFLAGS=+zxn -SO3 --opt-code-size --max-allocs-per-node200000 -Cs "--disable-warning 85" -clib=sdcc_iy -DSPECASM_TARGET_NEXT -DSPECASM_TARGET_NEXT_OPCODES
CZFLAGS=-Cz="--clean --fullsize --main-fence 0xDFE0"

line_parse_banked.o: line_parse.c
	$(CC) $(CFLAGS) -DSPECASM_NEXT_BANKED -o $@ --codesegBANK_43_H --constsegBANK_43_H --datasegBANK_43_H -c $<

line_dump_banked.o: line_dump.c
	$(CC) $(CFLAGS) -DSPECASM_NEXT_BANKED -o $@ --codesegBANK_44_H --constsegBANK_44_H --datasegBANK_44_H -c $<

ld_parse_banked.o: ld_parse.c
	$(CC) $(CFLAGS) -DSPECASM_NEXT_BANKED -o $@ --codesegBANK_45_H --constsegBANK_45_H --datasegBANK_45_H -c $<

clipboard_banked.o: clipboard.c
	$(CC) $(CFLAGS) -DSPECASM_NEXT_BANKED -o $@ --codesegBANK_46_H --constsegBANK_46_H --datasegBANK_46_H -c $<

expression_banked.o: expression.c
	$(CC) $(CFLAGS) -DSPECASM_NEXT_BANKED -o $@ --codesegBANK_43_H --constsegBANK_43_H --datasegBANK_43_H -c $<

map_banked.o: map.c
	$(CC) $(CFLAGS) -DSPECASM_NEXT_BANKED -o $@ --codesegBANK_44_H --constsegBANK_44_H --datasegBANK_44_H -c $<

queued_files_banked.o: queued_files.c
	$(CC) $(CFLAGS) -DSPECASM_NEXT_BANKED -o $@ --codesegBANK_44_H --constsegBANK_44_H --datasegBANK_44_H -c $<

link_obj_banked.o: link_obj.c
	$(CC) $(CFLAGS) -DSPECASM_NEXT_BANKED -o $@ --codesegBANK_45_H --constsegBANK_45_H --datasegBANK_45_H -c $<

include Make.include

SPECASM = \
	specasm_next.o \
	specasm_trampolines.o \
	clipboard_banked.o \
	line_common.o \
	ld_parse_banked.o \
	line_dump_banked.o \
	line_dump_common.o \
	line_parse_banked.o \
	line_parse_common.o \
	specasm_mainloop.o \
	error.o \
	state_base.o \
	state_dump.o \
	state_parse.o \
	editor.o \
	util_print_next.o \
	peer_next.o

SALINK = \
	salink.o \
	error.o \
	expression_banked.o \
	link_obj_banked.o \
	map_banked.o \
	queued_files_banked.o \
	salink_trampolines.o \
	state_base.o \
	peer_file_next.o \
	util_print_next.o \
	peer_next.o

SAEXPORT = \
	error.o \
	line_common.o \
	state_base.o \
	line_dump.o \
	line_dump_common.o \
	state_dump.o \
	peer_file_next.o \
	peer_next.o \
	saexport.o

SAIMPORT = \
	error.o \
	state_base.o \
	ld_parse.o \
	line_common.o \
	line_parse.o \
	line_parse_common.o \
	state_parse.o \
	peer_file_next.o \
	peer_next.o \
	saimport.o

SAMAKE =\
	samake.o \
	error.o \
	state_base.o \
	peer_file_next.o \
	peer_next.o

SPECASM: $(SPECASM)
	$(CC) $(CFLAGS) -m -startup=31 -o $@ $^ -pragma-include:zpragma.inc -subtype=dotn $(CZFLAGS) -create-app

SALINK: $(SALINK)
	$(CC) $(CFLAGS) -m -startup=31 -o $@ $^ -pragma-include:zpragma.inc -subtype=dotn $(CZFLAGS) -create-app

SAEXPORT: $(SAEXPORT)
	$(CC) $(CFLAGS) -startup=30 -o $@ $(SAEXPORT) -subtype=dotn -Cz"--clean" -create-app

SAIMPORT: $(SAIMPORT)
	$(CC) $(CFLAGS) -startup=30 -o $@ $(SAIMPORT) -subtype=dotn -Cz"--clean" -create-app

SAMAKE: $(SAMAKE)
	$(CC) $(CFLAGS) -startup=30 -o $@ $(SAMAKE) -subtype=dotn -Cz"--clean" -create-app

clean:
	- rm -rf specasm *.zip
	- rm *.X *.o *.bin SPECASM SALINK SAIMPORT SAEXPORT SAMAKE

.PHONY: release
release:
	- rm -rf release
	mkdir -p release/specasm
	cp ../../../COPYING release/specasm
	cp SPECASM SALINK SAIMPORT SAEXPORT SAMAKE release/specasm
	cp ../../../bas/next/INSTALL release/specasm
	cp ../../../bas/next/REMOVE release/specasm
	../../../buildlib.sh `realpath release` next
	cd release && zip -r specasmnext.zip specasm
