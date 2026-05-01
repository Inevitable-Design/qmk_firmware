# Womier SK87 — QMK firmware (with VIA, SignalRGB, battery indicator, and Snake)

A fork of [`qmk/qmk_firmware`](https://github.com/qmk/qmk_firmware) that adds modern host-side integrations and quality-of-life features to the **Womier SK87** wireless 87-key TKL (WB32FQ95 MCU, 2.4 GHz + Bluetooth + USB).

## Why this fork

The upstream Womier QMK fork targets an older QMK snapshot and ships **without** VIA, SignalRGB, or fixes for the QMK API churn that broke the board on current `qmk_firmware/master`. This fork:

- **Rebases the SK87 onto current QMK master** and patches the upstream API breakage (deprecated GPIO macros, removed `keyboard_protocol` global, renamed `RGB_*` → `RM_*` keycodes, removed line-number macro hack in `md_raw.h`).
- **Solves the WB32 USB endpoint budget** — the WB32FQ95 has only 3 USB IN endpoints. Disabling mouse keys and enabling `KEYBOARD_SHARED_EP` frees the slot needed for the RAW HID interface that VIA and SignalRGB require.
- **Adds VIA support** — live key remapping via [usevia.app](https://usevia.app/).
- **Adds SignalRGB support** — full per-key RGB streaming from the [SignalRGB](https://signalrgb.com/) Windows app via the QMK community module.
- **Adds a battery indicator** — hold **Fn + Space** in wireless mode to show battery level on F1–F10 as a 10-segment color-coded bar.
- **Adds a Snake minigame** (snake branch only) — toggled with **Fn + S**, played on the LEDs with the arrow keys.
- **Fixes wake/responsiveness bugs** — USB-on-wake reinitialisation, NKRO spin-wait removal, indicator pass ordering for stable battery bar rendering.

## Download pre-built firmware

Don't want to compile? Grab the latest `.hex` straight from the rolling releases. Each release also includes the SignalRGB plugin and the docs in one zip bundle.

| Branch                | Features                                        | Latest release                                                                                                                  |
|-----------------------|-------------------------------------------------|---------------------------------------------------------------------------------------------------------------------------------|
| **`womier`**          | VIA + SignalRGB + battery + upstream-API fixes  | [![womier release](https://img.shields.io/github/v/release/Inevitable-Design/qmk_firmware?filter=latest-womier&label=latest-womier)](https://github.com/Inevitable-Design/qmk_firmware/releases/tag/latest-womier) |
| **`sk87-snake-wip`**  | everything in `womier` + Snake minigame (Fn+S) | [![snake release](https://img.shields.io/github/v/release/Inevitable-Design/qmk_firmware?filter=latest-sk87-snake-wip&label=latest-snake)](https://github.com/Inevitable-Design/qmk_firmware/releases/tag/latest-sk87-snake-wip) |

Each release contains:

- **`sk87-<branch>-bundle.zip`** — one-click download with everything below.
- `sk87-<branch>.hex` — firmware binary (flash with [QMK Toolbox](https://qmk.fm/toolbox) or `wb32-dfu-updater_cli`).
- `WomierSK87.js` — SignalRGB host-side plugin. Drop in `%USERPROFILE%\Documents\WhirlwindFX\Plugins\` (Windows) or `~/Documents/WhirlwindFX/Plugins/` (mac/Linux).
- `BUILD.md` — compile-from-source steps.
- `SignalRGB-plugin-README.md` — SignalRGB setup walkthrough.

> After flashing, tap **Fn + Esc** (`EE_CLR`) once. VIA caches numeric keycode IDs, so an EEPROM reset is needed for new custom keycodes (`KC_USB`, `KC_BAT`, `KC_SNAKE`, etc.) to bind correctly.

## Compile from source

Full Linux + Windows toolchain install, build, and flash instructions in **[`BUILD.md`](BUILD.md)**.

Quick path (assumes the toolchain is already installed):

```sh
git clone --recursive https://github.com/Inevitable-Design/qmk_firmware.git qmk-womier
cd qmk-womier
git checkout womier            # or: git checkout sk87-snake-wip
git submodule update --init --recursive
export QMK_HOME="$PWD"
qmk compile -kb womier/sk87 -km default
```

Output: `.build/womier_sk87_default.hex`.

## Documentation

| Document                                                                                                       | What it covers                                                                                  |
|----------------------------------------------------------------------------------------------------------------|-------------------------------------------------------------------------------------------------|
| **[`BUILD.md`](BUILD.md)**                                                                                     | End-user compile + flash steps for Linux and Windows, including driver setup and post-flash.    |
| **[`docs/womier_sk87_signalrgb.md`](docs/womier_sk87_signalrgb.md)**                                           | Deep-dive knowledge base: USB endpoint budget, `KEYBOARD_SHARED_EP`, upstream-API fixes, battery indicator, snake minigame, file-by-file change index. |
| **[`keyboards/womier/sk87/readme.md`](keyboards/womier/sk87/readme.md)**                                       | User-facing change summary for the SK87 (Fn-layer keycodes, snake controls, etc.).              |
| **[`keyboards/womier/sk87/signalrgb-plugin/README.md`](keyboards/womier/sk87/signalrgb-plugin/README.md)**     | Step-by-step SignalRGB host setup: install path per OS, settings, troubleshooting.              |

## SignalRGB plugin

The host-side plugin that pairs with the in-firmware SignalRGB module lives at:

> [`keyboards/womier/sk87/signalrgb-plugin/WomierSK87.js`](keyboards/womier/sk87/signalrgb-plugin/WomierSK87.js)

Setup walkthrough: [`keyboards/womier/sk87/signalrgb-plugin/README.md`](keyboards/womier/sk87/signalrgb-plugin/README.md). The plugin is also bundled in every release zip so you can install firmware + host plugin in one shot.

## Branches

| Branch              | Purpose                                                                                  |
|---------------------|------------------------------------------------------------------------------------------|
| `master`            | Upstream `qmk/qmk_firmware` master + the original Womier additions, untouched.           |
| `womier`            | Fork-specific work: VIA + SignalRGB + battery + upstream-API/wake/NKRO fixes.            |
| `sk87-snake-wip`    | Everything in `womier` + the Snake minigame (Fn + S).                                    |

## License

GPL-2.0-or-later, same as upstream QMK. See [`license_GPLv2.md`](license_GPLv2.md) and [`license_GPLv3.md`](license_GPLv3.md).

## Credits

- Upstream [QMK firmware](https://github.com/qmk/qmk_firmware) — the entire framework this builds on.
- [SRGBmods QMK Community Module](https://github.com/SRGBmods/QMK_Community_Module) — the firmware-side SignalRGB integration, vendored as a submodule under [`modules/signalrgb`](modules/signalrgb).
- Original Womier QMK fork by [@yelishang](https://github.com/yelishang) and [@jonylee1986](https://github.com/jonylee1986) — the SK87 board files this fork extends.
