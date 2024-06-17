{
  stdenv,
  lib,
  makeWrapper,

  libsail
}:
stdenv.mkDerivation {
  pname = "icc";
  version = "0.0.0";
  src = lib.cleanSourceWith {
    filter = name: type:
      !(lib.hasSuffix ".nix" (builtins.baseNameOf name));
    src = lib.cleanSource ../.;
  };

  nativeBuildInputs = [ makeWrapper ];

  buildInputs = [ libsail ];

  installPhase = ''
    runHook preInstall

    $DRY_RUN_CMD mkdir -p "$out/bin"
    $DRY_RUN_CMD cp "bin/icc" "$out/bin/icc"

    runHook postInstall
  '';

  postFixup = ''
    wrapProgram "$out/bin/icc" \
      --set SAIL_CODECS_PATH "${libsail}/lib/sail/codecs"
  '';

  SAIL_INCLUDE_DIR = "${libsail}/include/sail";
}
