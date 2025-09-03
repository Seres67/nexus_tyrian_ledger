{
  description = "A Nexus addon to display currencies on an overlay.";
  inputs = {
    flake-compat = {
      url = "github:edolstra/flake-compat";
      flake = false;
    };
    imgui = {
      url = "github:RaidcoreGG/imgui/master";
      flake = false;
    };
    mumble = {
      url = "github:RaidcoreGG/RCGG-lib-mumble-api/main";
      flake = false;
    };
    nexus = {
      url = "github:RaidcoreGG/RCGG-lib-nexus-api/main";
      flake = false;
    };
    cpr = {
      url = "github:libcpr/cpr";
      flake = false;
    };
    flake-utils = {
      url = "github:numtide/flake-utils";
      inputs.nixpkgs.follows = "nixpkgs";
    };
    nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-unstable";
  };

  outputs = {
    flake-utils,
    nixpkgs,
    ...
  } @ inputs:
    flake-utils.lib.eachDefaultSystem (system: let
      pkgs = (import nixpkgs) {
        inherit system;
        crossSystem.config = "x86_64-w64-mingw32";
      };

      nexusTyrianLedger = pkgs.callPackage ./package.nix {
        inherit pkgs;
        inherit (inputs) imgui;
        inherit (inputs) mumble;
        inherit (inputs) nexus;
        inherit (inputs) cpr;
      };
    in {
      inherit pkgs;
      devShell = {
        default = nexusTyrianLedger.devShell;
      };
      packages = {
        inherit nexusTyrianLedger;
        default = nexusTyrianLedger;
      };
    });
}
