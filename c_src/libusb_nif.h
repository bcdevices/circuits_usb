#ifndef LIBUSB_NIF_H

#include <erl_nif.h>
#include <libusb-1.0/libusb.h>

#define MAX_BUF_LENGTH 255

typedef struct {
  struct libusb_device_handle *handle;
  struct libusb_device *dev;
} ResourceData;

typedef struct {
  ERL_NIF_TERM atom_ok;
  ERL_NIF_TERM atom_undefined;
  ERL_NIF_TERM atom_error;
  ERL_NIF_TERM atom_nil;
  ERL_NIF_TERM atom_libusb_init_fail;
  ERL_NIF_TERM atom_libusb_dev_list_fail;
  ERL_NIF_TERM atom_device_not_found;
  ERL_NIF_TERM atom_kernel_driver_detach_fail;
  ERL_NIF_TERM atom_claim_interface_fail;
} PrivData;

static int load(ErlNifEnv* env, void** priv, ERL_NIF_TERM info);
static int reload(ErlNifEnv* env, void** priv, ERL_NIF_TERM info);
static int upgrade(ErlNifEnv* env, void** priv, void** old_priv, ERL_NIF_TERM info);
static void unload(ErlNifEnv* env, void* priv);

static void libusb_rt_dtor(ErlNifEnv *env, void *obj);

static ErlNifResourceType *libusb_rt;

static ERL_NIF_TERM build_dev_list(ErlNifEnv *env, libusb_device *dev);
static ERL_NIF_TERM list_devices(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[]);

static ERL_NIF_TERM get_handle(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[]);
static ERL_NIF_TERM release_handle(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[]);

static ERL_NIF_TERM ctrl_send(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[]);
static ERL_NIF_TERM ctrl_receive(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[]);

static ERL_NIF_TERM bulk_send(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[]);
static ERL_NIF_TERM bulk_receive(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[]);

// Helpers.
static int map_put(ErlNifEnv *env, ERL_NIF_TERM map_in, ERL_NIF_TERM* map_out, ERL_NIF_TERM key, ERL_NIF_TERM value);
static ERL_NIF_TERM get_libusb_error(ErlNifEnv *env, int error);

static ERL_NIF_TERM set_configuration(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[]);
static ERL_NIF_TERM get_configuration(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[]);

static ErlNifFunc nif_funcs[] = {
    {"list_devices", 0, list_devices, ERL_NIF_DIRTY_JOB_IO_BOUND},
    {"get_handle", 3, get_handle, ERL_NIF_DIRTY_JOB_IO_BOUND},
    {"release_handle", 1, release_handle, ERL_NIF_DIRTY_JOB_IO_BOUND},
    {"ctrl_send", 7, ctrl_send, ERL_NIF_DIRTY_JOB_IO_BOUND},
    {"ctrl_receive", 7, ctrl_receive, ERL_NIF_DIRTY_JOB_IO_BOUND},
    {"bulk_send", 4, bulk_send, ERL_NIF_DIRTY_JOB_IO_BOUND},
    {"bulk_receive", 4, bulk_receive, ERL_NIF_DIRTY_JOB_IO_BOUND},

    {"set_configuration", 2, set_configuration, ERL_NIF_DIRTY_JOB_IO_BOUND},
    {"get_configuration", 1, get_configuration, ERL_NIF_DIRTY_JOB_IO_BOUND}
};

ERL_NIF_INIT(Elixir.Circuits.USB.Nif, nif_funcs,  &load, &reload, &upgrade, &unload)

#endif
