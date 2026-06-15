# Womier SK87 — SignalRGB plugin

Host-side plugin that pairs with the QMK SignalRGB community module compiled into this fork's firmware. Lets [SignalRGB](https://signalrgb.com/) control all 87 keys + 13 underglow LEDs (105 total) per-key.

## What you need

- A Womier SK87 flashed with this fork's firmware (any build from `womier` or `sk87-snake-wip` branch — see [`../../../../BUILD.md`](../../../../BUILD.md)).
- [SignalRGB](https://signalrgb.com/) installed on Windows (the only supported host OS today). macOS/Linux builds of SignalRGB exist but plugin support is Windows-first.
- The keyboard connected over **USB**. SignalRGB streams ~60 Hz of color data; 2.4 GHz / Bluetooth bandwidth is too low for smooth streaming.

## Install

### 1. Drop the plugin file

Copy `WomierSK87.js` into your SignalRGB plugins directory:

| OS       | Path                                                  |
|----------|-------------------------------------------------------|
| Windows  | `%USERPROFILE%\Documents\WhirlwindFX\Plugins\`        |
| macOS    | `~/Documents/WhirlwindFX/Plugins/`                    |
| Linux    | `~/Documents/WhirlwindFX/Plugins/`                    |

Create the folder if it doesn't exist. The path is the SignalRGB user-plugins directory; SignalRGB scans it on startup.

### 2. Restart SignalRGB

Quit SignalRGB completely (right-click the tray icon → **Exit**, not just close) and re-open. New plugins are only loaded on launch.

### 3. Verify the device shows up

Open SignalRGB → **Devices**. The keyboard should appear as **"Womier SK87 QMK"**. If it doesn't:

- Confirm the keyboard is in **USB mode** (Fn + 6 binds to `KC_USB`).
- Confirm you flashed the fork's firmware (the stock Womier firmware does **not** expose the SignalRGB RAW HID interface).
- Open SignalRGB → **Settings** → **Logs** and search for `342D` / `E401`. If those VID/PIDs aren't being enumerated at all, the plugin's `Validate` function isn't matching — check that interface 1 of the keyboard is exposed (it should be, with this firmware).
- On Windows, USB HID devices sometimes need to be re-plugged after a firmware change for SignalRGB to re-enumerate.

### 4. Apply an effect

Pick any built-in effect (Spectrum, Rainbow Wave, etc.) — it should immediately render across the keyboard.

## Settings

The plugin exposes three controls under the device's **Settings** tab:

| Setting              | What it does                                                                                        |
|----------------------|-----------------------------------------------------------------------------------------------------|
| **Lighting Mode**    | `SignalRGB` streams per-key colors from the active effect. `Hardware` ignores SignalRGB and sends a single forced color (`Forced Color`). |
| **Forced Color**     | Used only in `Hardware` lighting mode. A single static color for the entire keyboard.               |
| **Shutdown Color**   | Sent when SignalRGB exits cleanly (so the keyboard doesn't go dark on quit). Pure black `#000000` is sent automatically on system suspend regardless of this setting. |

## How it works

- Plugin claims **interface 1** of the keyboard (the RAW HID interface — interface 0 is the standard keyboard HID).
- USB IDs: VID `0x342D`, PID `0xE401`.
- On `Initialize`, the plugin sends a sequence of capability-probe packets (`0x21`–`0x28`) and then `effectEnable` (`0x25`) to put the firmware's SignalRGB module into "accept stream" mode.
- On every frame, `Render` calls `sendColors`, which slices the 105-LED RGB buffer into 9-LED chunks and ships each as a `0x24` RAW HID packet. The firmware's SignalRGB module decodes those into `rgb_matrix_set_color` calls.
- On `Shutdown`, the plugin either sends a final flat color (`SignalRGB` mode) or `effectDisable` (`Hardware` mode) so the firmware reverts to its built-in `rgb_matrix` effect.

## Known limitations

- **Snake minigame races SignalRGB.** On the `sk87-snake-wip` branch, the firmware ships a Snake game (Fn + S). While SignalRGB is streaming, its `0x24` packets land via `raw_hid_receive` and can paint over the game's pixels between frames. **Pause SignalRGB on the PC before playing.**
- **USB only.** 2.4 GHz / BT transports drop frames; you'll see noticeable streaks if you try to stream over wireless.
- **Hardware lighting mode is single-color only.** If you want per-key static patterns from the firmware, use the keyboard's built-in `rgb_matrix` modes (Fn + cluster — see [`../readme.md`](../readme.md)) instead.

## Layout reference

The plugin's `vKeyPositions` array maps QMK LED indices to a 19×7 grid for SignalRGB's UI. If you ever extend it (new key, missing LED), the order matches QMK's `rgb_matrix.layout` in [`../keyboard.json`](../keyboard.json). LED order alternates direction per matrix row — see the table at the top of `WomierSK87.js`.
