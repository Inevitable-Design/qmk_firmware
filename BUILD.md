# Building & flashing the Womier SK87 firmware

This is a fork of [qmk_firmware](https://github.com/qmk/qmk_firmware) that adds VIA + SignalRGB support, a battery indicator on Fn + Space, and (on `sk87-snake-game`) a Snake minigame to the **Womier SK87** wireless TKL.

If you just want a `.hex` to flash, **download the latest pre-built firmware from [Releases](../../releases)**: the rolling tags `latest-master` and `latest-sk87-snake-game` are auto-rebuilt on every push. You only need this guide if you want to compile from source.

## Branches

| Branch              | What it has                                                                       |
|---------------------|-----------------------------------------------------------------------------------|
| `master`            | VIA + SignalRGB + battery indicator + upstream-API/wake/NKRO fixes, on QMK master.                          |
| `sk87-snake-game`    | Everything in `master` plus the Snake minigame (Fn + S).                             |

Pick `master` for everyday use. Pick `sk87-snake-game` if you want the game.

---

## Linux

### 1. Install toolchain

Debian/Ubuntu:

```sh
sudo apt update
sudo apt install -y git python3 python3-pip gcc-arm-none-eabi build-essential dfu-util
pip3 install --user qmk
```

Arch:

```sh
sudo pacman -S git python python-pip arm-none-eabi-gcc arm-none-eabi-newlib base-devel dfu-util
pip install --user qmk
```

Fedora:

```sh
sudo dnf install -y git python3 python3-pip arm-none-eabi-gcc-cs arm-none-eabi-newlib make dfu-util
pip3 install --user qmk
```

Make sure `~/.local/bin` is on your `PATH` so `qmk` is callable.

### 2. Clone and init submodules

```sh
git clone --recursive https://github.com/Inevitable-Design/qmk_firmware.git qmk-womier
cd qmk-womier
git checkout sk87-snake-game   # optional: Snake-game branch (master is the default)
git submodule update --init --recursive
```

The `--recursive` is important: it pulls `lib/chibios`, `lib/chibios-contrib`, and `modules/signalrgb`. Without it the build will fail with missing-headers errors.

### 3. Compile

```sh
export QMK_HOME="$PWD"
qmk compile -kb womier/sk87 -km default
```

The output ends up at `.build/womier_sk87_default.hex` (~50 KB).

### 4. Flash with `wb32-dfu-updater_cli`

The SK87 uses a WB32FQ95 MCU with the WB32 DFU bootloader, **not** standard `dfu-util`. Get the updater:

```sh
git clone https://github.com/WestberryTech/wb32-dfu-updater
cd wb32-dfu-updater
make
sudo cp wb32-dfu-updater_cli /usr/local/bin/
```

Put the keyboard into bootloader (hold **Esc** while plugging in USB), then:

```sh
wb32-dfu-updater_cli -D .build/womier_sk87_default.hex
```

You may need a `udev` rule so the bootloader is writable as a non-root user:

```sh
echo 'SUBSYSTEM=="usb", ATTRS{idVendor}=="342d", ATTRS{idProduct}=="dfa0", MODE="0666"' | sudo tee /etc/udev/rules.d/50-wb32-dfu.rules
sudo udevadm control --reload-rules
```

---

## Windows

### 1. Install QMK MSYS

Download and run the QMK MSYS installer: **<https://msys.qmk.fm/>**. This bundles MSYS2, Python, the ARM cross-compiler, and the QMK CLI in one shot, no `apt`, no `pip`. Open **QMK MSYS** from the Start menu after install; commands below assume that shell.

### 2. Clone and init submodules

```sh
cd /d                          # or wherever you want it
git clone --recursive https://github.com/Inevitable-Design/qmk_firmware.git qmk-womier
cd qmk-womier
git checkout sk87-snake-game   # optional: Snake-game branch (master is the default)
git submodule update --init --recursive
```

### 3. Compile

```sh
export QMK_HOME="$PWD"
export QMK_USERSPACE="$PWD"
qmk compile -kb womier/sk87 -km default
```

If you get a `Could not determine home directory` error, also set `USERPROFILE`:

```sh
export USERPROFILE="C:/Users/$(whoami)"
```

(This is a known quirk of QMK MSYS Python's `Path.home()`; Windows-style env vars don't always propagate into the MSYS shell.)

The output is at `.build/womier_sk87_default.hex`.

### 4. Flash with QMK Toolbox (recommended)

1. Download **QMK Toolbox** from <https://qmk.fm/toolbox> and install. On first run it prompts to install drivers; let it.
2. If the bootloader doesn't enumerate, install the correct driver via [Zadig](https://zadig.akeo.ie/): in Zadig, **Options → List All Devices**, find `WB32 DFU` (or the unrecognized device that appears when the keyboard is in bootloader), and replace its driver with **WinUSB**.
3. Put the keyboard into bootloader (hold **Esc** while plugging in USB). Toolbox shows `*** WB32 DFU device connected`.
4. Click **Open**, pick `.build/womier_sk87_default.hex`, then click **Flash**.

When done, Toolbox prints `Validating ... Done`. The keyboard reboots into the firmware and lights come on.

### 4b. Flash from the command line (alternative)

QMK MSYS bundles `wb32-dfu-updater_cli`:

```sh
wb32-dfu-updater_cli -D .build/womier_sk87_default.hex
```

---

## After flashing

1. **Tap Fn + Esc once** (`EE_CLR`). This wipes the EEPROM; the new build adds custom keycodes (`KC_USB`, `KC_BAT`, `KC_SNAKE`, etc.) that VIA caches by numeric ID, so a fresh layout is needed for them to bind correctly.
2. **(Optional) VIA:** open <https://usevia.app/> in Chrome/Edge, click **Authorize device**, pick the SK87. Remap any key live.
3. **(Optional) SignalRGB:** install the host-side plugin per [`keyboards/womier/sk87/signalrgb-plugin/README.md`](keyboards/womier/sk87/signalrgb-plugin/README.md).

---

## Bootloader entry: quick reference

| Method                    | How                                                                          |
|---------------------------|------------------------------------------------------------------------------|
| **Bootmagic reset**       | Hold **Esc** while plugging in USB.                                          |
| **Keycode**               | Press a key bound to `QK_BOOT` (not bound by default).                       |

Both leave you in the WB32 DFU bootloader, which speaks the WB32-specific DFU protocol (use `wb32-dfu-updater_cli` or QMK Toolbox, **not** `dfu-util`).

---

## Knowledge base

- [`docs/womier_sk87_signalrgb.md`](docs/womier_sk87_signalrgb.md): full deep-dive on what changed in this fork: USB endpoint budget, `KEYBOARD_SHARED_EP`, upstream-API breakage and fixes, battery indicator, snake minigame (where present), file-by-file change index.
- [`keyboards/womier/sk87/readme.md`](keyboards/womier/sk87/readme.md): short user-facing summary of features added.
- [`keyboards/womier/sk87/signalrgb-plugin/README.md`](keyboards/womier/sk87/signalrgb-plugin/README.md): host-side SignalRGB plugin install + settings.
