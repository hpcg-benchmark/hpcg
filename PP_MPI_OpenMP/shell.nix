with import <nixpkgs> {}; {
  qpidEnv = stdenvNoCC.mkDerivation {
    name = "my-gcc-environment";
    buildInputs = [
        vim
        gcc13
        gdb
        openmpi
        mpi
        openssh
    ];
  };
}
