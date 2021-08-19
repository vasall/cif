# Options
DEBUG ?= 1
NO_ZLIB ?= 0

# Target names
TARGET_BIN ?= cif
TARGET_LIB ?= cif

# Programs
CC ?= gcc
AR ?= gcc-ar
RM ?= rm -f
MKDIR ?= mkdir
CP ?= cp

# Directories
prefix ?= /usr/local
bindir ?= bin
libdir ?= lib
includedir ?= include
datarootdir ?= share
mandir ?= man

##########

OBJECTS_BIN = src/cif.o src/cif_bin.o
OBJECTS_STATIC = src/cif.o
OBJECTS_SHARED = src/cif.shared.o

HEADER = src/cif.h

MANPAGE = cif.1

TARGET_STATIC = lib$(TARGET_LIB).a
TARGET_SHARED = lib$(TARGET_LIB).so

_CFLAGS = -Wall -Wextra
ifeq ($(DEBUG), 1)
_CFLAGS += -g -Og
else
_CFLAGS += -O3 -flto
endif

ifeq ($(NO_ZLIB), 1)
_CFLAGS += -DNO_ZLIB
else
LDLIBS_LIB = -lz
endif

CFLAGS := $(_CFLAGS) $(CFLAGS)

# LDLIBS_LIB = 
LDLIBS_BIN = $(LDLIBS_LIB) -lpng

##########

.PHONY: all bin static shared clean install uninstall

all: bin static shared

bin: $(TARGET_BIN)

static: $(TARGET_STATIC)

shared: $(TARGET_SHARED)

$(TARGET_BIN): $(OBJECTS_BIN)
	$(CC) -o $@ $^ $(LDLIBS_BIN)

$(TARGET_STATIC): $(OBJECTS_STATIC)
	$(AR) rcs $@ $^

$(TARGET_SHARED): $(OBJECTS_SHARED)
	$(CC) -o $@ -shared $^ $(LDLIBS_SHARED)

%.shared.o: %.c
	$(CC) -o $@ -c -fPIC $(CFLAGS) $^

%.o: %.c
	$(CC) -o $@ -c $(CFLAGS) $^

clean:
	$(RM) $(TARGET_BIN)
	$(RM) $(TARGET_STATIC)
	$(RM) $(TARGET_SHARED)
	$(RM) $(OBJECTS_BIN)
	$(RM) $(OBJECTS_STATIC)
	$(RM) $(OBJECTS_SHARED)

install:
	$(MKDIR) -p $(DESTDIR)$(prefix)/$(bindir)
	$(MKDIR) -p $(DESTDIR)$(prefix)/$(includedir)
	$(MKDIR) -p $(DESTDIR)$(prefix)/$(libdir)
	$(MKDIR) -p $(DESTDIR)$(prefix)/$(datarootdir)/$(mandir)/man1
	-$(CP) $(TARGET_BIN) $(DESTDIR)$(prefix)/$(bindir)
	-$(CP) $(HEADER) $(DESTDIR)$(prefix)/$(includedir)
	-$(CP) $(TARGET_STATIC) $(DESTDIR)$(prefix)/$(libdir)
	-$(CP) $(TARGET_SHARED) $(DESTDIR)$(prefix)/$(libdir)
	-$(CP) $(MANPAGE) $(DESTDIR)$(prefix)/$(datarootdir)/$(mandir)/man1

uninstall:
	$(RM) $(DESTDIR)$(prefix)/$(bindir)/$(TARGET_BIN)
	$(RM) $(DESTDIR)$(prefix)/$(includedir)/$(HEADER:src/%=%)
	$(RM) $(DESTDIR)$(prefix)/$(libdir)/$(TARGET_STATIC)
	$(RM) $(DESTDIR)$(prefix)/$(libdir)/$(TARGET_SHARED)
	$(RM) $(DESTDIR)$(prefix)/$(datarootdir)/$(mandir)/man1/$(MANPAGE)