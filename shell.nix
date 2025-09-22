{
  mkShell,
  cmake,
  clang-tools,
  bintools,
  windows,
  openssl,
}:
mkShell {
  nativeBuildInputs = [
    cmake
    clang-tools
    bintools
  ];

  buildInputs = [
    windows.pthreads
    openssl
  ];
}
