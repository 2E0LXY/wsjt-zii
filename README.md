# WSJT-Y

**Fork by 2E0LXY (Daren, Wakefield, UK). GPL-3.0.**

Based on [sq9fve/wsjt-z](https://github.com/sq9fve/wsjt-z) v2.0.16, itself based on WSJT-X 3.0.0.

---

## What is WSJT-Y?

WSJT-X is the reference implementation of weak-signal amateur radio digital modes (FT8, FT4, WSPR, FST4, MSK144, Q65, JT65, JT9). WSJT-Z extended it with multi-threaded decoding, automation, and band-hopping. WSJT-Y takes that foundation further: correctness fixes, systematic sensitivity improvements across all decoders, new UI features, and reproducible CI/CD builds for Linux and Windows.

---

## Downloads — v3.0.0

| Platform | File | Install |
|---|---|---|
| Windows 10+ 64-bit | `wsjty-3.0.0-2.0.16-win64.exe` | Run installer |
| Debian 12 / Ubuntu 22.04+ | `wsjty_3.0.0-2.0.16_amd64.deb` | `sudo dpkg -i …; sudo apt-get install -f` |

SHA256 checksums are listed on the [releases page](https://github.com/2E0LXY/wsjt-zii/releases/tag/v3.0.0).

---

## Improvements over WSJT-X

### Inherited from WSJT-Z (sq9fve)
- **Multi-threaded FT8 decoder** — parallel worker threads (configurable, default = CPU core count)
- **Auto-sequencing** — automated QSO progression with configurable depth
- **Band hopper** — automatic frequency/band switching on schedule
- **QRM protection** — avoids transmitting when the target frequency is occupied
- **SuperFox / SuperHound** — DXpedition pile-up modes
- **TCI support** — ExpertSDR-compatible rig control via TCP/IP
- **Enhanced filtering** — prefix/suffix, state, DXCC, callsign blacklist/whitelist

### Added by WSJT-Y

#### Decoder sensitivity

| Mode | Improvement | Expected gain |
|---|---|---|
| FT8 (OMP path) | LDPC BP iterations 30→50; OSD ndeep 5→6→7 | ~0.3–0.5 dB |
| FT8 (non-OMP path) | LDPC BP iterations 30→50 | ~0.3 dB |
| FT4 | LDPC BP iterations 30→55; OSD depth 2→3; maxosd 3→4 | ~0.3–0.4 dB |
| FST4 (240/101) | LDPC BP iterations 30→50; maxosd 3→4 | ~0.3 dB |
| FST4 (240/74) | LDPC BP iterations 30→50 (AP fast-scan path preserved) | ~0.3 dB |
| MSK144 (batch) | LDPC BP iterations 10→15 | marginal, latency-limited |
| MSK144 (real-time) | LDPC BP iterations 10→15 | marginal |
| MSK144 OSD | ndeep 5→6 (order-2, K=90, C(90,2)=4005 — trivially cheap) | ~0.1 dB |
| WSPR | OSD ndeep 5→6 (order-4 OSD, C(50,4)≈230k) | ~0.2–0.3 dB |

All LDPC iteration increases are validated against the Shannon limit for each mode.  
Hard Shannon limit for FT8: **−27.5 dB SNR** (50 Hz BW, 77 bits, 12.64 s). The −40 dB figure sometimes claimed elsewhere is physically impossible.

#### Experimental NMS decoder
`Decode → NMS decoder (experimental)` toggles a Normalized Min-Sum check-node update (α=0.75) in place of the default sum-product (tanh/atanh) BP decoder for FT8. Implementation credited to [iu8lmc/Decodium](https://github.com/iu8lmc/Decodium-4.0-Core-Shannon) (GPL-3.0). Claimed gain: +0.2–0.4 dB. **Not the default** — needs on-air statistical validation first. Set env var `WSJTY_USE_NMS_DECODER=1` to force-enable without the menu (for scripted batch comparison tests).

#### WSPR Maximum decode tier
`Decode → Maximum` passes `-o 6` to `wsprd`, enabling the order-4 OSD backend. The `wsprd` `-o` cap was lifted from 5→6 to match.

| Tier | Flags | OSD depth |
|---|---|---|
| Quick | `-qB` | none |
| Medium | `-C 500 -o 4` | 4 |
| Deep | `-C 500 -o 4 -d` | 4 + more candidates |
| **Maximum** | `-C 500 -o 6 -d` | **6 (order-4)** |

#### Waterfall callsign overlay
`View → Show callsigns on waterfall` draws decoded callsigns at their audio frequency on the 2D spectrum/waterfall overlay after each T/R period.

- **Blue** — CQ / DE / QRZ (calling station)
- **Green** — directed at your callsign
- **Grey** — directed at another station

Labels clear automatically each T/R period.

#### DX Station Map
A dockable map panel (`View → DX Station Map`) shows the geographic distribution of decoded stations using the Maidenhead grid system. No Internet connection required — pure QPainter, zero extra dependencies.

- All decoded stations plotted as colour-coded dots, accumulating across periods
- Click a callsign in the decode browser → selected station highlighted with its 4-char minor square and a dashed great-circle arc from your home QTH
- Info bar: callsign, grid, bearing (°), distance (km)
- Hover over the map → shows Maidenhead locator under cursor as tooltip
- Double-click map to clear all plotted stations
- Dock is movable / floatable / closeable; state persists across restarts

#### CAT / rig control
- **OmniRig Rig 1 fix** — `case OmniRigOneId:` label was absent from `TransceiverFactory.cpp` switch. The Rig 1 block compiled as dead code; selecting OmniRig Rig 1 fell through to a default Hamlib path, producing "Hamlib initialisation error". Rig 2 was unaffected because its case label existed.
- **OmniRig re-enabled in Windows build** — previously disabled due to four compounding CI build issues (package name, binary name, WOW64 registry bypass for `.tlb` fetch, narrowing conversion). All four fixed.

#### Bug fixes over WSJT-Z base
1. Operator-precedence error in `ft8bvar.f90` decode loop
2. Integer count/ratio type mismatch producing incorrect decode statistics
3. `log10()` of non-positive value (undefined behaviour on negative SNR edge case)
4. `ctab[]` array out-of-bounds access during callsign encoding
5. Five further correctness issues in the inherited wsjt-z codebase
6. **TX goal-post markers** drawn to wrong pixmap (`m_ScalePixmap` instead of `m_OverlayPixmap`) — markers were invisible in the 2D scope
7. **Waterfall callsign overlay** drawing to wrong pixmap (`m_ScalePixmap` instead of `m_DialOverlayPixmap`) — overlay was invisible
8. **Waterfall overlay accumulation** — overlay not cleared between T/R periods when `m_bars=false`, causing stacking
9. OmniRig Rig 1 missing switch case (above)
10. `OmniRigTransceiver.cpp` narrowing conversion in switch statement (GCC 14 error under `-Wnarrowing`)

#### Dark mode
`View → Dark mode` applies QDarkStyleSheet to the full application UI.

---

## Build

### Linux (Debian 12 / Ubuntu 22.04)

```bash
sudo apt-get install build-essential cmake ninja-build gfortran \
  libfftw3-dev libhamlib-dev libudev-dev libsamplerate-dev \
  qtbase5-dev qtmultimedia5-dev libqt5serialport5-dev \
  libqt5websockets5-dev libqt5sql5-sqlite libqt5svg5-dev

cmake -G Ninja -DCMAKE_BUILD_TYPE=Release \
      -DWSJT_SKIP_MANPAGES=ON ..
ninja
```

### Windows (MSYS2 / MinGW64)

See [`docs/WINDOWS-BUILD-PITFALLS.md`](docs/WINDOWS-BUILD-PITFALLS.md) — 9 documented pitfalls including:
- GCC 16 `.tls_common` regression (pin to GCC 14)
- Qt5 static runtime linking
- OmniRig WOW64 registry bypass
- `dumpcpp-qt5.exe` → `dumpcpp.exe` rename

```bash
pacman -S mingw-w64-x86_64-{gcc,cmake,ninja,qt5-base,qt5-activeqt,...}
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -DWSJT_WITH_OMNIRIG=ON \
      -DOMNIRIG_TLB_PATH="$(cd .. && pwd)/OmniRig.tlb" ..
ninja
```

### CI/CD

GitHub Actions produces signed, reproducible installers on every push to `master`:

| Workflow | Output |
|---|---|
| `linux-deb.yml` | `wsjty_*.deb` (Debian 12 amd64) |
| `windows-installer.yml` | `wsjty-*-win64.exe` (NSIS installer) |

---

## Configuration

Standard WSJT-X settings apply. WSJT-Y–specific options:

| Location | Setting | Purpose |
|---|---|---|
| `Decode` menu | `NMS decoder (experimental)` | Toggle Normalized Min-Sum LDPC |
| `Decode` menu | `Maximum` | 4th WSPR decode depth tier |
| `View` menu | `Show callsigns on waterfall` | Callsign overlay on 2D spectrum |
| `View` menu | `DX Station Map` | Toggle/restore the map dock |
| `Settings → Station` | `My Grid` | Home QTH for map great-circle arcs |

---

## Licence

WSJT-Y is distributed under **GNU GPL v3.0**, the same licence as WSJT-X and WSJT-Z.

- WSJT-X: © Joe Taylor K1JT and the WSJT-X Development Group
- WSJT-Z extensions: © SQ9FVE (Mateusz Lubas)
- WSJT-Y modifications: © 2E0LXY (Daren, 2025–2026)
- NMS decoder: © IU8LMC (Carmine Vitiello), GPL-3.0

See [LICENCE](LICENSE) for full terms.

---

## Support

- Issues: [github.com/2E0LXY/wsjt-zii/issues](https://github.com/2E0LXY/wsjt-zii/issues)
- WSJT-X documentation (applies broadly): [physics.princeton.edu/pulsar/k1jt/wsjtx-doc](https://physics.princeton.edu/pulsar/k1jt/wsjtx-doc/wsjtx-main-2.6.1.html)
