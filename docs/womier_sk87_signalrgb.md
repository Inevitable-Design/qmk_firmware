# Womier SK87 — SignalRGB & VIA Support

Knowledge base for the fork-specific changes that bring VIA and SignalRGB support to the **Womier SK87** wireless keyboard, along with the upstream-QMK API fixes required to keep it compiling against modern `qmk_firmware` master.

> Target: `keyboards/womier/sk87` (WB32FQ95 MCU, 87-key TKL, 2.4 GHz + Bluetooth + USB).
> Commits: `a247d3f8` (VIA/SignalRGB + upstream fixes) and `c1900a79` (battery indicator).

---

## 1. Why this fork diverges from upstream Womier

The upstream Womier QMK fork (`womier/qmk_firmware`) targets an older QMK snapshot and does not ship with:

- VIA (key remapping app) support
- SignalRGB (PC-side per-key RGB control) support
- Fixes for the QMK API churn that happened after the fork branched (GPIO macros, RGB keycodes, `keyboard_protocol` global, etc.)

This branch rebases the SK87 board onto current `qmk/qmk_firmware` master, adds the two integrations, and makes the minimum changes required for the board to compile, boot, and enumerate on all three transports (USB / 2.4 GHz / Bluetooth).

---

## 2. The USB endpoint budget problem

The SK87's MCU is a **WB32FQ95**. Its USB peripheral has a hard limit of **3 IN endpoints** (plus EP0). Every HID interface QMK enables consumes endpoints from that budget.

### Default layout (upstream, before changes)

| Interface        | Endpoint usage                    |
|------------------|-----------------------------------|
| Keyboard HID     | 1 IN                              |
| Mouse HID        | 1 IN                              |
| Extrakey (consumer/system) | 1 IN                    |
| **Total**        | **3 IN — budget full**            |

There is no room for the RAW HID interface that both VIA and SignalRGB require.

### Fix — two changes in `keyboard.json` + `rules.mk`

1. **`"mousekey": false`** in `keyboard.json` — drops the mouse HID interface, freeing 1 endpoint. This is the reason mouse keys are unavailable on this build; the SK87 simply cannot carry both mouse-keys and RAW HID on the WB32's 3-endpoint USB.
2. **`KEYBOARD_SHARED_EP = yes`** in `rules.mk` — packs the keyboard, NKRO, and extrakey HID reports onto a single shared endpoint using report IDs.

### Resulting layout

| Interface        | Endpoint usage                    |
|------------------|-----------------------------------|
| Shared HID (keyboard + NKRO + extrakey) | 1 IN         |
| RAW HID (VIA + SignalRGB) | 1 IN                     |
| Console (debug)  | 1 IN                              |
| **Total**        | **3 IN — budget fits**            |

Tradeoff accepted: no mouse keys from the keyboard. SignalRGB and VIA both function.

---

## 3. SignalRGB integration

SignalRGB is integrated via the **QMK community module** system, not as a board-level fork.

### What was added

- `.gitmodules` — new submodule entry:
  ```ini
  [submodule "modules/signalrgb"]
      path = modules/signalrgb
      url = https://github.com/SRGBmods/QMK_Community_Module
  ```
- `modules/signalrgb` — the submodule itself (pointer commit).
- `keyboards/womier/sk87/keyboard.json` — `"modules": ["signalrgb"]` entry opts the SK87 into the module.
- `keyboards/womier/sk87/signalrgb-plugin/WomierSK87.js` — host-side SignalRGB plugin (paired install — see [`keyboards/womier/sk87/signalrgb-plugin/README.md`](../keyboards/womier/sk87/signalrgb-plugin/README.md)).

### How it works

The SignalRGB community module registers itself with QMK's RGB matrix pipeline. When the SignalRGB PC app sends frame data over RAW HID, the module intercepts it and drives the per-LED colors directly, bypassing the local RGB matrix effect engine for the duration of the session.

No custom code lives in the SK87 board files for the firmware-side module — it is entirely self-contained. The only board-level requirement is that RAW HID be available (see §2).

### Setup for end users

1. Clone with submodules: `git submodule update --init --recursive`.
2. Flash a build of this firmware (or download a pre-built `.hex` from [Releases](../../../releases)).
3. Install SignalRGB on the PC and drop `WomierSK87.js` into the SignalRGB plugins directory; full walkthrough in [`signalrgb-plugin/README.md`](../keyboards/womier/sk87/signalrgb-plugin/README.md).

---

## 4. VIA integration

VIA is enabled through the standard QMK toggle:

- `"via": true` in the `features` block of `keyboard.json`.

