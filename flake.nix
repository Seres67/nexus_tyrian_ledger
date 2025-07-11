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
    };
    nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-unstable";
  };

  outputs = {
    flake-utils,
    nixpkgs,
    ...
  } @ inputs:
    flake-utils.lib.eachDefaultSystem (system: let
      pkgs = import inputs.nixpkgs {
        inherit system;
      };

      crossPkgs = import inputs.nixpkgs {
        system = "x86_64-linux";
        crossSystem = {
          config = "x86_64-w64-mingw32";
        };
      };
    in {
      packages.default = crossPkgs.callPackage ./package.nix {
        inherit (inputs) imgui mumble nexus cpr;
      };
      devShells.default = import ./shell.nix {inherit pkgs;};
    });
}
