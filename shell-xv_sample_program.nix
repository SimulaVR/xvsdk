{ pkgs ? import <nixpkgs> {} }:

let
  xvsdk = pkgs.callPackage ./xvsdk.nix {};
in

pkgs.stdenv.mkDerivation {
  name = "xv_sample_program";
  src = ./.;
  
  buildInputs = [ xvsdk pkgs.gcc pkgs.gcc-unwrapped.lib ];

  buildPhase = ''
    # This phase intentionally left empty
  '';

  installPhase = ''
    mkdir -p $out/bin
  '';

  shellHook = ''
    export LD_LIBRARY_PATH=${xvsdk}/lib:${xvsdk}/lib/opencv4.2:$LD_LIBRARY_PATH
    
    xvSampleBuildC() {
      gcc -I${xvsdk}/include -L${xvsdk}/lib -o xv_sample_program_c xv_sample_program_c.c -lxv_c_wrapper -lxvsdk -Wl,-rpath,${xvsdk}/lib:${xvsdk}/lib/opencv4.2
    }
    
    xvSampleBuildCPP() {
      g++ -std=c++11 -I${xvsdk}/include/xvsdk -L${xvsdk}/lib -o xv_sample_program_cpp xv_sample_program_cpp.cpp -lxvsdk -Wl,-rpath,${xvsdk}/lib:${xvsdk}/lib/opencv4.2
    }
    
    xvSampleRunC() {
      ./xv_sample_program_c
    }
    
    xvSampleRunCPP() {
      ./xv_sample_program_cpp
    }
  '';
}
