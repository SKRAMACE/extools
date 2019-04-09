# Set the processor type and import the toolchain
BUILD_CONFIG=x86_64.gcc
include tools.mk

HDR = radpool.h
LIB = libradpool
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

ifeq ($(debug),on)
CFLAGS += -ggdb
endif

.IGNORE: clean
.PHONY: install clean uninstall

SRC = \
	radpool.c

$(LIB): $(SRC)
	$(CC) $^ $(INC) $(LDFLAGS) $(CFLAGS) -o $(LIB)

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
