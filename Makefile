# Abscom - plain Makefile build (alternative to Meson/CMake).
#
#   make            build the static and shared libraries plus the examples
#   make test       build and run the test suite
#   make install    install headers, libraries and the pkg-config file
#   make clean      remove build outputs
#
# Useful variables (override on the command line):
#   make CC=clang CFLAGS="-O0 -g" PREFIX=$HOME/.local
#   make build-make BUILDDIR=out       pick a different build directory
#   make SHARED=no                     skip the shared library

VERSION := 0.1.0

CC      ?= cc
AR      ?= ar
RANLIB  ?= ranlib
PREFIX  ?= /usr/local

CFLAGS  ?= -O2
CFLAGS  += -std=gnu11 -Wall -Wextra -Iinclude
LDFLAGS ?=

BUILDDIR ?= build-make
OBJDIR   := $(BUILDDIR)/obj
TESTDIR  := $(BUILDDIR)/tests
EXEDIR   := $(BUILDDIR)/examples

SHARED ?= yes

SRCS := \
	src/abs_dynarray.c \
	src/abs_string.c \
	src/abs_hash.c \
	src/abs_hashmap.c \
	src/abs_time.c \
	src/abs_fs.c \
	src/abs_matrix.c \
	src/abs_stats.c \
	src/abs_csv.c \
	src/abs_path.c \
	src/abs_thread.c \
	src/abs_except.c \
	src/abs_regex.c \
	src/abs_datetime.c \
	src/abs_gen.c \
	src/abs_encode.c \
	src/abs_env.c \
	src/abs_server.c \
	src/abs_events.c \
	src/abs_plugins.c \
	src/abs_func.c \
	src/abs_introspect.c \
	src/abs_itertools.c \
	src/abs_sort.c \
	src/abs.c
OBJS := $(patsubst src/%.c,$(OBJDIR)/%.o,$(SRCS))

TESTS    := test_dynarray test_string test_hash test_hashmap test_platform test_abs \
            test_matrix test_stats test_csv test_path test_thread \
            test_regex test_except test_datetime test_gen test_encode test_env \
            test_server test_events test_plugins test_func test_introspect test_itertools \
            test_sort
EXAMPLES := demo py_demo data_demo v6_demo sci_demo lang_demo framework_demo sort_demo

# Windows (MinGW/MSYS) links ws2_32; POSIX links libm and pthreads.
ifeq ($(OS),Windows_NT)
  PLATFORM_LIBS := -lws2_32
  SHLIB := $(BUILDDIR)/abscom.dll
  EXEEXT := .exe
else
  PLATFORM_LIBS := -lm -lpthread
  UNAME_S := $(shell uname -s)
  ifeq ($(UNAME_S),Darwin)
    SHLIB := $(BUILDDIR)/libabscom.dylib
  else
    SHLIB := $(BUILDDIR)/libabscom.so
    PLATFORM_LIBS += -ldl
  endif
  EXEEXT :=
endif

STATIC := $(BUILDDIR)/libabscom.a

TARGETS := $(STATIC)
ifeq ($(SHARED),yes)
  TARGETS += $(SHLIB)
endif

all: $(TARGETS) examples

$(OBJDIR)/%.o: src/%.c
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -DABS_BUILDING_LIBRARY -fPIC -c $< -o $@

$(STATIC): $(OBJS)
	@mkdir -p $(BUILDDIR)
	$(AR) rcs $@ $(OBJS)
	$(RANLIB) $@

ifeq ($(SHARED),yes)
$(SHLIB): $(OBJS)
	@mkdir -p $(BUILDDIR)
ifeq ($(OS),Windows_NT)
	$(CC) -shared -o $(SHLIB) $(OBJS) $(PLATFORM_LIBS) $(LDFLAGS) -Wl,--out-implib,$(BUILDDIR)/libabscom.dll.a
else
	$(CC) -shared -o $(SHLIB) $(OBJS) $(PLATFORM_LIBS) $(LDFLAGS)
endif
endif

examples: $(addprefix $(EXEDIR)/,$(addsuffix $(EXEEXT),$(EXAMPLES)))

$(EXEDIR)/%$(EXEEXT): examples/%.c $(STATIC)
	@mkdir -p $(EXEDIR)
	$(CC) $(CFLAGS) $(LDFLAGS) $< $(STATIC) $(PLATFORM_LIBS) -o $@

tests: $(addprefix $(TESTDIR)/,$(addsuffix $(EXEEXT),$(TESTS)))

$(TESTDIR)/%$(EXEEXT): tests/%.c $(STATIC)
	@mkdir -p $(TESTDIR)
	$(CC) $(CFLAGS) $(LDFLAGS) $< $(STATIC) $(PLATFORM_LIBS) -o $@

test: tests
	@set -e; for t in $(TESTS); do echo "== $$t =="; ./$(TESTDIR)/$$t$(EXEEXT); done

install: all
	@mkdir -p $(DESTDIR)$(PREFIX)/include/abscom \
	           $(DESTDIR)$(PREFIX)/lib/pkgconfig
	@cp include/abscom/*.h $(DESTDIR)$(PREFIX)/include/abscom/
	@cp $(STATIC) $(DESTDIR)$(PREFIX)/lib/
	@if [ "$(SHARED)" = "yes" ]; then cp $(SHLIB) $(DESTDIR)$(PREFIX)/lib/; fi
	@printf 'prefix=%s\nexec_prefix=$${prefix}\nlibdir=$${exec_prefix}/lib\nincludedir=$${prefix}/include\n\nName: abscom\nDescription: A C11 library of reusable data structures and a Python-inspired dynamic runtime\nVersion: %s\nLibs: -L$${libdir} -labscom\nCflags: -I$${includedir}\n' \
		'$(DESTDIR)$(PREFIX)' '$(VERSION)' > $(DESTDIR)$(PREFIX)/lib/pkgconfig/abscom.pc

clean:
	rm -rf $(BUILDDIR)

.PHONY: all examples tests test install clean
