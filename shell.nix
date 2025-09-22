{
  mkShell,
  cmake,
  clang-tools,
  bintools,
  openssl,
}:
mkShell {
  nativeBuildInputs = [
    cmake
    clang-tools
    bintools
  ];

  buildInputs = [
    openssl
  ];
}
