{
  stdenv,
  windows,
  pkg-config,
  cmake,
  imgui,
  nexus,
  mumble,
  cpr,
  pkgs,
}:
stdenv.mkDerivation {
  src = ./.;
  pname = "nexus_tyrian_ledger";
  version = "0.1.0.1";

  patchPhase = ''
    mkdir -p ./modules/
    cp --no-preserve=mode -r ${imgui.outPath} ./modules/imgui
    cp --no-preserve=mode -r ${nexus.outPath} ./modules/nexus
    cp --no-preserve=mode -r ${mumble.outPath} ./modules/mumble
    cp --no-preserve=mode -r ${cpr.outPath} ./modules/cpr
  '';

  installPhase = ''
    mkdir -p $out/lib
    cp ./*.dll $out/lib
  '';

  nativeBuildInputs = [
    cmake
    pkg-config
  ];

  buildInputs = [
    windows.pthreads
    pkgs.curl
    pkgs.nlohmann_json
  ];
}
