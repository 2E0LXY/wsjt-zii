# Installation

## Linux (Debian / Ubuntu 22.04+)

Download the `.deb` from the [Releases](https://github.com/2E0LXY/wsjt-zii/releases) page, then:

```bash
sudo dpkg -i wsjtz_3.0.0-2.0.16_amd64.deb
sudo apt-get install -f          # resolve any missing deps
```

The application installs to `/usr/bin/wsjtx` and appears in your desktop menu as **WSJT-Z**.

## Windows (64-bit)

Download the `.exe` installer from the [Releases](https://github.com/2E0LXY/wsjt-zii/releases) page and run it.
Default installation path: `C:\WSJT\wsjtx`.

No OmniRig is required for the CI-built releases — all Hamlib-supported radios work out of the box.
