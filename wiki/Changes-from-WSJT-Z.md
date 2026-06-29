# Changes from WSJT-Z

All changes relative to [sq9fve/wsjt-z](https://github.com/sq9fve/wsjt-z) v2.0.16 / WSJT-X 3.0.0.

## v3.0.0 — 29 June 2026

### Bug fixes

| # | File | Description |
|---|------|-------------|
| 1 | `all2cab.py` | Python 2 `print` statements — tool crashed on Python 3 |
| 2 | `all2cab.py` | `f.close` missing parentheses — file descriptor leaked |
| 3 | `UDPExamples/decode_WSJT-UDP.py` | Null QDate sentinel wrong (`-1` → `-(2**63)`) |
| 4 | `UDPExamples/decode_WSJT-UDP.py` | Multicast UDP bound to group address — non-portable on Windows/macOS |
| 5 | `widgets/mainwindow.cpp` | `auto_qrm_guard_state` missing `superFox()` guard — auto-halt fired in Fox mode |
| 6 | `map65/libm65/decode0.f90` | `NFFT` undeclared as array bound — GFortran 14 strict-mode rejection |
| 7 | `Transceiver/TransceiverFactory.cpp` | OmniRig include/enum/case blocks not guarded by `WSJT_NO_OMNIRIG` |
| 8 | `CMakeLists.txt` | Missing `-fallow-argument-mismatch` — MAP65 legacy Fortran type-punning rejected by GFortran 14 |
| 9 | `CMake/Modules/FindFFTW3.cmake` | `NOT WIN32` guard excluded fftw3f_threads on Windows — link error on MSYS2 modular packages |

### Build / CMake

- `WSJT_WITH_OMNIRIG` / `WSJT_NO_OMNIRIG` CMake option and C++ guards added
- `GCC_RUNTIME_OVERRIDE_DIR` CMake variable added to `bundle_fixup/CMakeLists.txt` for DLL injection
- GitHub Actions CI/CD for Linux (`.deb`) and Windows (NSIS `.exe`) — see `.github/workflows/`
- Windows build fully documented in `docs/WINDOWS-BUILD-PITFALLS.md`
- Hamlib 4.5.5 built from source in Windows CI (not in MSYS2 pacman)
- GCC compiler pinned to 14.2.0-3 in Windows CI (GCC 15+ `.tls_common` regression)
- GCC 14 runtime DLLs injected via `GCC_RUNTIME_OVERRIDE_DIR` to fix `__emutls` installer crash

### Identity

- Rebranded as **WSJT-Zii**, independent of `sq9fve/wsjt-z`
- `PROJECT_VENDOR`, `PROJECT_CONTACT`, `PROJECT_HOMEPAGE` updated to 2E0LXY
