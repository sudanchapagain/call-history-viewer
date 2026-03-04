{ pkgs ? import <nixpkgs> {} }:

pkgs.mkShell {
  buildInputs = [
    pkgs.clang
    pkgs.gnumake
    pkgs.pkg-config
    pkgs.tinyxml-2
  ];
}
