{
  clang13Stdenv,
  cmake,
  glew,
  glfw3,
  glm,
  darwin,
  lib,
}:
clang13Stdenv.mkDerivation {
  name = "rtgp-fluid-simulation";
  version = "0.0.1";

  src = lib.cleanSource ../..;

  buildInputs =
    [
      cmake
      glew
      glfw3
      glm
    ]
    ++ (lib.optional (clang13Stdenv.isDarwin)
      darwin.apple_sdk.frameworks.ApplicationServices);

  installPhase = ''
    mkdir -p $out/bin
    cp rtgp_fluid_simulation $out/bin/
    cp -r ../shaders $out/shaders
  '';
}
