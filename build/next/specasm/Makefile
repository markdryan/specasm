VPATH=../../../src

.PHONY: all
all: SPECASM SALINK SAEXPORT SAIMPORT SAMAKE

CC=zcc
CFLAGS=+zxn -SO3 --opt-code-size --max-allocs-per-node200000 -Cs "--disable-warning 85" -clib=sdcc_iy -DSPECASM_TARGET_NEXT -DSPECASM_TARGET_NEXT_OPCODES
CZFLAGS=-Cz="--clean --fullsize --main-fence 0xDFE0"
SACZFLAGS=-Cz="--clean --fullsize --main-fence 0xBFFE"

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

editor_banked.o: editor.c
	$(CC) $(CFLAGS) -DSPECASM_NEXT_BANKED -o $@ --codesegBANK_45_L --constsegBANK_45_L --datasegBANK_45_L -c $<

editor_extra_banked.o: editor_extra.c
	$(CC) $(CFLAGS) -DSPECASM_NEXT_BANKED -o $@ --codesegBANK_45_H --constsegBANK_45_H --datasegBANK_45_H -c $<

analysis_banked.o: analysis.c
	$(CC) $(CFLAGS) -DSPECASM_NEXT_BANKED -o $@ --codesegBANK_45_H --constsegBANK_45_H --datasegBANK_45_H -c $<

expression_banked.o: expression.c
	$(CC) $(CFLAGS) -DSPECASM_NEXT_BANKED -o $@ --codesegBANK_43_H --constsegBANK_43_H --datasegBANK_43_H -c $<

map_banked.o: map.c
	$(CC) $(CFLAGS) -DSPECASM_NEXT_BANKED -o $@ --codesegBANK_44_H --constsegBANK_44_H --datasegBANK_44_H -c $<

queued_files_banked.o: queued_files.c
	$(CC) $(CFLAGS) -DSPECASM_NEXT_BANKED -o $@ --codesegBANK_44_H --constsegBANK_44_H --datasegBANK_44_H -c $<

link_obj_banked.o: link_obj.c
	$(CC) $(CFLAGS) -DSPECASM_NEXT_BANKED -o $@ --codesegBANK_45_H --constsegBANK_45_H --datasegBANK_45_H -c $<

clipboard.o: clipboard.c
	$(CC) $(CFLAGS) -DSPECASM_NEXT_BANKED -o $@ -c $<

include Make.include

SPECASM = \
	specasm_next.o \
	specasm_trampolines_next.o \
	specasm_mainloop.o \
	error.o \
	scratch.o \
	state_base.o \
	editor_buffers.o \
	util_print_next.o \
	peer_next.o \
	line_common.o \
	clipboard.o \
	line_parse_banked.o \
	ld_parse_banked.o \
	line_parse_common_banked.o \
	state_parse_banked.o \
	line_dump_banked.o \
	line_dump_common_banked.o \
	state_dump_banked.o \
	editor_banked.o \
	editor_extra_banked.o \
	analysis_banked.o

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
	scratch.o \
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
	$(CC) $(CFLAGS) -m -startup=31 -o $@ $^ -pragma-include:zpragmasa.inc -subtype=dotn $(SACZFLAGS) -create-app

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
	sed 's/```//g' ../../../docs/specasm.md > release/specasm/specasm.txt
	cp SPECASM SALINK SAIMPORT SAEXPORT SAMAKE release/specasm
	cp ../../../bas/next/INSTALL.BAS release/specasm
	cp ../../../bas/next/REMOVE.BAS release/specasm
	../../../buildlib.sh `realpath release` next
	cd release && zip -r specasmnext.zip specasm
