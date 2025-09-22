{
  cmake,
  openssl,
  stdenv,
}:
stdenv.mkDerivation {
  pname = "nexus_tyrian_ledger";
  version = "0.2.0.0";
  src = ./.;

  nativeBuildInputs = [
    cmake
  ];

  buildInputs = [
    openssl
  ];

  installPhase = ''
    x86_64-w64-mingw32-strip ./*.dll
    mkdir -p $out/lib
    cp ./*.dll $out/lib
  '';
}
