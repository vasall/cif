DEBUG ?= 1
NO_ZLIB ?= 0

CC ?= gcc
AR ?= gcc-ar
RM ?= rm -f
CP ?= cp

TARGET_BIN ?= cif
TARGET_LIB ?= cif

CFLAGS := -Wall -Wextra $(CFLAGS)
LDFLAGS := -Wall -Wextra $(LDFLAGS)

prefix ?= /usr/local
bindir ?= bin
libdir ?= lib
includedir ?= include
datarootdir ?= share
mandir ?= man

ifeq ($(DEBUG), 1)
	CFLAGS := -g -Og $(CFLAGS)
	LDFLAGS := -g -Og $(LDFLAGS)
else
	CFLAGS := -O3 -flto $(CFLAGS)
	LDFLAGS := -O3 -flto -s $(CFLAGS)
endif

CFLAGS_LIB_SHARED := -fPIC $(CFLAGS)
LDFLAGS_LIB_SHARED := -shared $(LDFLAGS)

TARGET_LIB_STATIC := lib$(TARGET_LIB).a
TARGET_LIB_SHARED := lib$(TARGET_LIB).so

LDLIBS_BIN := $(TARGET_LIB_STATIC) -lpng
LDLIBS_SHARED :=

ifeq ($(NO_ZLIB), 1)
	CFLAGS += -DNO_ZLIB
	CFLAGS_LIB_SHARED += -DNO_ZLIB
else
	LDLIBS_BIN += -lz
	LDLIBS_SHARED += -lz
endif

SOURCES_BIN := src/cif_bin.c
SOURCES_LIB := src/cif.c

OBJECTS_BIN := $(SOURCES_BIN:%.c=%.o)
OBJECTS_LIB_STATIC := $(SOURCES_LIB:%.c=%.o)
OBJECTS_LIB_SHARED := $(SOURCES_LIB:%.c=%.shared.o)


.PHONY: all clean install uninstall bin static shared

all: bin static shared

bin: $(TARGET_LIB_STATIC) $(TARGET_BIN)

static: $(TARGET_LIB_STATIC)

shared: $(TARGET_LIB_SHARED)

$(TARGET_BIN): $(OBJECTS_BIN)
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS_BIN)

$(TARGET_LIB_STATIC): $(OBJECTS_LIB_STATIC)
	$(AR) rcs $@ $^

$(TARGET_LIB_SHARED): $(OBJECTS_LIB_SHARED)
	$(CC) $(LDFLAGS_LIB_SHARED) -o $@ $^ $(LDLIBS_SHARED)

$(OBJECTS_BIN) $(OBJECTS_LIB_STATIC): %.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<

$(OBJECTS_LIB_SHARED): %.shared.o: %.c
	$(CC) -c $(CFLAGS_LIB_SHARED) -o $@ $<

clean:
	$(RM) $(TARGET_LIB_STATIC) $(TARGET_LIB_SHARED) $(TARGET_BIN) $(OBJECTS_BIN) $(OBJECTS_LIB_STATIC) $(OBJECTS_LIB_SHARED)

install:
	-$(CP) $(TARGET_BIN) $(DESTDIR)$(prefix)/$(bindir)
	-$(CP) src/cif.h $(DESTDIR)$(prefix)/$(includedir)
	-$(CP) $(TARGET_LIB_STATIC) $(DESTDIR)$(prefix)/$(libdir)
	-$(CP) $(TARGET_LIB_SHARED) $(DESTDIR)$(prefix)/$(libdir)
	-$(CP) cif.1 $(DESTDIR)$(prefix)/$(datarootdir)/$(mandir)

uninstall:
	$(RM) $(DESTDIR)$(prefix)/$(bindir)/$(TARGET_BIN)
	$(RM) $(DESTDIR)$(prefix)/$(includedir)/cif.h
	$(RM) $(DESTDIR)$(prefix)/$(libdir)/$(TARGET_LIB_STATIC)
	$(RM) $(DESTDIR)$(prefix)/$(libdir)/$(TARGET_LIB_SHARED)
	$(RM) $(DESTDIR)$(prefix)/$(datarootdir)/$(mandir)/cif.1