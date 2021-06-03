#include <erl_nif.h>
#include <libusb-1.0/libusb.h>

#include <stdio.h>
#include <string.h>

#include "libusb_nif.h"

// Allocation:
// - ly10-pltrunner: iface#0, EP: 0x81/0x01
// - ly11_plt_ex: iface#1, EP: 0x82/0x02
// - ly11_plt_pkcs: iface#2, EP: 0x83/0x03
#define USB_INTERFACE_NUM 2

static ERL_NIF_TERM build_dev_list(ErlNifEnv *env, libusb_device *dev) {
    struct libusb_device_descriptor desc;
    libusb_device_handle *handle = NULL;

    char string[MAX_BUF_LENGTH];

    int ret;
    uint8_t i;

    ERL_NIF_TERM dev_map = enif_make_new_map(env);
    ErlNifBinary bin;

    ret = libusb_get_device_descriptor(dev, &desc);
    if (ret < 0) {
        enif_fprintf(stderr, "failed to get device descriptor");
    }

    ret = libusb_open(dev, &handle);
    if (ret == LIBUSB_SUCCESS) {

        if (desc.iManufacturer) {
            ret = libusb_get_string_descriptor_ascii(handle, desc.iManufacturer, (unsigned char *)string, sizeof(string));
            if (ret > 0) {
                enif_alloc_binary(strlen(string), &bin);
                memcpy(bin.data, string, bin.size);
                map_put(env, dev_map, &dev_map, enif_make_atom(env, "iManufacturer"), enif_make_binary(env, &bin));
            }
        }

        if (desc.iProduct) {
            ret = libusb_get_string_descriptor_ascii(handle, desc.iProduct, (unsigned char *)string, sizeof(string));
            if (ret > 0) {
                enif_alloc_binary(strlen(string), &bin);
                memcpy(bin.data, string, bin.size);
                map_put(env, dev_map, &dev_map, enif_make_atom(env, "iProduct"), enif_make_binary(env, &bin));
            }
        }

    }

    map_put(env, dev_map, &dev_map, enif_make_atom(env, "bus_number"), enif_make_int(env, libusb_get_bus_number(dev)));
    map_put(env, dev_map, &dev_map, enif_make_atom(env, "device_address"), enif_make_int(env, libusb_get_device_address(dev)));
    map_put(env, dev_map, &dev_map, enif_make_atom(env, "idProduct"), enif_make_int(env, desc.idProduct));
    map_put(env, dev_map, &dev_map, enif_make_atom(env, "idVendor"), enif_make_int(env, desc.idVendor));

    if (handle) {
        if (desc.iSerialNumber) {
            ret = libusb_get_string_descriptor_ascii(handle, desc.iSerialNumber, (unsigned char *)string, sizeof(string));
            if (ret > 0) {
                enif_alloc_binary(strlen(string), &bin);
                memcpy(bin.data, string, bin.size);
                map_put(env, dev_map, &dev_map, enif_make_atom(env, "iSerialNumber"), enif_make_binary(env, &bin));
            }
        }
    }

    if (handle)
        libusb_close(handle);

    enif_release_binary(&bin);
    return dev_map;
}

static ERL_NIF_TERM list_devices(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[]) {
    libusb_device **devs;
    ssize_t cnt;
    int r, i;

    r = libusb_init(NULL);
    if (r < 0)
        return enif_make_badarg(env);

    cnt = libusb_get_device_list(NULL, &devs);
    if (cnt < 0) {
        return enif_make_badarg(env);
    }

    ERL_NIF_TERM ret_arr[cnt];

    enif_fprintf(stderr, "Starting list.\r\n");
    for(i = 0; devs[i]; ++i) {
        ret_arr[i] = build_dev_list(env, devs[i]);
    }

    libusb_free_device_list(devs, 1);

    libusb_exit(NULL);

    enif_fprintf(stderr, "Done.\r\n");

    return enif_make_list_from_array(env, ret_arr, cnt);
}

static void libusb_rt_dtor(ErlNifEnv *env, void *obj)
{
    enif_fprintf(stderr, "libusb_rt_dtor called\r\n");

    ResourceData *resource_data = (ResourceData *)obj;
    if (resource_data->handle) {
        libusb_release_interface(resource_data->handle, 0);
    }
}

