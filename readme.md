# Womier SK87: QMK firmware (with VIA, SignalRGB, battery indicator, and Snake game)

A fork of [`qmk/qmk_firmware`](https://github.com/qmk/qmk_firmware) that adds modern host-side integrations and quality-of-life features to the **Womier SK87** wireless 87-key TKL (WB32FQ95 MCU, 2.4 GHz + Bluetooth + USB).

## ⚠️ Disclaimer: please read

**TL;DR: the actual risk is very low.** The SK87's bootloader is in ROM and is *not* overwritten by a firmware flash. Holding **Esc** while plugging the keyboard in always drops you back into the WB32 DFU bootloader, even if the firmware on it is missing or broken. From there you can re-flash either this firmware *or* the stock Womier firmware using the same tool you used the first time ([QMK Toolbox](https://qmk.fm/toolbox) on Windows, or `wb32-dfu-updater_cli` on Linux). In other words: **you can always roll back.** This is not a one-way door.

**Even so, the terms below govern your use of this firmware. Please read them before flashing:**

- This is an **unofficial community fork**. It is not endorsed by, supported by, or affiliated with Womier in any way. The firmware here is provided **as-is**, with **no warranty** of any kind, express or implied.
- **You alone are responsible** for what happens to your hardware. By downloading, building, flashing, or otherwise using anything in this repository, you accept all risk.
- The maintainer and all contributors accept **no liability** for damage to your keyboard, computer, peripherals, data, or anything else, regardless of cause.
- Flashing custom firmware **may void any manufacturer warranty** Womier offers on the device.
- If any of the above makes you uncomfortable, do not flash this firmware.

### Recovering / rolling back to stock (for beginners)

If something feels off after flashing (typing weird, lights wrong, keyboard not recognised), you don't need to panic; you go back to stock the same way you got here:

1. **Unplug** the keyboard.
2. **Hold the Esc key** and **plug the USB cable back in**, keeping Esc held until the lights settle. The keyboard is now in the WB32 DFU bootloader (it won't type, that's expected).
3. Open **QMK Toolbox** (Windows) or run `wb32-dfu-updater_cli -D <file>.hex` (Linux).
4. Pick the **stock Womier firmware** (`.hex` from <https://womierkeyboard.com/pages/softwares>), or any other build from this repo, and flash it.
5. Unplug, plug back in normally. Done.

The bootloader can't be erased by a normal flash, so this recovery path works even if the keyboard otherwise looks completely dead. (It would take a deliberate, rare misuse to actually brick the bootloader itself.)

### Other practical notes

- **Don't flash on low battery.** A wireless flash that gets interrupted by a dying battery is the one realistic way to corrupt things. Plug in via USB before flashing; the keyboard charges over USB anyway.
- **Wireless pairings may need redoing.** After a flash, the saved BT/2.4 GHz pairings in EEPROM may be cleared, especially if you also do `EE_CLR` (Fn + Esc, recommended after this fork to register new keycodes). Just re-pair via the dongle / Bluetooth, which takes a few seconds.
- **Keep the stock `.hex` somewhere safe.** Even though you can re-download it from Womier's site, having a local copy means you can roll back without needing internet.

## Why this fork

The upstream Womier QMK fork targets an older QMK snapshot and ships **without** VIA, SignalRGB, or fixes for the QMK API churn that broke the board on current `qmk_firmware/master`. This fork:

- **Rebases the SK87 onto current QMK master** and patches the upstream API breakage (deprecated GPIO macros, removed `keyboard_protocol` global, renamed `RGB_*` → `RM_*` keycodes, removed line-number macro hack in `md_raw.h`).
- **Solves the WB32 USB endpoint budget**: the WB32FQ95 has only 3 USB IN endpoints. Disabling mouse keys and enabling `KEYBOARD_SHARED_EP` frees the slot needed for the RAW HID interface that VIA and SignalRGB require.
- **Adds VIA support**: live key remapping via [usevia.app](https://usevia.app/).
- **Adds SignalRGB support**: full per-key RGB streaming from the [SignalRGB](https://signalrgb.com/) Windows app via the QMK community module.
- **Adds a battery indicator**: hold **Fn + Space** in wireless mode to show battery level on F1–F10 as a 10-segment color-coded bar.
- **Adds a Snake game** (snake branch only): toggled with **Fn + S**, played on the LEDs with the arrow keys.
- **Fixes wake/responsiveness bugs**: USB-on-wake reinitialisation, NKRO spin-wait removal, indicator pass ordering for stable battery bar rendering.

## Download pre-built firmware

Don't want to compile? Grab the latest `.hex` straight from the rolling releases. Each release also includes the SignalRGB plugin and the docs in one zip bundle.

All builds live on the [**Releases** page](https://github.com/Inevitable-Design/qmk_firmware/releases). Per-branch quick links:

| Branch                | Features                                        | Latest release                                                                                                                  |
|-----------------------|-------------------------------------------------|---------------------------------------------------------------------------------------------------------------------------------|
| **`womier`**          | VIA + SignalRGB + battery + upstream-API fixes  | [![womier release](https://img.shields.io/github/v/release/Inevitable-Design/qmk_firmware?filter=latest-womier&label=latest-womier)](https://github.com/Inevitable-Design/qmk_firmware/releases/tag/latest-womier) |
| **`sk87-snake-wip`**  | everything in `womier` + Snake game (Fn+S)     | [![snake release](https://img.shields.io/github/v/release/Inevitable-Design/qmk_firmware?filter=latest-sk87-snake-wip&label=latest-snake)](https://github.com/Inevitable-Design/qmk_firmware/releases/tag/latest-sk87-snake-wip) |

Each release contains:

- **`sk87-<branch>-bundle.zip`**: one-click download with everything below.
- `sk87-<branch>.hex`: firmware binary (flash with [QMK Toolbox](https://qmk.fm/toolbox) or `wb32-dfu-updater_cli`).
- `WomierSK87.js`: SignalRGB host-side plugin. Drop in `%USERPROFILE%\Documents\WhirlwindFX\Plugins\` (Windows) or `~/Documents/WhirlwindFX/Plugins/` (mac/Linux).
- `BUILD.md`: compile-from-source steps.
- `SignalRGB-plugin-README.md`: SignalRGB setup walkthrough.

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
| **[`docs/womier_sk87_signalrgb.md`](docs/womier_sk87_signalrgb.md)**                                           | Deep-dive knowledge base: USB endpoint budget, `KEYBOARD_SHARED_EP`, upstream-API fixes, battery indicator, Snake game, file-by-file change index. |
| **[`keyboards/womier/sk87/readme.md`](keyboards/womier/sk87/readme.md)**                                       | User-facing change summary for the SK87 (Fn-layer keycodes, Snake game controls, etc.).         |
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
| `sk87-snake-wip`    | Everything in `womier` + the Snake game (Fn + S).                                        |

## License

GPL-2.0-or-later, same as upstream QMK. See [`license_GPLv2.md`](license_GPLv2.md) and [`license_GPLv3.md`](license_GPLv3.md).

## Credits

- Upstream [QMK firmware](https://github.com/qmk/qmk_firmware): the entire framework this builds on.
- [SRGBmods QMK Community Module](https://github.com/SRGBmods/QMK_Community_Module): the firmware-side SignalRGB integration, vendored as a submodule under [`modules/signalrgb`](modules/signalrgb).
- Original Womier QMK fork by [@yelishang](https://github.com/yelishang) and [@jonylee1986](https://github.com/jonylee1986): the SK87 board files this fork extends.
