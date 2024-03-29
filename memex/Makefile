# Set the processor type and import the toolchain
BUILD_CONFIG=x86_64.gcc
include tools.mk

HDR = memex.h
LIB = libmemex
VERSION = 1.0.0
SO_VERSION = 0
LIBFILE = $(LIB).so.$(VERSION)

NOLDCONFIG ?= n

# Common prefix for installation directories
# NOTE: This directory must exist when you start the install
prefix ?= /usr/local
exec_prefix = $(prefix)
# Where to put library files
includedir = $(prefix)/include
# Where to put header files
libdir = $(exec_prefix)/lib

INC = -I./include
SRC_DIR = src
OUT_DIR = .
VPATH = $(SRC_DIR)

# Flags
CFLAGS += -Werror -fPIC -shared
LDFLAGS += -Wl,-soname,$(LIBFILE)
TEST_CFLAGS=\
    -ggdb \

ifeq ($(debug),on)
CFLAGS += -ggdb
endif

.IGNORE: clean
.PHONY: install clean uninstall

SRC=\
    memex-log.c \
	pool.c \
	list.c \
	sort.c \
	cleanup.c

TESTLIBS = \
	-ltestex \
    -lpthread \
    -luuid \

%.o: %.c
	$(CC) $(LDFLAGS) $(CFLAGS) -c $^

$(LIB): $(SRC)
	$(CC) $^ $(INC) $(LDFLAGS) $(CFLAGS) -o $(LIB)

memex-test: test/test.c $(SRC)
	$(CC) $^ -ggdb $(INC) -o memex-test

memex-list-test: $(SRC)
	$(CC) $(TEST_CFLAGS) test/list-test.c $^ $(INC) -o test/bin/$@ $(TESTLIBS)

memex-pool-test: $(SRC)
	$(CC) $(TEST_CFLAGS) test/pool-test.c $^ $(INC) -o test/bin/$@ $(TESTLIBS)

memex-sort-test: $(SRC)
	$(CC) $(TEST_CFLAGS) test/sort-test.c $^ $(INC) -o test/bin/$@ $(TESTLIBS)

tests: memex-sort-test memex-pool-test memex-list-test

install: $(LIB)
	install -m 0755 $(LIB) -D $(DESTDIR)$(libdir)/$(LIBFILE)
	cd $(DESTDIR)$(libdir); \
		ln -f -s $(LIB).so.$(VERSION) $(LIB).so.$(SO_VERSION); \
		ln -f -s $(LIB).so.$(SO_VERSION) $(LIB).so
	install -m 0644 include/$(HDR) -D $(DESTDIR)$(includedir)/$(HDR)
ifneq ($(NOLDCONFIG),y)
	ldconfig
endif

uninstall:
	rm -f $(DESTDIR)$(libdir)/$(LIB)*
	rm -f $(DESTDIR)$(includedir)/$(HDR)
ifneq ($(NOLDCONFIG),y)
	ldconfig
endif

clean:
	rm -f $(LIB)*
	rm -f *.o
	rm -f test/bin/*
