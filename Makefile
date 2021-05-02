# Makefile for building the NIF
#
# Makefile targets:
#
# all           build and install the NIF
# mix_clean     clean build products and intermediates
#
# Variables to override:
#
# MIX_APP_PATH  path to the build directory
#
# CC            C compiler
# CROSSCOMPILE	crosscompiler prefix, if any
# CFLAGS	compiler flags for compiling all C files
# ERL_CFLAGS	additional compiler flags for files using Erlang header files
# ERL_EI_INCLUDE_DIR include path to ei.h (Required for crosscompile)
# ERL_EI_LIBDIR path to libei.a (Required for crosscompile)
# LDFLAGS	linker flags for linking all binaries
# ERL_LDFLAGS	additional linker flags for projects referencing Erlang libraries

LIBUSB_VERSION := 1.0.22

PREFIX = $(MIX_APP_PATH)/priv
BUILD  = $(MIX_APP_PATH)/obj

LIBUSB_NIF := $(PREFIX)/libusb_nif.so

LIBUSB_SRC_DIR = $(MIX_APP_PATH)/libusb-$(LIBUSB_VERSION)
LIBUSB_BUILD_DIR = $(MIX_APP_PATH)/libusb
LIBUSB_INCLUDE_DIR = $(LIBUSB_BUILD_DIR)/include
LIBUSB_LIBDIR = $(LIBUSB_BUILD_DIR)/lib
LIBUSB_LIB = $(LIBUSB_LIBDIR)/libusb-1.0.a

LIBUSB_CFLAGS = -I$(LIBUSB_INCLUDE_DIR)
LIBUSB_LDFLAGS = -L$(LIBUSB_LIBDIR) -lusb-1.0
LIBUSB_NIF_SRC = c_src/libusb_nif.c

NIF_CFLAGS = -fPIC -O2 -Wall -Wextra -Wno-unused-parameter
NIF_LDFLAGS = -fPIC -shared -pedantic

ifeq ($(CROSSCOMPILE),)
    # Not crosscompiling
    ifeq ($(shell uname),Darwin)
        NIF_LDFLAGS += -undefined dynamic_lookup
    endif
else
    # Crosscompiled build
    # NOTE: Use REBAR_TARGET_ARCH out of convenience.
    CONFIGURE_OPTS = --target=$(REBAR_TARGET_ARCH) --host=$(REBAR_TARGET_ARCH) --disable-udev
endif

# Set Erlang-specific compile and linker flags
ERL_CFLAGS ?= -I$(ERL_EI_INCLUDE_DIR)
ERL_LDFLAGS ?= -L$(ERL_EI_LIBDIR)

LIBUSB_TAR_BZ2 := libusb-$(LIBUSB_VERSION).tar.bz2
LIBUSB_DL_URL := "https://iweb.dl.sourceforge.net/project/libusb/libusb-1.0/libusb-$(LIBUSB_VERSION)/$(LIBUSB_TAR_BZ2)"
LIBUSB_DL := $(MIX_APP_PATH)/$(LIBUSB_TAR_BZ2)

calling_from_make:
	mix compile

all: $(PREFIX) $(BUILD) $(LIBUSB_SRC_DIR) $(LIBUSB_NIF)

$(LIBUSB_DL):
	cd $(MIX_APP_PATH) && wget $(LIBUSB_DL_URL)

$(LIBUSB_SRC_DIR): $(LIBUSB_DL)
	cd $(MIX_APP_PATH) && tar xf $(LIBUSB_DL)

$(LIBUSB_NIF): $(LIBUSB_LIB) $(LIBUSB_NIF_SRC) Makefile
	$(CC) -o $@ $(LIBUSB_NIF_SRC) \
	  $(CFLAGS) $(LIBUSB_CFLAGS) $(ERL_CFLAGS) $(NIF_CFLAGS) \
	  $(LDFLAGS) $(ERL_LDFLAGS) $(NIF_LDFLAGS) $(LIBUSB_LDFLAGS)

$(LIBUSB_SRC_DIR)/config.status: $(LIBUSB_BUILD_DIR)
	cd $(LIBUSB_SRC_DIR) && ./configure --prefix=$(LIBUSB_BUILD_DIR) $(CONFIGURE_OPTS)

$(LIBUSB_LIB): $(LIBUSB_SRC_DIR)/config.status
	$(MAKE) -C $(LIBUSB_SRC_DIR) all install

$(PREFIX) $(BUILD) $(LIBUSB_BUILD_DIR):
	mkdir -p $@

mix_clean:
	$(RM) $(LIBUSB_NIF)
	$(MAKE) -C $(LIBUSB_SRC_DIR) clean

.PHONY: all mix_clean
