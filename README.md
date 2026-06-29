# WSJT-Zii

**WSJT-Zii** is an independent fork of [WSJT-Z](https://github.com/sq9fve/wsjt-z) by SQ9FVE,
which is itself a fork of [WSJT-X](https://physics.princeton.edu/pulsar/k1jt/wsjtx.html) by K1JT and the WSJT Development Group.

Maintained by **Daren Loxley — 2E0LXY** (Wakefield, West Yorkshire, UK).

---

## What is WSJT-Zii?

WSJT-X is the definitive weak-signal digital-mode application for amateur radio.
WSJT-Z extended it with automation features (auto-sequencing, QRM protection, multi-threaded FT8 decoding, bandhopping, and more).
WSJT-Zii takes that foundation further, applying correctness fixes and shipping reproducible CI/CD release builds for Linux (Debian) and Windows.

This repository is **not connected** to `sq9fve/wsjt-z` and will never auto-merge with it.
All upstream improvements are reviewed and cherry-picked manually.

---

## Differences from WSJT-Z (sq9fve/wsjt-z)

### Bug fixes — v3.0.0 (28 June 2026)

| # | File | Description |
|---|------|-------------|
| 1 | `all2cab.py` | Python 2 `print` statements crash on Python 3 — fixed |
| 2 | `all2cab.py` | `f.close` missing parentheses — file handle leaked — fixed |
| 3 | `UDPExamples/decode_WSJT-UDP.py` | Null QDate sentinel wrong (`-1` instead of `-(2**63)`) — fixed |
| 4 | `UDPExamples/decode_WSJT-UDP.py` | Multicast UDP socket bound to group address — non-portable on Windows/macOS — fixed |
| 5 | `widgets/mainwindow.cpp` | `auto_qrm_guard_state` missing `superFox()` exclusion — auto-halt fired incorrectly during Fox operation — fixed (developer's own TODO resolved) |

### Build & CI/CD

- GitHub Actions workflows for reproducible release builds on every `v*` tag
- Linux: Ubuntu 22.04, Qt 5.15, GFortran 13, Hamlib 4.5.5 → `.deb` package
- Windows: MSYS2/MinGW64, Qt 5.15, GFortran 14 (pinned), Hamlib 4.5.5 → NSIS `.exe` installer
- OmniRig made optional (`-DWSJT_WITH_OMNIRIG=OFF`) — all Hamlib-supported radios work without it

### CMake

- `WSJT_WITH_OMNIRIG` option added (default `ON` for JTSDK builds, `OFF` for CI)

---

## Releases & Downloads

Latest: [v3.0.0](https://github.com/2E0LXY/wsjt-zii/releases/tag/v3.0.0)

| Platform | File | Notes |
|----------|------|-------|
| Linux (Debian/Ubuntu 22.04+) | `wsjtz_3.0.0-2.0.16_amd64.deb` | `sudo dpkg -i …; sudo apt-get install -f` |
| Windows 64-bit | `wsjtz-3.0.0.0-win64.exe` | Run installer, default path `C:\WSJT\wsjtx` |

---

## Building from source

### Linux (Debian / Ubuntu 22.04+)

```bash
sudo apt-get install -y build-essential cmake ninja-build gfortran \
  qtbase5-dev qttools5-dev qtmultimedia5-dev libqt5serialport5-dev \
  libqt5websockets5-dev libqt5sql5-sqlite libqt5svg5-dev \
  libfftw3-dev libboost-log-dev libboost-thread-dev \
  libhamlib-dev libhamlib-utils libusb-1.0-0-dev libudev-dev portaudio19-dev

mkdir build && cd build
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr \
      -DWSJT_GENERATE_DOCS=OFF ..
ninja
cpack -G DEB
```

### Windows (MSYS2 / MinGW64)

See `.github/workflows/windows-installer.yml` for the full reproducible build script.
Key steps: install MSYS2 packages, pin gcc-fortran to 14.2.0-3, build Hamlib 4.5.5 from source, then cmake + ninja + cpack.

---

## Licence

WSJT-Zii is distributed under the **GNU General Public Licence v3.0** (GPL-3.0),
the same licence as WSJT-X and WSJT-Z. See [LICENCE](LICENSE) for full terms.

In short: you may use, modify, and distribute this software freely, provided that
any distributed derivative work is also released under GPL-3.0 and carries
appropriate attribution to the original authors.

**Attribution chain:**
- WSJT-X — © K1JT and the WSJT Development Group
- WSJT-Z — © SQ9FVE (fork of WSJT-X)
- WSJT-Zii — © 2E0LXY (fork of WSJT-Z)

---

## Contributing

Issues and pull requests welcome at https://github.com/2E0LXY/wsjt-zii.
Please do **not** open pull requests against `sq9fve/wsjt-z` or `kj4xo/wsjtx` on behalf of changes made here.

---

## Links

- WSJT-X homepage: https://physics.princeton.edu/pulsar/k1jt/wsjtx.html
- WSJT-Z upstream: https://github.com/sq9fve/wsjt-z
- 2E0LXY amateur radio projects: https://github.com/2E0LXY
