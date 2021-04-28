defmodule Circuits.USB.Nif do
  require Logger

  @on_load {:load_nif, 0}
  @compile {:autoload, false}

  @nif_not_loaded_err "nif not loaded"

  @moduledoc false

  def load_nif do
    nif_binary = Application.app_dir(:circuits_usb, "priv/libusb_nif")

    case :erlang.load_nif(nif_binary, 0) do
      :ok -> :ok
      {:error, {:reload, _}} -> :ok
      {:error, reason} -> Logger.warn("Failed to load nif: #{inspect(reason)}")
    end
  end

  def list_devices, do: :erlang.nif_error(@nif_not_loaded_err)
  def get_handle(_id_vendor, _id_product), do: :erlang.nif_error(@nif_not_loaded_err)
  def release_handle(_handle), do: :erlang.nif_error(@nif_not_loaded_err)

  @spec ctrl_send(handle :: reference, integer, integer, integer, integer, binary, integer) :: any
  def ctrl_send(_handle, _request_type, _request, _value, _index, _data, _timeout),
    do: :erlang.nif_error(@nif_not_loaded_err)

  def ctrl_receive(_handle, _request_type, _request, _value, _index, _length, _timeout),
    do: :erlang.nif_error(@nif_not_loaded_err)

  @spec bulk_send(handle :: reference, integer, binary, integer) :: any
  def bulk_send(_handle, _endpoint, _data, _timeout),
    do: :erlang.nif_error(@nif_not_loaded_err)

  def bulk_receive(_handle, _endpoint, _length, _timeout),
    do: :erlang.nif_error(@nif_not_loaded_err)

  def get_configuration(_handle), do: :erlang.nif_error(@nif_not_loaded_err)

  def set_configuration(_handle, _config), do: :erlang.nif_error(@nif_not_loaded_err)
end

