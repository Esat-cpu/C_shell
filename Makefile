CC      := gcc
CFLAGS  = -Wall -Wextra -I$(INCDIR)
TARGET  := shell
SRCDIR  := src
TESTDIR := tests
TESTLIB := tests/test_lib
INCDIR  := include

all: $(TARGET)

$(TARGET): release_dir $(wildcard $(SRCDIR)/*.c $(INCDIR)/*.h)
	$(CC) $(CFLAGS) -O2 $(wildcard $(SRCDIR)/*.c) -lreadline -o build/release/$(TARGET)


test_cd_handle: test_dir $(TESTDIR)/cd_handle_test.c $(SRCDIR)/cd_handle.c
	@$(CC) $(CFLAGS) $(TESTDIR)/cd_handle_test.c $(SRCDIR)/cd_handle.c -o build/test/cd_handle_test
	@build/test/cd_handle_test


test_trim: test_dir $(TESTDIR)/trim_test.c $(SRCDIR)/trim.c
	@$(CC) $(CFLAGS) $(TESTDIR)/trim_test.c $(SRCDIR)/trim.c -o build/test/trim_test
	@build/test/trim_test


test_tokenize: test_dir $(TESTDIR)/tokenize_test.c $(SRCDIR)/tokenize.c
	@$(CC) $(CFLAGS) -I$(TESTLIB) -o build/test/tokenize_test \
		$(TESTLIB)/test_lib.c $(TESTDIR)/tokenize_test.c $(SRCDIR)/tokenize.c
	@build/test/tokenize_test



test_expansion: test_dir $(TESTDIR)/expansion_test.c $(SRCDIR)/expansion.c
	@$(CC) $(CFLAGS) -I$(TESTLIB) -o build/test/expansion_test \
		$(TESTLIB)/test_lib.c \
		$(TESTDIR)/expansion_test.c \
		$(SRCDIR)/expansion.c $(SRCDIR)/tokenize.c
	@build/test/expansion_test



test: test_cd_handle test_trim test_tokenize test_expansion


docs:
	@doxygen docs/Doxyfile
	@echo "Documentation generated at docs/html/index.html"


release_dir:
	@mkdir -p build/release

test_dir:
	@mkdir -p build/test

clean:
	rm -f $(wildcard build/release/* build/test/*)

distclean:
	rm -fr build


.PHONY: all \
	 clean distclean \
	 test test_cd_handle test_trim test_tokenize test_expansion \
	 release_dir test_dir \