No `via.json` payload is currently maintained in-tree; VIA uses the default keymap matrix inferred from `keyboard.json`'s `layouts` section. Custom keycodes (`KC_BT1`, `KC_BT2`, `KC_BT3`, `KC_2G4`, `KC_USB`, `KC_BAT`) are declared in the `keycodes` block of `keyboard.json` so they appear in VIA's keycode picker.

---

## 5. Upstream QMK API fixes

These are the smallest changes needed to make the fork compile and run on current QMK master.

### 5.1 Deprecated GPIO macros (`lpwr_wb32.c`)

QMK removed the camelCase pin macros. Replaced across `keyboards/womier/common/wireless/lpwr_wb32.c`:

| Old (deprecated)         | New (current)                     |
|--------------------------|-----------------------------------|
| `setPinOutputOpenDrain`  | `gpio_set_pin_output_open_drain`  |
| `writePinLow`            | `gpio_write_pin_low`              |
| `setPinInputHigh`        | `gpio_set_pin_input_high`         |
| `setPinInput`            | `gpio_set_pin_input`              |

### 5.2 `UART_RX_PAL_MODE` fallback (`lpwr_wb32.c`)

Newer QMK chibios HAL headers no longer define `UART_RX_PAL_MODE` for the WB32 by default, causing a build break in wireless sleep/wake code. Added a fallback at the top of the file:

```c
#ifndef UART_RX_PAL_MODE
#    ifdef USE_GPIOV1
#        define UART_RX_PAL_MODE PAL_MODE_INPUT
#    else
#        define UART_RX_PAL_MODE 7  // WB32 default alternate function for UART RX
#    endif
#endif
```

### 5.3 `keyboard_protocol` global removed (`transport.c`)

The global `keyboard_protocol` variable was replaced upstream by a setter. Changed in `keyboards/womier/common/wireless/transport.c`:

```c
// before
keyboard_protocol = true; // default with true

// after
usb_device_state_set_protocol(USB_PROTOCOL_REPORT);
```

An `#include "usb_device_state.h"` was added.

### 5.4 RGB → RM keycodes (`keymap.c`)

QMK renamed the old `RGB_*` keycodes to `RM_*` as part of the RGB-matrix / RGB-light split. Rewritten in the default keymap (`keymaps/default/keymap.c`) for both `WIN_FN` and `MAC_FN` layers:

| Old        | New        | Meaning                       |
|------------|------------|-------------------------------|
| `RGB_TOG`  | `RM_TOGG`  | Toggle RGB matrix on/off      |
| `RGB_MOD`  | `RM_NEXT`  | Next effect                   |
| `RGB_SAI`  | `RM_SATU`  | Saturation up                 |
| `RGB_SAD`  | `RM_SATD`  | Saturation down               |
| `RGB_HUI`  | `RM_HUEU`  | Hue up                        |
| `RGB_VAI`  | `RM_VALU`  | Brightness up                 |
| `RGB_VAD`  | `RM_VALD`  | Brightness down               |
| `RGB_SPI`  | `RM_SPDU`  | Effect speed up               |
| `RGB_SPD`  | `RM_SPDD`  | Effect speed down             |

### 5.5 `md_raw` simplification (`md_raw.h`, `md_raw.c`)

The upstream Womier fork carried a fragile line-number hack in `md_raw.h` to hijack calls to `raw_hid_send` across the tree:

```c
// removed
#define RENAME_WITH_LINE(A, B) COMBINE(A, B)
#define COMBINE(A, B) A##B
#define raw_hid_send(a, b) RENAME_WITH_LINE(_temp_rhs_, __LINE__)(a, b)
#define _temp_rhs_29 replaced_hid_send  // raw_hid.h line 29
#define _temp_rhs_461 replaced_hid_send // via.c line 461
```

The intent was to reroute RAW HID traffic to the wireless radio when running untethered. The hack broke the moment upstream added or removed a line from `raw_hid.h` or `via.c` — i.e., every rebase.

**Replacement strategy:** modern QMK routes RAW HID sends through `host_raw_hid_send()` which dispatches to the active host driver (USB or the wireless host driver the Womier fork already registers via `host_set_driver(&wireless_driver)` in `transport.c`). As long as the wireless driver exposes a `send_raw` callback, nothing else needs to intercept.

So `md_raw.h` becomes a near-empty header, and `md_raw.c` keeps only the *receive* half:

```c
void md_receive_raw_cb(uint8_t *data, uint8_t length) {
    void raw_hid_receive(uint8_t * data, uint8_t length);
    raw_hid_receive(data, length);
}
```

The `replaced_hid_send` function and its include of `usb_endpoints.h` / `usb_main.h` were deleted.

**Caveat:** the wireless host driver is expected to carry RAW HID frames to the receiver when the transport is `TRANSPORT_BT` / `TRANSPORT_2G4`. Verify on hardware that SignalRGB still updates per-key color when operating wirelessly; if frames stall, the wireless host driver's `send_raw` hook is the place to look.