static ERL_NIF_TERM get_handle(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[]) {
    PrivData* priv = enif_priv_data(env);
    ResourceData *resource_data;
    ERL_NIF_TERM res;

    struct libusb_device **devs;
    struct libusb_device_handle *handle = NULL, *hDevice_expected = NULL;
    struct libusb_device *dev,*dev_expected;
    struct libusb_device_descriptor desc;
    struct libusb_endpoint_descriptor *epdesc;
    struct libusb_interface_descriptor *intdesc;

    int i, r, e, found, id_vendor, id_product;
    found = 0;

    // Get args from function call.
    r = enif_get_int(env, argv[0], &id_vendor);
    if (!r)
        return enif_make_badarg(env);

    r = enif_get_int(env, argv[1], &id_product);
    if (!r)
        return enif_make_badarg(env);

    // Initialize libusb.
    r = libusb_init(NULL);
    if (r < 0)
        return enif_make_tuple2(env, priv->atom_error,  get_libusb_error(env, r));

    int cnt = libusb_get_device_list(NULL, &devs);
    if (cnt < 0)
        return enif_make_tuple2(env, priv->atom_error,  get_libusb_error(env, r));

    while ((dev = devs[i++]) != NULL) {
        r = libusb_get_device_descriptor(dev, &desc);
        if (r < 0)
        {
            libusb_free_device_list(devs, 1);
            return enif_make_tuple2(env, priv->atom_error, get_libusb_error(env, r));
        }

        if(desc.idVendor == id_vendor && desc.idProduct == id_product) {
            found = 1;
            break;
        }
    }

    if (found == 0) {
        libusb_free_device_list(devs, 1);
        return enif_make_tuple2(env, priv->atom_error, enif_make_atom(env, "device_not_found"));
    }

    e = libusb_open(dev, &handle);
    if (e < 0)
    {
        libusb_free_device_list(devs, 1);
        return enif_make_tuple2(env, priv->atom_error, get_libusb_error(env, e));
    }

    dev_expected = dev;
    hDevice_expected = handle;

    if(libusb_kernel_driver_active(handle, 0) == 1)
    {
        enif_fprintf(stderr, "Kernel Driver Active, trying to detach.\r\n");
        e = libusb_detach_kernel_driver(handle, 0);
        if(e != 0) {
            enif_fprintf(stderr, "Couldn't detach kernel driver!\r\n");
            libusb_free_device_list(devs, 1);
            libusb_close(handle);
            return enif_make_tuple2(env, priv->atom_error, get_libusb_error(env, e));
        }
    }

    e = libusb_claim_interface(handle, USB_INTERFACE_NUM);
    if(e < 0)
    {
        enif_fprintf(stderr, "Cannot Claim Interface\r\n");
        libusb_free_device_list(devs, 1);
        libusb_close(handle);
        return enif_make_tuple2(env, priv->atom_error, get_libusb_error(env, e));
    }

    resource_data = enif_alloc_resource(libusb_rt, sizeof(ResourceData));

    resource_data->handle = hDevice_expected;
    resource_data->dev = dev_expected;

    res = enif_make_resource(env, resource_data);
    enif_release_resource(resource_data);
    return enif_make_tuple2(env, priv->atom_ok, res);
}

static ERL_NIF_TERM release_handle(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[]) {
    ResourceData *resource_data;
    PrivData* priv = enif_priv_data(env);
    int e;

    if(!enif_get_resource(env, argv[0], libusb_rt, (void **)&resource_data)) {
        return enif_make_badarg(env);
    }

    e = libusb_release_interface(resource_data->handle, 0);
    libusb_close(resource_data->handle);
    resource_data->handle = 0;
    libusb_exit(NULL);
    return priv->atom_ok;
}

