with import <nixpkgs> {}; rec {
  ebispEnv = stdenv.mkDerivation {
    name = "ebisp-env";
    buildInput = [stdenv gcc gdb];
  };
}