---

## 6. Battery indicator (Fn + Space)

Added in commit `c1900a79`. An optional QoL feature unrelated to VIA/SignalRGB but shipped on the same branch.

### Behavior

Holding **Fn + Space** while in wireless mode (BT or 2.4 GHz) lights LEDs 1–10 (the F1–F10 row) as a battery bar:

- 100% → all 10 LEDs lit.
- Each segment = 10%.
- Color:
  - **green** when battery > 60%
  - **yellow** when 20% < battery ≤ 60%
  - **red** when battery ≤ 20%

Releasing the key restores the normal RGB matrix frame.

### Implementation (`sk87.c`)

Gated entirely behind `#ifdef WIRELESS_ENABLE`:

1. A new custom keycode `KC_BAT` is declared in `keyboard.json`.
2. `process_record_kb` sets/clears a static `show_battery` flag on press/release of `KC_BAT` and returns `false` so it is not forwarded to the host.
3. `rgb_matrix_indicators_advanced_kb` reads the battery percentage via `*md_getp_bat()` and paints LEDs 1–10 with the color logic above. Uses ceiling division `(bat + 9) / 10` so e.g. 1% → 1 segment lit.

### Keymap placement

`KC_BAT` is bound to **Space** on both `WIN_FN` and `MAC_FN` layers in the default keymap — i.e. **Fn + Space**.

---

## 7. Snake minigame (Fn + S)

A Snake game played on the keyboard's LEDs. Toggled via the new custom keycode `KC_SNAKE`, bound on **S** in both `WIN_FN` and `MAC_FN` layers.

### Controls

| Key          | Action                                                             |
|--------------|--------------------------------------------------------------------|
| **Fn + S**   | Enter or exit the game                                             |
| **Arrows**   | Steer; 180° reversals are rejected to avoid self-kill on a mistap  |
| **ESC**      | Exit immediately                                                   |
| **Any key**  | Restart after game over                                            |

Modifier and layer keycodes are passed through while the game is active so `Fn` (`MO(WIN_FN)`) still activates the Fn layer and `Fn + S` can always reach `KC_SNAKE` to exit. Every other keycode is consumed so arrow presses and typing don't leak to the host during play.

### Play field — 14 cols × 4 rows

The SK87's LED index order alternates direction per matrix row (routing-driven), so the grid is a hardcoded table derived from `keyboard.json`'s `rgb_matrix.layout`:

