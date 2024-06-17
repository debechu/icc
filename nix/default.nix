let
  pkgs = import <nixpkgs> { };
in
rec {
  libsail = pkgs.callPackage ./libsail.nix { };
  icc = pkgs.callPackage ./icc.nix { inherit libsail; };
}
