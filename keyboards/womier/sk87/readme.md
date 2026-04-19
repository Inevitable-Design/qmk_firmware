# SK87
[![SK87](https://s21.ax1x.com/2025/01/20/pEkH8wd.png)](https://imgse.com/i/pEkH8wd)
A customizable 80% multimodal keyboard.

* Keyboard Maintainer: [jonylee@hfd](https://github.com/jonylee1986)
* Hardware Supported: [SK87](https://womierkeyboard.com/products/womier-sk87)
* Hardware Availability: [womier](http://www.womierkeyboard.com)

Make example for this keyboard (after setting up your build environment):

    make womier/sk87:default

Flashing example for this keyboard:

    make womier/sk87:default:flash
    
See the [build environment setup](https://docs.qmk.fm/#/getting_started_build_tools) and the [make instructions](https://docs.qmk.fm/#/getting_started_make_guide) for more information. Brand new to QMK? Start with our [Complete Newbs Guide](https://docs.qmk.fm/#/newbs).

## Bootloader

Enter the bootloader in 2 ways:

* **Bootmagic reset**: Hold down the Hold down the top left key (commonly programmed as *Esc*) and plug in the keyboard
* **Keycode in layout**: Press the key mapped to `QK_BOOT` if it is available

## Changes from upstream womier fork

This build is rebased onto the latest `qmk/qmk_firmware` master and includes the following modifications:

### Features added
* **VIA support** — `via: true` in `keyboard.json`. Remap keys live via the VIA app.
* **SignalRGB support** — Community module added at `modules/signalrgb`. Per-key RGB control from SignalRGB on PC.
* **KEYBOARD_SHARED_EP** — Keyboard HID report shares an endpoint with NKRO/extrakey, freeing an endpoint for the RAW HID interface required by VIA and SignalRGB.
* **Battery indicator** — Hold **Fn + Space** in wireless mode to show battery as a 10-LED bar on F1–F10 (green > 60%, yellow > 20%, red ≤ 20%).
* **Snake minigame** — Play Snake on the LEDs (see below).

### Fn-layer custom keycodes
| Keycode      | Default binding | Effect                                                             |
|--------------|-----------------|--------------------------------------------------------------------|
| `KC_USB`     | Fn + 6          | Switch to USB transport                                            |
| `KC_2G4`     | Fn + 5          | Switch to 2.4 GHz dongle transport (hold 3 s to pair)              |
| `KC_BT1`–`KC_BT3` | Fn + 2/3/4 | Switch to Bluetooth slot 1/2/3 (hold 3 s to pair)                  |
| `KC_BAT`     | Fn + Space      | Hold to display battery level on F1–F10                            |
| `KC_SNAKE`   | Fn + S          | Toggle the snake minigame                                          |

## Snake minigame

A Snake game rendered on the keyboard's LEDs. Toggle with **Fn + S**.

### Controls
| Key          | Action                                                             |
|--------------|--------------------------------------------------------------------|
| **Fn + S**   | Enter or exit the game                                             |
| **Arrows**   | Steer (180° reversals are rejected so you can't die instantly)     |
| **ESC**      | Exit immediately                                                   |
| **Any key**  | Restart after game over                                            |

### Play field
A 14 × 4 grid over the main keyboard area:

```
Row 0:  `   1  2  3  4  5  6  7  8  9  0  -  =  BSP
Row 1:  Tab Q  W  E  R  T  Y  U  I  O  P  [  ]  \
Row 2:  Cap A  S  D  F  G  H  J  K  L  ;  '  #  Ent
Row 3:  LSh ISO Z X  C  V  B  N  M  ,  .  /  RSh [wall]
```

Walls (the edges + one cell at the end of the shift row) are instant death. Typing is suppressed while the game is running so arrow presses don't leak to the host; modifier and layer keys still work so **Fn + S** always exits.

### Visual design
* **Head** — bright cyan-green, painted last so it wins on self-overlap.
* **Body** — linear green gradient, bright near the head fading to dim at the tail; a tiny cyan tint near the head gives the snake an obvious direction at a glance.
* **Food** — red, slow breathing pulse (~1.2 s period).
* **Game over** — head pulses orange/yellow (~0.6 s), body goes dim red.

### State preservation
On entry, the full RGB matrix state (enabled, mode, hue, saturation, value, speed) is snapshotted; on exit, all six are restored exactly, so your previous effect comes back intact. The snapshot only fires on the *inactive → active* transition — restart-after-death does **not** re-snapshot, preventing the saved state from getting overwritten with the game's blanked-out `SOLID_COLOR` state.

### Known limitation
If SignalRGB is actively streaming colors when you toggle the game, it can still paint over the game area via its `raw_hid_receive` callback. Pause SignalRGB on the PC for a clean display.

### Post-flash
After flashing new firmware that adds `KC_SNAKE`, tap **Fn + Esc** (`EE_CLR`) once. VIA caches numeric keycode IDs; the EEPROM reset makes it pick up the new keycode.

### Features removed
* **Mouse keys** — Disabled (`mousekey: false`) to stay within the WB32FQ95's 3 USB endpoint limit. VIA/SignalRGB's RAW HID + Console endpoints require the budget that mousekey occupied.

### API fixes for upstream QMK compatibility
* **`transport.c`** — `keyboard_protocol = true` replaced with `usb_device_state_set_protocol(USB_PROTOCOL_REPORT)`. The global `keyboard_protocol` variable was removed upstream.
* **`lpwr_wb32.c`** — Deprecated GPIO macros (`setPinOutputOpenDrain`, `writePinLow`, `setPinInputHigh`, `setPinInput`) replaced with current `gpio_*` equivalents. Added `UART_RX_PAL_MODE` fallback definition (defaults to `7` for WB32).
* **`keymap.c`** — Old `RGB_*` keycodes replaced with new `RM_*` keycodes:
  * `RGB_TOG` → `RM_TOGG`, `RGB_MOD` → `RM_NEXT`
  * `RGB_SAI` → `RM_SATU`, `RGB_SAD` → `RM_SATD`
  * `RGB_HUI` → `RM_HUEU`, `RGB_VAI` → `RM_VALU`, `RGB_VAD` → `RM_VALD`
  * `RGB_SPI` → `RM_SPDU`, `RGB_SPD` → `RM_SPDD`
* **`md_raw.h` / `md_raw.c`** — Removed the fragile line-number-based `raw_hid_send` macro hack. The upstream QMK `host_raw_hid_send()` now routes through the active host driver automatically, making the custom interception unnecessary. The wireless receive callback (`md_receive_raw_cb`) is preserved.

### Known warnings
* `FORCE_NKRO` is deprecated — should be migrated to `NKRO_DEFAULT_ON` in a future update. Non-blocking.

### Firmware size
| Build                          | Size     |
|--------------------------------|----------|
| Original (default)             | 41.9 KB  |
| + SignalRGB                    | 47.1 KB  |
| + SignalRGB + VIA              | 49.9 KB  |
| + Battery indicator            | 48.6 KB  |
| + Battery + Snake minigame     | 50.2 KB  |
