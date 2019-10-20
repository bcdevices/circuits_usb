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

# NIF_CFLAGS := -O2 -flat_namespace -undefined suppress
# NIF_LDFLAGS := -fPIC -shared -pedantic
NIF_CFLAGS ?= -fPIC -O2 -Wall -Wextra -Wno-unused-parameter

ifeq ($(CROSSCOMPILE),)
ifeq ($(shell uname),Darwin)
LDFLAGS += -undefined dynamic_lookup
endif
endif

# BUILD_DIR := $(PWD)/_build

PRIV_DIR := priv
LIBUSB_NIF := $(PRIV_DIR)/libusb_nif.so

.PHONY: all clean dir-clean

# MIX_BUILD_PATH := $(PWD)/_build
LIBUSB_VERSION := 1.0.22
LIBUSB_SRC_DIR := $(MIX_BUILD_PATH)/libusb-$(LIBUSB_VERSION)
LIBUSB_BUILD_DIR := $(MIX_BUILD_PATH)/libusb
LIBUSB_INCLUDE_DIR := $(LIBUSB_BUILD_DIR)/include
LIBUSB_LIBDIR := $(LIBUSB_BUILD_DIR)/lib
LIBUSB_LIB := $(LIBUSB_LIBDIR)/libusb-1.0.so

PRIV_DIR := priv
LIBUSB_CFLAGS := -I$(LIBUSB_INCLUDE_DIR)
LIBUSB_LDFLAGS := -L$(LIBUSB_LIBDIR) -Wl,-rpath=. -lusb-1.0
LIBUSB_NIF_SRC := c_src/libusb_nif.c

NIF_CFLAGS := -O2
NIF_LDFLAGS := -fPIC -shared -pedantic

LIBUSB_DL := libusb-$(LIBUSB_VERSION).tar.bz2
LIBUSB_DL_URL := "https://iweb.dl.sourceforge.net/project/libusb/libusb-1.0/libusb-$(LIBUSB_VERSION)/$(LIBUSB_DL)"

.PHONY: all clean libusb-clean dir-clean

all: $(PRIV_DIR) $(LIBUSB_SRC_DIR) $(LIBUSB_NIF)

$(LIBUSB_SRC_DIR):
	cd $(MIX_BUILD_PATH) && wget $(LIBUSB_DL_URL) && tar xf $(LIBUSB_DL)

$(PRIV_DIR):
	mkdir -p $(PRIV_DIR)

# $(LIBUSB_NIF): c_src/libusb_nif.c
# 	echo haai
# 	$(CC) $(ERL_CFLAGS) $(CFLAGS) $(ERL_LDFLAGS) $(LDFLAGS) -o $@ $<

$(LIBUSB_NIF): $(LIBUSB_LIB) $(LIBUSB_NIF_SRC) Makefile
	$(CC) -o $@ $(LIBUSB_NIF_SRC) \
	$(LIBUSB_CFLAGS) $(ERL_CFLAGS) $(NIF_CFLAGS) \
	$(ERL_LDFLAGS) $(NIF_LDFLAGS) $(LIBUSB_LDFLAGS)

$(LIBUSB_BUILD_DIR):
	mkdir -p $(LIBUSB_BUILD_DIR)

$(LIBUSB_SRC_DIR)/config.status: $(LIBUSB_BUILD_DIR)
	cd $(LIBUSB_SRC_DIR) && ./configure --prefix=$(LIBUSB_BUILD_DIR) --host=$(MAKE_HOST) --build=$(BUILD) --disable-udev

$(LIBUSB_LIB): $(LIBUSB_SRC_DIR)/config.status
	cd $(LIBUSB_SRC_DIR) && make && make install

clean:
	$(RM) $(LIBUSB_NIF)
	$(RM) c_src/*.o

libusb-clean:
	cd $(LIBUSB_SRC_DIR) && make clean

dir-clean: clean libusb-clean
	$(RM) $(LIBUSB_NOF)
