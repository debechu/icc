# ICC (Image Color Clustering)
ICC is a command line program written in C to determine the dominant colors of an image using the DBSCAN algorithm.

## Building

### Nix / NixOS
Simply run `nix-build nix`. Dependencies should be installed automatically.

`icc` should be created in `result/bin`.

### Other platforms
Dependencies:
- [sail v0.9.5](https://github.com/HappySeaFox/sail)

Set `SAIL_INCLUDE_DIR` to the sail's include directory:
> `SAIL_INCLUDE_DIR=<PATH/TO/SAIL/INCLUDE>`

Run `make`.

`icc` and `libicc.a` should be created in the `bin` directory.
