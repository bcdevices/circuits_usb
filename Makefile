# Variables to override
#
# CC            C compiler
# CROSSCOMPILE	crosscompiler prefix, if any
# CFLAGS	compiler flags for compiling all C files
# ERL_CFLAGS	additional compiler flags for files using Erlang header files
# ERL_EI_INCLUDE_DIR include path to ei.h (Required for crosscompile)
# ERL_EI_LIBDIR path to libei.a (Required for crosscompile)
# LDFLAGS	linker flags for linking all binaries
# ERL_LDFLAGS	additional linker flags for projects referencing Erlang libraries

MIX_APP_PATH ?= .

PREFIX = $(MIX_APP_PATH)/priv
LIBUSB_NIF := $(PREFIX)/libusb_nif.so

DEFAULT_TARGETS ?= $(PREFIX) $(LIBUSB_NIF)

# Look for the EI library and header files
# For crosscompiled builds, ERL_EI_INCLUDE_DIR and ERL_EI_LIBDIR must be
# passed into the Makefile
ifeq ($(ERL_EI_INCLUDE_DIR),)
ERL_ROOT_DIR = $(shell erl -eval "io:format(\"~s~n\", [code:root_dir()])" -s init stop -noshell)
ifeq ($(ERL_ROOT_DIR),)
   $(error Could not find the Erlang installation. Check to see that 'erl' is in your PATH)
endif
ERL_EI_INCLUDE_DIR = "$(ERL_ROOT_DIR)/usr/include"
ERL_EI_LIBDIR = "$(ERL_ROOT_DIR)/usr/lib"
endif

# Set Erlang-specific compile and linker flags
ERL_CFLAGS ?= -I$(ERL_EI_INCLUDE_DIR)
ERL_LDFLAGS ?= -L$(ERL_EI_LIBDIR)

NIF_LDFLAGS := -fPIC -shared -pedantic
NIF_CFLAGS ?= -fPIC -O2 -Wall -Wextra

ifeq ($(CROSSCOMPILE),)
ifeq ($(shell uname),Darwin)
NIF_LDFLAGS += -undefined dynamic_lookup
else
NIF_LDFLAGS += -lusb-1.0
endif
else
NIF_CFLAGS += -I$(PKG_CONFIG_SYSROOT_DIR)/usr/include/
NIF_LDFLAGS += -L$(PKG_CONFIG_SYSROOT_DIR)/usr/lib -lusb-1.0
endif

LIBUSB_NIF_SRC := c_src/libusb_nif.c

calling_from_make:
	mix compile

all: $(DEFAULT_TARGETS)

$(PREFIX):
	mkdir -p $(PREFIX)

$(LIBUSB_NIF):  $(LIBUSB_NIF_SRC) Makefile
	export
	$(CC) -o $@ $(LIBUSB_NIF_SRC) \
	$(LIBUSB_CFLAGS) $(ERL_CFLAGS) $(NIF_CFLAGS) \
	$(ERL_LDFLAGS) $(NIF_LDFLAGS)

$(LIBUSB_BUILD_DIR):
	mkdir -p $(LIBUSB_BUILD_DIR)

clean:
	$(RM) $(LIBUSB_NIF)
	$(RM) c_src/*.o

dir-clean: clean
	$(RM) -rf $(PREFIX)

PHONY: all clean calling_from_make
