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

  dontStrip = true;

  installPhase = ''
    mkdir -p $out/lib
    cp ./*.dll $out/lib
  '';
}
