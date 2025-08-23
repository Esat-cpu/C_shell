CC = gcc
CFLAGS = -Wall -Wextra -I$(INCDIR)
TARGET = shell
SRCDIR = src
TESTDIR = tests
INCDIR = include

all: $(TARGET)

$(TARGET): $(SRCDIR)/*.c $(INCDIR)/*.h
	$(CC) $(CFLAGS) $(SRCDIR)/*.c -o $(TARGET)

test_cd_handle: $(TESTDIR)/cd_handle_test.c $(SRCDIR)/cd_handle.c
	$(CC) $(CFLAGS) $(TESTDIR)/cd_handle_test.c $(SRCDIR)/cd_handle.c -o cd_handle_test
	./cd_handle_test

test_trim: $(TESTDIR)/trim_test.c $(SRCDIR)/trim.c
	$(CC) $(CFLAGS) $(TESTDIR)/trim_test.c $(SRCDIR)/trim.c -o trim_test
	./trim_test

test: test_cd_handle test_trim

clean:
	rm -f $(TARGET) cd_handle_test trim_test

.PHONY: all test clean test_cd_handle test_trim

