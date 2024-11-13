{ stdenv
, lib
, dpkg
, autoPatchelfHook
, makeWrapper
, patchelf
, libusb1
, libjpeg8
, zlib
, opencv
, boost185
, systemd
, callPackage
}:

let
  octomap = callPackage ./octomap-1.9.nix { };
in

stdenv.mkDerivation rec {
  pname = "xvsdk";
  version = "3.2.0-20230907";

  src = ./xvsdk_3.2.0-20230907_focal_amd64.deb;

  nativeBuildInputs = [
    dpkg
    autoPatchelfHook
    makeWrapper
    patchelf
  ];

  buildInputs = [
    stdenv.cc.cc.lib
    libusb1
    libjpeg8
    zlib
    systemd
    opencv
    octomap
    boost185
  ];

  unpackPhase = "dpkg-deb -x $src .";

  installPhase = ''
    mkdir -p $out
    cp -r usr/* $out/
    
    if [ -d lib ]; then
      cp -r lib $out/
    elif [ -d usr/lib ]; then
      cp -r usr/lib $out/
    else
      echo "Error: Neither 'lib' nor 'usr/lib' directory found"
      exit 1
    fi
    
    mkdir -p $out/include $out/src $out/include2
    cp -r usr/include/* $out/include/
    cp -r usr/include2/* $out/include2/
    cp -r usr/share/xvsdk/* $out/src/
    
    substituteInPlace $out/lib/pkgconfig/xvsdk.pc \
      --replace "/usr" "$out"
    
    mkdir -p $out/bin
    if [ -d usr/bin ]; then
      for f in usr/bin/*; do
        if [ ! -e "$out/bin/$(basename $f)" ]; then
          ln -s $out/$(realpath --relative-to="$out" "$f") $out/bin/$(basename $f)
        fi
      done
    fi

    mkdir -p $out/lib/opencv4.2
    
    ln -sf ${opencv}/lib/libopencv_imgproc.so.409 $out/lib/opencv4.2/libopencv_imgproc.so.4.2
    ln -sf ${opencv}/lib/libopencv_core.so.409 $out/lib/opencv4.2/libopencv_core.so.4.2
    ln -sf ${opencv}/lib/libopencv_highgui.so.409 $out/lib/opencv4.2/libopencv_highgui.so.4.2
    ln -sf ${opencv}/lib/libopencv_imgcodecs.so.409 $out/lib/opencv4.2/libopencv_imgcodecs.so.4.2

    if [ -f $out/lib/x86_64-linux-gnu/libxvuvc.so.0 ]; then
      ln -sf $out/lib/x86_64-linux-gnu/libxvuvc.so.0 $out/lib/libxvuvc.so.0
      patchelf --set-rpath "${libjpeg8}/lib:$out/lib" $out/lib/x86_64-linux-gnu/libxvuvc.so.0
    fi

    ln -s ${libjpeg8}/lib/libjpeg.so.8 $out/lib/libjpeg.so.8

cp ${./xv_c_wrapper.h} $out/include/xvsdk/xv_c_wrapper.h

g++ -std=c++11 -shared -fPIC \
  -I$out/include/xvsdk \
  -I$out/include2 \
  -L$out/lib \
  -L$out/lib/opencv4.2 \
  -L${libjpeg8}/lib \
  -o $out/lib/libxv_c_wrapper.so ${./xv_c_wrapper.cpp} \
  -lxvsdk -lxslam-slam_core -lxslam-uvc-sdk -lxslam-vsc-sdk \
  -loctomap -loctomath -lusb-1.0 -ljpeg \
  -Wl,-rpath,$out/lib:$out/lib/opencv4.2:${octomap}/lib:${libusb1}/lib:${libjpeg8}/lib


    cp ${./xv_c_wrapper.h} $out/include/xv_c_wrapper.h
  '';

  postFixup = ''
    for f in $out/bin/*; do
      wrapProgram $f --prefix LD_LIBRARY_PATH : $out/lib:$out/lib/opencv4.2:${lib.makeLibraryPath buildInputs}
    done
  '';

  meta = with lib; {
    description = "XVSDK for XVisio devices";
    homepage = "https://www.xvisio.com/";
    license = licenses.unfree;
    platforms = ["x86_64-linux"];
    maintainers = with maintainers; [ ];
  };
}