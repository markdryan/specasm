VPATH = src sbc

BASE =\
	error.c \
	state_base.c

COMMON =\
	line_common.c

SRCS =\
	editor.c \
	editor_tests.c \
	editor_test_content.c \
	line_parse.c \
	line_dump.c \
	peer_unit.c \
	peer_posix_screen.c \
	peer_text_screen.c \
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
	line_parse.c \
	saimport.c \
	state_parse.c 

SAEXPORT =\
	line_dump.c \
	state_dump.c \
	saexport.c

SALINK =\
	salink.c \
	expression.c

TEST_CONTENT_ZX =\
	test_content.c \
	test_content_zx.c

SBC_COMMON =\
	sbc_overlay.c \
	sbc_lexer.c

SBC_LEX_TEST =\
	sbc_lex_test.c

CFLAGS += -Wall -MMD -DUNITTESTS -Isrc

all: unittests saimport saexport salink sbclextest

unittests: $(BASE:%.c=%.o) $(COMMON:%.c=%.o) $(SRCS:%.c=%.o)
	$(CC) $(CFLAGS) -o $@ $^

saimport: $(BASE:%.c=%.o) $(COMMON:%.c=%.o) $(POSIX:%.c=%.o) $(SAIMPORT:%.c=%.o)
	$(CC) $(CFLAGS) -o $@ $^

saexport: $(BASE:%.c=%.o) $(COMMON:%.c=%.o) $(POSIX:%.c=%.o) $(SAEXPORT:%.c=%.o)
	$(CC) $(CFLAGS) -o $@ $^

salink: $(BASE:%.c=%.o) $(POSIX:%.c=%.o) $(SALINK:%.c=%.o)
	$(CC) $(CFLAGS) -o $@ $^

sbclextest: $(BASE:%.c=%.o) $(POSIX:%.c=%.o) $(SBC_LEX_TEST:%.c=%.o) $(SBC_COMMON:%.c=%.o)
	$(CC) $(CFLAGS) -o $@ $^

test_content_zx: $(TEST_CONTENT_ZX:%.c=%.o)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	- rm *.d *.o unittests saimport saexport salink sbclextest

-include $(BASE:%.c=%.d)
-include $(COMMON:%.c=%.d)
-include $(SRCS:%.c=%.d)
-include $(POSIX:%.c=%.d)
-include $(SAIMPORT:%.c=%.d)
-include $(SAEXPORT:%.c=%.d)
-include $(SALINK:%.c=%.d)
-include $(TEST_CONTENT_ZX:%.c=%.d)
-include $(SBC_COMMON:%.c=%.d)
-include $(SBC_LEX_TEST:%.c=%.d)
