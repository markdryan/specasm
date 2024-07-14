VPATH = src

BASE =\
	error.c \
	state_base.c

COMMON =\
	line_common.c

SRCS =\
	analysis.c \
	clipboard.c \
	editor.c \
	editor_buffers.c \
	editor_extra.c \
	editor_tests.c \
	editor_test_content.c \
	ld_parse.c \
	line_parse.c \
	line_parse_common.c \
	line_dump_common.c \
	line_dump.c \
	peer_unit.c \
	peer_posix_screen.c \
	peer_text_screen.c \
	scratch.c \
	state_dump.c \
	state_parse.c \
	test_content.c \
	unittests.c

POSIX = \
	peer_posix.c \
	peer_file_posix.c \
	peer_posix_screen.c \
	peer_text_screen.c

SAIMPORT =\
	ld_parse.c \
	line_parse.c \
	line_parse_common.c \
	saimport.c \
	scratch.c \
	state_parse.c 

SAEXPORT =\
	line_dump.c \
	line_dump_common.c \
	state_dump.c \
	saexport.c

SALINK =\
	link_obj.c \
	map.c \
	queued_files.c \
	salink.c \
	expression.c

SAMAKE =\
	samake.c

TEST_CONTENT_ZX =\
	test_content.c \
	test_content_zx.c

CFLAGS += -Wall -MMD -DUNITTESTS -Isrc

all: unittests saimport saexport salink samake

unittests: $(BASE:%.c=%.o) $(COMMON:%.c=%.o) $(SRCS:%.c=%.o)
	$(CC) $(CFLAGS) -o $@ $^

saimport: $(BASE:%.c=%.o) $(COMMON:%.c=%.o) $(POSIX:%.c=%.o) $(SAIMPORT:%.c=%.o)
	$(CC) $(CFLAGS) -o $@ $^

saexport: $(BASE:%.c=%.o) $(COMMON:%.c=%.o) $(POSIX:%.c=%.o) $(SAEXPORT:%.c=%.o)
	$(CC) $(CFLAGS) -o $@ $^

salink: $(BASE:%.c=%.o) $(POSIX:%.c=%.o) $(SALINK:%.c=%.o)
	$(CC) $(CFLAGS) -o $@ $^

test_content_zx: $(TEST_CONTENT_ZX:%.c=%.o)
	$(CC) $(CFLAGS) -o $@ $^

samake: $(BASE:%.c=%.o) $(POSIX:%.c=%.o) $(SAMAKE:%.c=%.o)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	- rm *.d *.o unittests saimport saexport salink samake

-include $(BASE:%.c=%.d)
-include $(COMMON:%.c=%.d)
-include $(SRCS:%.c=%.d)
-include $(POSIX:%.c=%.d)
-include $(SAIMPORT:%.c=%.d)
-include $(SAEXPORT:%.c=%.d)
-include $(SALINK:%.c=%.d)
-include $(SAMAKE:%.c=%.d)
-include $(TEST_CONTENT_ZX:%.c=%.d)