static ERL_NIF_TERM bulk_send(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[]) {
    ResourceData *resource_data;
    PrivData* priv = enif_priv_data(env);
    ErlNifBinary bin;

    int acc;

    uint8_t endpoint;
    int actual;
    unsigned int timeout;

    if(!enif_get_resource(env, argv[0], libusb_rt, (void **)&resource_data)) {
        return enif_make_badarg(env);
    }

    if(!enif_get_int(env, argv[1], &acc)) return enif_make_badarg(env);
    endpoint = (uint8_t)acc;

    if(!enif_get_int(env, argv[3], &acc)) return enif_make_badarg(env);
    timeout = (unsigned int)acc;

    if(!enif_inspect_binary(env, argv[2], &bin)) return enif_make_badarg(env);

    acc = libusb_bulk_transfer(resource_data->handle, endpoint, bin.data, bin.size, &actual, timeout);
    if (acc < 0 || actual != bin.size) {
        return enif_make_tuple2(env, priv->atom_error, get_libusb_error(env, acc));
    } else {
        return enif_make_tuple2(env, priv->atom_ok, enif_make_int(env, acc));
    }
}

static ERL_NIF_TERM bulk_receive(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[]) {
    ResourceData *resource_data;
    PrivData* priv = enif_priv_data(env);
    ErlNifBinary bin;

    int acc;

    uint8_t endpoint;
    int length;
    unsigned char * data;
    int actual;
    unsigned int timeout;

    if(!enif_get_resource(env, argv[0], libusb_rt, (void **)&resource_data)) {
        return enif_make_badarg(env);
    }

    if(!enif_get_int(env, argv[1], &acc)) return enif_make_badarg(env);
    endpoint = (uint8_t)acc;

    if(!enif_get_int(env, argv[2], &acc)) return enif_make_badarg(env);
    length = (int)acc;

    if(!enif_get_int(env, argv[3], &acc)) return enif_make_badarg(env);
    timeout = (unsigned int)acc;

    data = (unsigned char *) malloc(length);

    acc = libusb_bulk_transfer(resource_data->handle, endpoint | LIBUSB_ENDPOINT_IN, data, length, &actual, timeout);
    if (acc < 0) {
        free(data);
        return enif_make_tuple2(env, priv->atom_error, get_libusb_error(env, acc));
    } else {
        if(!enif_alloc_binary(actual, &bin))
            return enif_make_tuple2(env, priv->atom_error, enif_make_atom(env, "alloc_failed"));

        memcpy(bin.data, data, actual);
        free(data);
        ERL_NIF_TERM ret = enif_make_tuple2(env, priv->atom_ok, enif_make_binary(env, &bin));
        enif_release_binary(&bin);
        return ret;
    }
}

static ERL_NIF_TERM ctrl_send(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[]) {
    ResourceData *resource_data;
    PrivData* priv = enif_priv_data(env);
    ErlNifBinary bin;

    int acc;

    uint8_t 	bmRequestType, bRequest;
    uint16_t 	wValue, wIndex, wLength;
    unsigned char * data;
    unsigned int 	timeout;

    if(!enif_get_resource(env, argv[0], libusb_rt, (void **)&resource_data)) {
        return enif_make_badarg(env);
    }

    if(!enif_get_int(env, argv[1], &acc)) return enif_make_badarg(env);
    bmRequestType = (uint8_t)acc;

    if(!enif_get_int(env, argv[2], &acc)) return enif_make_badarg(env);
    bRequest = (uint8_t)acc;

    if(!enif_get_int(env, argv[3], &acc)) return enif_make_badarg(env);
    wValue = (uint16_t)acc;

    if(!enif_get_int(env, argv[4], &acc)) return enif_make_badarg(env);
    wIndex = (uint16_t)acc;

    if(!enif_get_int(env, argv[6], &acc)) return enif_make_badarg(env);
    timeout = (unsigned int)acc;

    if(!enif_inspect_binary(env, argv[5], &bin)) return enif_make_badarg(env);

    acc = libusb_control_transfer(resource_data->handle, bmRequestType, bRequest, wValue, wIndex, bin.data, bin.size, timeout);
    if (acc < 0) {
        return enif_make_tuple2(env, priv->atom_error, get_libusb_error(env, acc));
    } else {
        return enif_make_tuple2(env, priv->atom_ok, enif_make_int(env, acc));
    }
}

