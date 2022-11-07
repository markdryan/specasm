VPATH = src

BASE =\
	error.c \
	state_base.c

COMMON =\
	line.c \
	state.c

SRCS =\
	editor.c \
	editor_tests.c \
	editor_test_content.c \
	peer_unit.c \
	peer_posix_screen.c \
	test_content.c \
	unittests.c

POSIX = \
	peer_posix.c \
	peer_file_posix.c \
	peer_posix_screen.c

SAIMPORT =\
	saimport.c

SAEXPORT =\
	saexport.c

SALINK =\
	salink.c

TEST_CONTENT_ZX =\
	test_content.c \
	test_content_zx.c

CFLAGS += -Wall -MMD -DUNITTESTS -Isrc

all: unittests saimport saexport salink

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

clean:
	- rm *.d *.o unittests saimport saexport salink

-include $(BASE:%.c=%.d)
-include $(COMMON:%.c=%.d)
-include $(SRCS:%.c=%.d)
-include $(POSIX:%.c=%.d)
-include $(SAIMPORT:%.c=%.d)
-include $(SAEXPORT:%.c=%.d)
-include $(SALINK:%.c=%.d)
-include $(TEST_CONTENT_ZX:%.c=%.d)