```
Row 0 (` 1 2 3 4 5 6 7 8 9 0 - = BSP):   LEDs 33 32 31 30 29 28 27 26 25 24 23 22 21 20
Row 1 (Tab Q W E R T Y U I O P [ ] \):    LEDs 34 35 36 37 38 39 40 41 42 43 44 45 46 47
Row 2 (Caps A S D F G H J K L ; ' # Ent): LEDs 64 63 62 61 60 59 58 57 56 55 54 53 52 51
Row 3 (LSft ISO Z X C V B N M , . / RSft + [wall]):
                                           LEDs 65 66 67 68 69 70 71 72 73 74 75 76 77 -1
```

`-1` means wall (instant death). The ISO LED (index 66) has no physical ANSI key but the LED still lights, so it's a valid cell. One wall cell pads the right edge of the shift row to keep the grid rectangular at 14 wide.

### Render

Painted from inside `rgb_matrix_indicators_advanced_kb`, which short-circuits all existing indicators (caps-lock LED, wls indicator, battery bar) while snake is active so those don't flicker through the game.

- **Head** — bright cyan-green (`0x40, 0xFF, 0x80`), painted last so it wins on self-overlap.
- **Body** — linear green gradient from bright near the head to dim at the tail; a subtle cyan tint near the head provides visual "direction".
- **Food** — red, breathing `0x60 → 0xFF` over ~1.2 s via a triangle wave on `timer_read32()`.
- **Game over** — head pulses orange/yellow at ~1.7 Hz, body goes dim red with the same gradient shape.

### Game tick

Advanced inside the indicator callback using `timer_elapsed32(snake_last_step) >= STEP_MS` as a gate, with `snake_last_step = timer_read32()` reset inside the gate. This is naturally idempotent even if QMK later enables `RGB_MATRIX_LED_PROCESS_LIMIT` (which would fire the indicator callback multiple times per frame).

Step interval: 180 ms, lightly accelerating with length down to 80 ms minimum at ~40 segments.

### State save / restore

On the inactive → active transition, `snake_start` snapshots the full `rgb_matrix` state:

- `rgb_matrix_is_enabled()`
- `rgb_matrix_get_mode()`
- `rgb_matrix_get_hue() / get_sat() / get_val()`
- `rgb_matrix_get_speed()`

It then forces `rgb_matrix_enable_noeeprom()`, switches to `RGB_MATRIX_SOLID_COLOR`, and sets HSV to `(0, 0, 0)` so the rest of the keyboard stays dark. On exit, `snake_stop` restores all six values exactly.

Critical detail: the snapshot is gated by `if (!snake_active)`. Restart-after-death also calls `snake_start()`; re-snapshotting there would clobber the saved state with the game's own `SOLID_COLOR` + HSV(0,0,0), and the eventual exit would "restore" to invisible black. The guard makes the first snapshot sticky until the final exit.

### Known limitation

`modules/signalrgb/signalrgb.c:134` writes LED colors via `rgb_matrix_set_color` from the `raw_hid_receive` callback, outside the indicator pipeline. When SignalRGB is actively streaming, it can clobber the game's pixels between frames. Pause SignalRGB on the PC before playing for a clean display. Switching to `SOLID_COLOR`+black on entry reduces, but does not eliminate, the visible race.

### Flash / RAM cost

- Flash: +1.6 KB (49.8 → 51.4 KB)
- RAM: ~200 B (40-segment body buffer, grid table, state)

### Post-flash

After flashing new firmware that adds `KC_SNAKE`, tap **Fn + Esc** (`EE_CLR`) once. VIA caches numeric keycode IDs in EEPROM; the reset makes it pick up the new keycode.

---

## 8. Firmware size impact

Measured on the SK87 build, in order of accumulation:

| Build                           | Size     |
|---------------------------------|----------|
| Baseline (no SignalRGB, no VIA) | 41.9 KB  |
| + SignalRGB module              | 47.1 KB  |
| + SignalRGB + VIA               | 49.9 KB  |
| + Battery indicator             | 48.6 KB  |
| + Battery + Snake minigame      | 50.2 KB  |

Well within the WB32FQ95's flash budget.

---

## 9. Known warnings / follow-ups

- **`FORCE_NKRO` deprecation warning** — current QMK prefers `NKRO_DEFAULT_ON`. The SK87's config should be migrated when touched again. Non-blocking.
- **No `via.json`** — VIA falls back to the inferred keymap. A hand-authored `via.json` would give a nicer layer-and-macro UI in the VIA app but is not required for remapping.
- **Wireless RAW HID validation** — the simplification in §5.5 removes a shim that explicitly handled the `TRANSPORT_USB` vs wireless case. Confirm on hardware that SignalRGB frames still reach the keyboard over 2.4 GHz / BT, not just USB.

---

## 10. Building

See [`BUILD.md`](../BUILD.md) at the repo root for full Linux + Windows compile and flash instructions, including toolchain install and post-flash setup. Pre-built `.hex` files are also published on [Releases](../../../releases).

Quick version (assuming toolchain is already set up):

```sh
git submodule update --init --recursive
qmk compile -kb womier/sk87 -km default
```

Flash with the WB32 DFU bootloader — enter it by holding **Esc** while plugging in, or by pressing a key bound to `QK_BOOT` if the keymap exposes one.

---

## 11. File-change index

| File                                                  | Reason                                                          |
|-------------------------------------------------------|-----------------------------------------------------------------|
| `.gitmodules`                                         | SignalRGB community module submodule                            |
| `modules/signalrgb`                                   | New submodule pointer                                           |
| `keyboards/womier/sk87/keyboard.json`                 | `via`/`modules`/`mousekey:false`, `KC_BAT` and `KC_SNAKE` keycodes |
| `keyboards/womier/sk87/rules.mk`                      | `KEYBOARD_SHARED_EP = yes`                                      |
| `keyboards/womier/sk87/keymaps/default/keymap.c`      | `RGB_*` → `RM_*`, `KC_BAT` on Fn+Space, `KC_SNAKE` on Fn+S      |
| `keyboards/womier/sk87/sk87.c`                        | Battery indicator, USB-wake fix, NKRO spin-wait removal, snake minigame |
| `keyboards/womier/sk87/readme.md`                     | User-facing change summary                                    |
| `keyboards/womier/sk87/signalrgb-plugin/`             | Host-side SignalRGB plugin + setup README                     |
| `keyboards/womier/common/wireless/lpwr_wb32.c`        | GPIO macros, `UART_RX_PAL_MODE` fallback                      |
| `keyboards/womier/common/wireless/transport.c`        | `keyboard_protocol` → `usb_device_state_set_protocol`         |
| `keyboards/womier/common/wireless/md_raw.c`           | Removed `replaced_hid_send` hack                              |
| `keyboards/womier/common/wireless/md_raw.h`           | Emptied of line-number macro hack                             |
| `BUILD.md`                                            | End-user compile + flash instructions                         |
| `.github/workflows/build-sk87.yml`                    | CI build → rolling release with `.hex` + plugin bundle        |
