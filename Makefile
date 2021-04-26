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
# BUILD_DIR := $(PWD)/_build

PRIV_DIR := priv
LIBUSB_NIF := $(PRIV_DIR)/libusb_nif.so

.PHONY: all clean dir-clean

PRIV_DIR := priv
LIBUSB_NIF_SRC := c_src/libusb_nif.c

NIF_CFLAGS := -O2
NIF_LDFLAGS := -fPIC -shared -pedantic
NIF_CFLAGS ?= -fPIC -O2 -Wall -Wextra -Wno-unused-parameter

ifeq ($(CROSSCOMPILE),)
ifeq ($(shell uname),Darwin)
NIF_LDFLAGS += -undefined dynamic_lookup
endif
endif


.PHONY: all clean libusb-clean dir-clean

all: $(PRIV_DIR) $(LIBUSB_NIF)


$(PRIV_DIR):
	mkdir -p $(PRIV_DIR)

$(LIBUSB_NIF): $(LIBUSB_NIF_SRC) Makefile
	$(CC) -o $@ $(LIBUSB_NIF_SRC) \
	$(LIBUSB_CFLAGS) $(ERL_CFLAGS) $(NIF_CFLAGS) \
	$(ERL_LDFLAGS) $(NIF_LDFLAGS) $(LIBUSB_LDFLAGS)

$(LIBUSB_BUILD_DIR):
	mkdir -p $(LIBUSB_BUILD_DIR)

clean:
	$(RM) $(LIBUSB_NIF)
	$(RM) c_src/*.o

libusb-clean:
	cd $(LIBUSB_SRC_DIR) && make clean

dir-clean: clean libusb-clean
	$(RM) $(LIBUSB_NFF)
