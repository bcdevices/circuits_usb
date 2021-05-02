defmodule LibUsb.MixProject do
  use Mix.Project

  def project do
    [
      app: :libusb,
      version: "0.1.0",
      elixir: "~> 1.9",
      compilers: [:elixir_make] ++ Mix.compilers(),
      make_clean: ["mix_clean"],
      make_targets: ["all"],
      package: package(),
      description: description(),
      start_permanent: Mix.env() == :prod,
      deps: deps(),
      aliases: [format: [&format_c/1, "format"]]
      # dialyzer: [plt_add_apps: [:iex]]
    ]
  end

  # Run "mix help compile.app" to learn about applications.
  def application do
    [
      extra_applications: [:logger]
    ]
  end

  # Run "mix help deps" to learn about dependencies.
  defp deps do
    [
      {:elixir_make, "~> 0.6.2", runtime: false},
      {:dialyxir, "~> 0.5.1", only: :dev, runtime: false},
      {:ex_doc, "~> 0.18.1", only: [:dev, :test]}
    ]
  end

  defp format_c([]) do
    astyle =
      System.find_executable("astyle") ||
        Mix.raise("""
        Could not format C code since astyle is not available.
        """)

    System.cmd(astyle, ["-n", "c_src/*.c", "src/*.h"], into: IO.stream(:stdio, :line))
  end

  defp format_c(_args), do: true

  defp package do
    [
      licenses: ["MIT", "GPL"],
      maintainers: ["konnorrigby@gmail.com"],
      links: %{
        "GitHub" => "https://github.com/connorrigby/libusb"
      },
      files: ["lib", "mix.exs", "README*", "LICENSE*", "c_src", "Makefile"],
      source_url: "https://github.com/connorrigby/libusb"
    ]
  end

  defp description do
    """
    Simple Elixir LibUSB Wrapper.
    """
  end
end
