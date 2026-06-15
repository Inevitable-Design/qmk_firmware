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
| Build                | Size     |
|----------------------|----------|
| Original (default)   | 41.9 KB  |
| + SignalRGB          | 47.1 KB  |
| + SignalRGB + VIA    | 49.9 KB  |
