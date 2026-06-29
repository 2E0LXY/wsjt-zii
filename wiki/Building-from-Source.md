# Building from Source

## Linux (Debian / Ubuntu 22.04+)

```bash
sudo apt-get install -y build-essential cmake ninja-build gfortran \
  qtbase5-dev qttools5-dev qtmultimedia5-dev libqt5serialport5-dev \
  libqt5websockets5-dev libqt5sql5-sqlite libqt5svg5-dev \
  libfftw3-dev libboost-log-dev libboost-thread-dev \
  libhamlib-dev libhamlib-utils libusb-1.0-0-dev libudev-dev portaudio19-dev

git clone https://github.com/2E0LXY/wsjt-zii
cd wsjt-zii
mkdir build && cd build
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_INSTALL_PREFIX=/usr \
      -DWSJT_GENERATE_DOCS=OFF ..
ninja
cpack -G DEB
```

## Windows (MSYS2 / MinGW64)

See `.github/workflows/windows-installer.yml` for the full reproducible CI script. Key steps:

1. Install MSYS2 MinGW64 with Qt5, fftw, boost, portaudio, nsis
2. Pin `gcc` + `gcc-fortran` to 14.2.0-3 (avoids `.tls_common` GCC 15 bug)
3. Build Hamlib 4.5.5 from source (`--disable-winradio`)
4. `cmake -DWSJT_WITH_OMNIRIG=OFF ...`
5. `ninja` then `cpack -G NSIS`

## CMake Options

| Option | Default | Description |
|--------|---------|-------------|
| `WSJT_WITH_OMNIRIG` | `ON` | Enable OmniRig CAT control (Windows only, requires OmniRig COM server) |
| `WSJT_GENERATE_DOCS` | `ON` | Generate asciidoctor HTML documentation |
| `WSJT_SKIP_MANPAGES` | `OFF` | Skip man page generation |
