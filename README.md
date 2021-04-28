# Elixir Circuits - USB

`Circuits.USB` lets you communicate with hardware devices using USB protocols.

# _This is a work in progress_

## Geting Started

If [available in Hex](https://hex.pm/docs/publish), the package can be installed
by adding `circuits_usb` to your list of dependencies in `mix.exs`:

```elixir
def deps do
  [{:circuits_usb, "~> 0.1.0"}]
end
```
## FAQ

### Does this require a custom Nerves System?

Currently, Libusb isn't enabled in Nerves Projects by default but may be added
since it is a small libray.

