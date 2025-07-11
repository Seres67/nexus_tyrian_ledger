{pkgs}:
pkgs.mkShell {
  name = "nexus_tyrian_ledger";
  packages = with pkgs; [
    llvmPackages_20.clang
    cmake
    bear
  ];
}
