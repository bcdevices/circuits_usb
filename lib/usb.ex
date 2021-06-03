defmodule Circuits.USB do
  require Logger


  alias Circuits.USB.Nif

  defdelegate list_devices, to: Nif

  defdelegate get_handle(id_vendor, id_product, interface_num \\ 0), to: Nif
  defdelegate release_handle(handle), to: Nif

  defdelegate ctrl_send(handle, request_type, request, value, index, data, timeout),
      to: Nif

  defdelegate ctrl_receive(handle, request_type, request, value, index, length, timeout),
    to: Nif

  defdelegate bulk_send(handle, endpoint, data, timeout),
    to: Nif

  defdelegate bulk_receive(handle, endpoint, length, timeout),
    to: Nif

  defdelegate get_configuration(handle), to: Nif

  defdelegate set_configuration(handle, config), to: Nif

end