static ERL_NIF_TERM ctrl_receive(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[]) {
    ResourceData *resource_data;
    PrivData* priv = enif_priv_data(env);
    ErlNifBinary bin;

    int acc;

    uint8_t 	bmRequestType, bRequest;
    uint16_t 	wValue, wIndex, wLength;
    unsigned char * data;
    unsigned int 	timeout;

    if(!enif_get_resource(env, argv[0], libusb_rt, (void **)&resource_data)) {
        return enif_make_badarg(env);
    }

    if(!enif_get_int(env, argv[1], &acc)) return enif_make_badarg(env);
    bmRequestType = (uint8_t)acc;

    if(!enif_get_int(env, argv[2], &acc)) return enif_make_badarg(env);
    bRequest = (uint8_t)acc;

    if(!enif_get_int(env, argv[3], &acc)) return enif_make_badarg(env);
    wValue = (uint16_t)acc;

    if(!enif_get_int(env, argv[4], &acc)) return enif_make_badarg(env);
    wIndex = (uint16_t)acc;

    if(!enif_get_int(env, argv[5], &acc)) return enif_make_badarg(env);
    wLength = (uint16_t)acc;

    if(!enif_get_int(env, argv[6], &acc)) return enif_make_badarg(env);
    timeout = (unsigned int)acc;


    acc = libusb_control_transfer(resource_data->handle, bmRequestType, bRequest, wValue, wIndex, bin.data, bin.size, timeout);
    if (acc < 0) {
        return enif_make_tuple2(env, priv->atom_error, get_libusb_error(env, acc));
    } else {
        if(!enif_alloc_binary(acc, &bin))
            return enif_make_tuple2(env, priv->atom_error, enif_make_atom(env, "alloc_failed"));

        memcpy(bin.data, data, acc);
        ERL_NIF_TERM ret = enif_make_tuple2(env, priv->atom_ok, enif_make_binary(env, &bin));
        enif_release_binary(&bin);
        return ret;
    }
}

static ERL_NIF_TERM set_configuration(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[]) {
    ResourceData *resource_data;
    PrivData* priv = enif_priv_data(env);
    int config;

    if(!enif_get_resource(env, argv[0], libusb_rt, (void **)&resource_data)) {
        return enif_make_badarg(env);
    }

    if(!enif_get_int(env, argv[1], &config)) {
        return enif_make_badarg(env);
    }

    libusb_set_configuration(resource_data->handle, config);
    return priv->atom_ok;
}

static ERL_NIF_TERM get_configuration(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[]) {
    ResourceData *resource_data;
    PrivData* priv = enif_priv_data(env);
    int config;

    if(!enif_get_resource(env, argv[0], libusb_rt, (void **)&resource_data)) {
        return enif_make_badarg(env);
    }

    libusb_get_configuration(resource_data->handle, &config);

    return enif_make_tuple2(env, priv->atom_ok, enif_make_int(env, config));
}

static int load(ErlNifEnv* env, void** priv, ERL_NIF_TERM info) {
    PrivData* data = enif_alloc(sizeof(PrivData));

    if (data == NULL) {
        return 1;
    }

    data->atom_ok = enif_make_atom(env, "ok");
    data->atom_undefined = enif_make_atom(env, "undefined");
    data->atom_error = enif_make_atom(env, "error");
    data->atom_nil = enif_make_atom(env, "nil");
    *priv = (void*) data;

    libusb_rt = enif_open_resource_type(env, "Elixir.Circuits.USB", "libusb_resource", libusb_rt_dtor, ERL_NIF_RT_CREATE, NULL);

    return !libusb_rt;
}

static int reload(ErlNifEnv* env, void** priv, ERL_NIF_TERM info)
{
    return load(env, priv, info);
}

static int upgrade(ErlNifEnv* env, void** priv, void** old_priv, ERL_NIF_TERM info)
{
    return load(env, priv, info);
}

static void unload(ErlNifEnv* env, void* priv)
{
    enif_free(priv);
}

static int map_put(ErlNifEnv *env, ERL_NIF_TERM map_in, ERL_NIF_TERM* map_out, ERL_NIF_TERM key, ERL_NIF_TERM value)
{
    return enif_make_map_put(env, map_in, key, value, map_out);
}

static ERL_NIF_TERM get_libusb_error(ErlNifEnv *env, int error) {
    const char * error_str = libusb_error_name(error);
    return enif_make_atom(env, error_str);
}
