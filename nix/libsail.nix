{
  stdenv,
  fetchFromGitHub,

  cmake,

  libavif,
  giflib,
  libjpeg,
  jasper,
  libjxl,
  libpng,
  resvg,
  libtiff,
  libwebp
}:
stdenv.mkDerivation {
  pname = "libsail";
  version = "0.9.5";
  src = fetchFromGitHub {
    owner = "HappySeaFox";
    repo = "sail";
    rev = "v0.9.5";
    hash = "sha256-yzHFjYQVjuEYb0LF00xjcTzEpDaeEogyxJVf/kTTWec=";
  };

  nativeBuildInputs = [ cmake ];

  buildInputs = [
    libavif
    giflib
    libjpeg
    jasper
    libjxl
    libpng
    resvg
    libtiff
    libwebp
  ];
}
