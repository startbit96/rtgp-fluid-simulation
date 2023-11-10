{
  description = "RTGP Fluid Simulation";
  inputs.nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-unstable";
  inputs.flake-utils.url = "github:numtide/flake-utils";

  outputs = { self, nixpkgs, flake-utils, ... }:
    flake-utils.lib.eachDefaultSystem (system: let
      pkgs = nixpkgs.legacyPackages.${system};
      # Darwin specifig package.
      darwinPackages = if system == "aarch64-darwin" then [
        pkgs.darwin.apple_sdk.frameworks.ApplicationServices
      ] else [];
    in {
      devShells.default = pkgs.mkShell {
        packages = with pkgs; [ 
          cmake
          glfw
          glew
          glm 
        ] ++ darwinPackages;
      };
    });
}
