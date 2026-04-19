// Copyright 2024 Wind (@yelishang)
// SPDX-License-Identifier: GPL-2.0-or-later

#include QMK_KEYBOARD_H

#ifdef WIRELESS_ENABLE
#    include "wireless.h"
#endif

typedef union {
    uint32_t raw;
    struct {
        uint8_t flag : 1;
        uint8_t devs : 3;
    };
} confinfo_t;
confinfo_t confinfo;

uint32_t post_init_timer = 0x00;

void eeconfig_confinfo_update(uint32_t raw) {

    eeconfig_update_kb(raw);
}

uint32_t eeconfig_confinfo_read(void) {

    return eeconfig_read_kb();
}

void eeconfig_confinfo_default(void) {

    confinfo.flag = true;
#ifdef WIRELESS_ENABLE
    confinfo.devs = DEVS_USB;
#endif

    eeconfig_confinfo_update(confinfo.raw);
}

void eeconfig_confinfo_init(void) {

    confinfo.raw = eeconfig_confinfo_read();
    if (!confinfo.raw) {
        eeconfig_confinfo_default();
    }
}

void keyboard_post_init_kb(void) {

#ifdef CONSOLE_ENABLE
    debug_enable = true;
#endif

    eeconfig_confinfo_init();

#ifdef LED_POWER_EN_PIN
    gpio_set_pin_output(LED_POWER_EN_PIN);
    gpio_write_pin_low(LED_POWER_EN_PIN);
#endif

#ifdef USB_POWER_EN_PIN
    gpio_write_pin_low(USB_POWER_EN_PIN);
    gpio_set_pin_output(USB_POWER_EN_PIN);
#endif

#ifdef WIRELESS_ENABLE
    wireless_init();
    wireless_devs_change(!confinfo.devs, confinfo.devs, false);
    post_init_timer = timer_read32();
#endif

    keyboard_post_init_user();
}

#ifdef WIRELESS_ENABLE

void usb_power_connect(void) {

#    ifdef USB_POWER_EN_PIN
    gpio_write_pin_low(USB_POWER_EN_PIN);
#    endif
}

void usb_power_disconnect(void) {

#    ifdef USB_POWER_EN_PIN
    gpio_write_pin_high(USB_POWER_EN_PIN);
#    endif
}

void suspend_power_down_kb(void) {

#    ifdef LED_POWER_EN_PIN
    gpio_write_pin_high(LED_POWER_EN_PIN);
#    endif
    suspend_power_down_user();
}

void suspend_wakeup_init_kb(void) {

#    ifdef LED_POWER_EN_PIN
    gpio_write_pin_low(LED_POWER_EN_PIN);
#    endif

    wireless_devs_change(wireless_get_current_devs(), wireless_get_current_devs(), false);
    suspend_wakeup_init_user();
}

void wireless_post_task(void) {

    // auto switching devs
    if (post_init_timer && timer_elapsed32(post_init_timer) >= 100) {
        md_send_devctrl(MD_SND_CMD_DEVCTRL_FW_VERSION);   // get the module fw version.
        md_send_devctrl(MD_SND_CMD_DEVCTRL_SLEEP_BT_EN);  // timeout 30min to sleep in bt mode, enable
        md_send_devctrl(MD_SND_CMD_DEVCTRL_SLEEP_2G4_EN); // timeout 30min to sleep in 2.4g mode, enable
        wireless_devs_change(!confinfo.devs, confinfo.devs, false);
        post_init_timer = 0x00;
    }
}

uint32_t wls_process_long_press(uint32_t trigger_time, void *cb_arg) {
    uint16_t keycode = *((uint16_t *)cb_arg);

    switch (keycode) {
        case KC_BT1: {
            wireless_devs_change(wireless_get_current_devs(), DEVS_BT1, true);
        } break;
        case KC_BT2: {
            wireless_devs_change(wireless_get_current_devs(), DEVS_BT2, true);
        } break;
        case KC_BT3: {
            wireless_devs_change(wireless_get_current_devs(), DEVS_BT3, true);
        } break;
        case KC_2G4: {
            wireless_devs_change(wireless_get_current_devs(), DEVS_2G4, true);
        } break;
        default:
            break;
    }

    return 0;
}

bool process_record_wls(uint16_t keycode, keyrecord_t *record) {
    static uint16_t keycode_shadow                     = 0x00;
    static deferred_token wls_process_long_press_token = INVALID_DEFERRED_TOKEN;

    keycode_shadow = keycode;

#    ifndef WLS_KEYCODE_PAIR_TIME
#        define WLS_KEYCODE_PAIR_TIME 3000
#    endif

#    define WLS_KEYCODE_EXEC(wls_dev)                                                                                          \
        do {                                                                                                                   \
            if (record->event.pressed) {                                                                                       \
                wireless_devs_change(wireless_get_current_devs(), wls_dev, false);                                             \
                if (wls_process_long_press_token == INVALID_DEFERRED_TOKEN) {                                                  \
                    wls_process_long_press_token = defer_exec(WLS_KEYCODE_PAIR_TIME, wls_process_long_press, &keycode_shadow); \
                }                                                                                                              \
            } else {                                                                                                           \
                cancel_deferred_exec(wls_process_long_press_token);                                                            \
                wls_process_long_press_token = INVALID_DEFERRED_TOKEN;                                                         \
            }                                                                                                                  \
        } while (false)

    switch (keycode) {
        case KC_BT1: {
            WLS_KEYCODE_EXEC(DEVS_BT1);
        } break;
        case KC_BT2: {
            WLS_KEYCODE_EXEC(DEVS_BT2);
        } break;
        case KC_BT3: {
            WLS_KEYCODE_EXEC(DEVS_BT3);
        } break;
        case KC_2G4: {
            WLS_KEYCODE_EXEC(DEVS_2G4);
        } break;
        case KC_USB: {
            if (record->event.pressed) {
                wireless_devs_change(wireless_get_current_devs(), DEVS_USB, false);
            }
        } break;
        default:
            return true;
    }

    return false;
}
#endif

#ifdef WIRELESS_ENABLE
static bool show_battery = false;

// Prevent deep sleep while on USB. On USB the host can suspend at any time,
// and our EXTI-based wake path does not restart the USB driver on resume
// (wireless_devs_change(USB, USB) short-circuits when old==new), so after
// a host sleep/wake cycle the keyboard can come back with no lights and
// no input until a physical replug. USB also supplies power, so there is
// no battery benefit to deep-sleeping it.
bool lpwr_is_allow_timeout_hook(void) {
    return wireless_get_current_devs() != DEVS_USB;
}
#endif

#ifdef RGB_MATRIX_ENABLE
// Snake game — implementation lives further down in the RGB_MATRIX_ENABLE block
extern bool snake_active;
void snake_start(void);
void snake_stop(void);
bool snake_process_record(uint16_t keycode, keyrecord_t *record);
#endif

bool process_record_kb(uint16_t keycode, keyrecord_t *record) {

    if (process_record_user(keycode, record) != true) {
        return false;
    }

#ifdef WIRELESS_ENABLE
    if (process_record_wls(keycode, record) != true) {
        return false;
    }

    if (keycode == KC_BAT) {
        show_battery = record->event.pressed;
        return false;
    }
#endif

#ifdef RGB_MATRIX_ENABLE
    if (keycode == KC_SNAKE) {
        if (record->event.pressed) {
            if (snake_active) {
                snake_stop();
            } else {
                snake_start();
            }
        }
        return false;
    }
    if (snake_active) {
        return snake_process_record(keycode, record);
    }
#endif

    return true;
}

#ifdef RGB_MATRIX_ENABLE

#    ifdef WIRELESS_ENABLE
bool wls_rgb_indicator_reset        = false;
uint32_t wls_rgb_indicator_timer    = 0x00;
uint32_t wls_rgb_indicator_interval = 0;
uint32_t wls_rgb_indicator_times    = 0;
uint32_t wls_rgb_indicator_index    = 0;
RGB wls_rgb_indicator_rgb           = {0};

void rgb_matrix_wls_indicator_set(uint8_t index, RGB rgb, uint32_t interval, uint8_t times) {

    wls_rgb_indicator_timer = timer_read32();

    wls_rgb_indicator_index    = index;
    wls_rgb_indicator_interval = interval;
    wls_rgb_indicator_times    = times * 2;
    wls_rgb_indicator_rgb      = rgb;
}

void wireless_devs_change_kb(uint8_t old_devs, uint8_t new_devs, bool reset) {

    wls_rgb_indicator_reset = reset;

    if (confinfo.devs != wireless_get_current_devs()) {
        confinfo.devs = wireless_get_current_devs();
        eeconfig_confinfo_update(confinfo.raw);
    }

    switch (new_devs) {
        case DEVS_BT1: {
            if (reset) {
                rgb_matrix_wls_indicator_set(35, (RGB){RGB_BLUE}, 200, 1);
            } else {
                rgb_matrix_wls_indicator_set(35, (RGB){RGB_BLUE}, 500, 1);
            }
        } break;
        case DEVS_BT2: {
            if (reset) {
                rgb_matrix_wls_indicator_set(36, (RGB){RGB_BLUE}, 200, 1);
            } else {
                rgb_matrix_wls_indicator_set(36, (RGB){RGB_BLUE}, 500, 1);
            }
        } break;
        case DEVS_BT3: {
            if (reset) {
                rgb_matrix_wls_indicator_set(37, (RGB){RGB_BLUE}, 200, 1);
            } else {
                rgb_matrix_wls_indicator_set(37, (RGB){RGB_BLUE}, 500, 1);
            }
        } break;
        case DEVS_2G4: {
            if (reset) {
                rgb_matrix_wls_indicator_set(38, (RGB){RGB_BLUE}, 200, 1);
            } else {
                rgb_matrix_wls_indicator_set(38, (RGB){RGB_BLUE}, 500, 1);
            }
        } break;
        default:
            break;
    }
}

bool rgb_matrix_wls_indicator_cb(void) {

    if (*md_getp_state() != MD_STATE_CONNECTED) {
        wireless_devs_change_kb(wireless_get_current_devs(), wireless_get_current_devs(), wls_rgb_indicator_reset);
        return true;
    }

    // refresh led
    led_wakeup();

    return false;
}

void rgb_matrix_wls_indicator(void) {

    if (wls_rgb_indicator_timer) {

        if (timer_elapsed32(wls_rgb_indicator_timer) >= wls_rgb_indicator_interval) {
            wls_rgb_indicator_timer = timer_read32();

            if (wls_rgb_indicator_times) {
                wls_rgb_indicator_times--;
            }

            if (wls_rgb_indicator_times <= 0) {
                wls_rgb_indicator_timer = 0x00;
                if (rgb_matrix_wls_indicator_cb() != true) {
                    return;
                }
            }
        }

        if (wls_rgb_indicator_times % 2) {
            rgb_matrix_set_color(wls_rgb_indicator_index, wls_rgb_indicator_rgb.r, wls_rgb_indicator_rgb.g, wls_rgb_indicator_rgb.b);
        } else {
            rgb_matrix_set_color(wls_rgb_indicator_index, 0x00, 0x00, 0x00);
        }
    }
}
#    endif

// ---------- Snake game ----------
// Toggled via KC_SNAKE (Fn+S). Play field is a 14x4 grid covering the number
// row through the shift row. Arrow keys steer, ESC exits, Fn+S also exits.
// On entry we switch the rgb_matrix effect to SOLID_COLOR+black so the rest
// of the keyboard stays dark and doesn't fight the game paint; on exit we
// restore the previous effect.

#    define SNAKE_W 14
#    define SNAKE_H 4
#    define SNAKE_MAX_LEN 40
#    define SNAKE_STEP_MS 180
#    define SNAKE_MIN_STEP_MS 80

typedef enum {
    SNAKE_DIR_UP,
    SNAKE_DIR_DOWN,
    SNAKE_DIR_LEFT,
    SNAKE_DIR_RIGHT,
} snake_dir_t;

typedef struct {
    int8_t x;
    int8_t y;
} snake_pos_t;

// LED index per grid cell. Derived from keyboard.json rgb_matrix.layout —
// matrix rows alternate assignment direction, so the mapping is hardcoded.
// -1 = wall (instant death).
static const int8_t snake_grid[SNAKE_H][SNAKE_W] = {
    // ` 1 2 3 4 5 6 7 8 9 0 - = BSP
    { 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20 },
    // Tab Q W E R T Y U I O P [ ] Bsl
    { 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47 },
    // Caps A S D F G H J K L ; ' # Enter
    { 64, 63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51 },
    // LSft ISO Z X C V B N M , . / RSft [wall]
    { 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, -1 },
};

bool                snake_active     = false;
static bool         snake_game_over  = false;
static snake_pos_t  snake_body[SNAKE_MAX_LEN];
static uint8_t      snake_length     = 0;
static snake_dir_t  snake_dir        = SNAKE_DIR_RIGHT;
static snake_dir_t  snake_dir_queued = SNAKE_DIR_RIGHT;
static snake_pos_t  snake_food       = {0, 0};
static uint32_t     snake_last_step  = 0;
static uint32_t     snake_rand_state = 0xDEADBEEF;

// Full RGB matrix state saved on entry so exit is a perfect restore.
static uint8_t snake_saved_mode    = 0;
static uint8_t snake_saved_hue     = 0;
static uint8_t snake_saved_sat     = 0;
static uint8_t snake_saved_val     = 0;
static uint8_t snake_saved_speed   = 0;
static bool    snake_saved_enabled = true;

static uint8_t snake_rand(uint8_t max) {
    snake_rand_state ^= snake_rand_state << 13;
    snake_rand_state ^= snake_rand_state >> 17;
    snake_rand_state ^= snake_rand_state << 5;
    return max ? (uint8_t)(snake_rand_state % max) : 0;
}

static bool snake_cell_is_wall(int8_t x, int8_t y) {
    if (x < 0 || x >= SNAKE_W || y < 0 || y >= SNAKE_H) return true;
    return snake_grid[y][x] < 0;
}

static bool snake_cell_on_body(int8_t x, int8_t y) {
    for (uint8_t i = 0; i < snake_length; i++) {
        if (snake_body[i].x == x && snake_body[i].y == y) return true;
    }
    return false;
}

static void snake_place_food(void) {
    for (uint8_t tries = 0; tries < 100; tries++) {
        int8_t x = (int8_t)snake_rand(SNAKE_W);
        int8_t y = (int8_t)snake_rand(SNAKE_H);
        if (snake_cell_is_wall(x, y)) continue;
        if (snake_cell_on_body(x, y)) continue;
        snake_food.x = x;
        snake_food.y = y;
        return;
    }
    // Fallback: linear scan for any empty cell
    for (int8_t y = 0; y < SNAKE_H; y++) {
        for (int8_t x = 0; x < SNAKE_W; x++) {
            if (!snake_cell_is_wall(x, y) && !snake_cell_on_body(x, y)) {
                snake_food.x = x;
                snake_food.y = y;
                return;
            }
        }
    }
}

void snake_start(void) {
    // Only snapshot + switch rgb_matrix on the inactive → active transition.
    // Restart-after-death also calls snake_start(); re-snapshotting there
    // would clobber the original state with SOLID_COLOR/HSV(0,0,0) and make
    // the eventual exit "restore" to black.
    if (!snake_active) {
        snake_saved_enabled = rgb_matrix_is_enabled();
        snake_saved_mode    = rgb_matrix_get_mode();
        snake_saved_hue     = rgb_matrix_get_hue();
        snake_saved_sat     = rgb_matrix_get_sat();
        snake_saved_val     = rgb_matrix_get_val();
        snake_saved_speed   = rgb_matrix_get_speed();

        if (!snake_saved_enabled) rgb_matrix_enable_noeeprom();
        rgb_matrix_mode_noeeprom(RGB_MATRIX_SOLID_COLOR);
        rgb_matrix_sethsv_noeeprom(0, 0, 0);
    }

    snake_active     = true;
    snake_game_over  = false;
    snake_length     = 3;
    snake_body[0]    = (snake_pos_t){8, 1};
    snake_body[1]    = (snake_pos_t){7, 1};
    snake_body[2]    = (snake_pos_t){6, 1};
    snake_dir        = SNAKE_DIR_RIGHT;
    snake_dir_queued = SNAKE_DIR_RIGHT;
    snake_rand_state ^= timer_read32();
    snake_place_food();
    snake_last_step = timer_read32();
}

void snake_stop(void) {
    snake_active    = false;
    snake_game_over = false;

    rgb_matrix_mode_noeeprom(snake_saved_mode);
    rgb_matrix_sethsv_noeeprom(snake_saved_hue, snake_saved_sat, snake_saved_val);
    rgb_matrix_set_speed_noeeprom(snake_saved_speed);
    if (!snake_saved_enabled) rgb_matrix_disable_noeeprom();
}

static void snake_step(void) {
    if (snake_game_over) return;
    snake_dir = snake_dir_queued;

    snake_pos_t new_head = snake_body[0];
    switch (snake_dir) {
        case SNAKE_DIR_UP:    new_head.y--; break;
        case SNAKE_DIR_DOWN:  new_head.y++; break;
        case SNAKE_DIR_LEFT:  new_head.x--; break;
        case SNAKE_DIR_RIGHT: new_head.x++; break;
    }

    if (snake_cell_is_wall(new_head.x, new_head.y)) {
        snake_game_over = true;
        return;
    }

    bool ate = (new_head.x == snake_food.x && new_head.y == snake_food.y);
    // The tail vacates unless we're about to grow.
    uint8_t check_len = (ate || snake_length == 0) ? snake_length : (uint8_t)(snake_length - 1);
    for (uint8_t i = 0; i < check_len; i++) {
        if (snake_body[i].x == new_head.x && snake_body[i].y == new_head.y) {
            snake_game_over = true;
            return;
        }
    }

    if (ate && snake_length < SNAKE_MAX_LEN) snake_length++;
    for (int8_t i = (int8_t)snake_length - 1; i > 0; i--) {
        snake_body[i] = snake_body[i - 1];
    }
    snake_body[0] = new_head;

    if (ate) snake_place_food();
}

static void snake_tick(void) {
    uint32_t delay = SNAKE_STEP_MS;
    if (snake_length > 5) {
        uint32_t accel = (uint32_t)(snake_length - 5) * 3;
        uint32_t span  = SNAKE_STEP_MS - SNAKE_MIN_STEP_MS;
        if (accel > span) accel = span;
        delay = SNAKE_STEP_MS - accel;
    }
    if (timer_elapsed32(snake_last_step) >= delay) {
        snake_last_step = timer_read32();
        snake_step();
    }
}

static void snake_paint_cell(int8_t x, int8_t y, uint8_t r, uint8_t g, uint8_t b,
                             uint8_t led_min, uint8_t led_max) {
    if (x < 0 || x >= SNAKE_W || y < 0 || y >= SNAKE_H) return;
    int8_t led = snake_grid[y][x];
    if (led < 0) return;
    if ((uint8_t)led < led_min || (uint8_t)led >= led_max) return;
    rgb_matrix_set_color((uint8_t)led, r, g, b);
}

// 0..255 triangle wave over `period_ms` milliseconds.
static uint8_t snake_triangle(uint32_t period_ms) {
    uint32_t half = period_ms / 2;
    if (half == 0) return 0;
    uint32_t t = timer_read32() % period_ms;
    if (t < half) {
        return (uint8_t)((t * 255) / half);
    }
    return (uint8_t)(((period_ms - t) * 255) / half);
}

static void snake_render(uint8_t led_min, uint8_t led_max) {
    // Clear play area
    for (int8_t y = 0; y < SNAKE_H; y++) {
        for (int8_t x = 0; x < SNAKE_W; x++) {
            snake_paint_cell(x, y, 0x00, 0x00, 0x00, led_min, led_max);
        }
    }

    // Food: slow red breathe (0x60 .. 0xFF over ~1.2 s)
    uint8_t food_pulse = snake_triangle(1200);
    uint8_t food_r     = (uint8_t)(0x60 + ((uint16_t)food_pulse * 0x9F / 0xFF));
    snake_paint_cell(snake_food.x, snake_food.y, food_r, 0x00, 0x00, led_min, led_max);

    // Body: gradient from bright green near the head to dim green at the tail.
    // On game over: dim red gradient, same shape.
    for (uint8_t i = 1; i < snake_length; i++) {
        int16_t val = 0xC0 - (int16_t)(i * 8);
        if (val < 0x28) val = 0x28;
        if (snake_game_over) {
            snake_paint_cell(snake_body[i].x, snake_body[i].y, (uint8_t)val, 0x00, 0x00, led_min, led_max);
        } else {
            // Subtle cyan tint at the front fading to pure green at the tail,
            // just enough to read the snake as having a "direction".
            int16_t tint = 0x30 - (int16_t)(i * 4);
            if (tint < 0) tint = 0;
            snake_paint_cell(snake_body[i].x, snake_body[i].y, 0, (uint8_t)val, (uint8_t)tint, led_min, led_max);
        }
    }

    // Head — painted last so it wins on overlap.
    if (snake_length > 0) {
        if (snake_game_over) {
            // Fast orange pulse to sell "you died".
            uint8_t p = snake_triangle(600);
            uint8_t r = (uint8_t)(0x70 + ((uint16_t)p * 0x8F / 0xFF));
            uint8_t g = (uint8_t)(0x18 + ((uint16_t)p * 0x28 / 0xFF));
            snake_paint_cell(snake_body[0].x, snake_body[0].y, r, g, 0x00, led_min, led_max);
        } else {
            // Bright cyan-tinted green — distinct from body.
            snake_paint_cell(snake_body[0].x, snake_body[0].y, 0x40, 0xFF, 0x80, led_min, led_max);
        }
    }
}

// Pass-through for modifier and layer keycodes so Fn (MO) stays functional
// while the game is active — otherwise Fn+S could never fire to exit.
static bool snake_is_passthrough(uint16_t keycode) {
    if (keycode >= KC_LCTL && keycode <= KC_RGUI) return true;
    if (keycode >= QK_MOMENTARY && keycode <= QK_MOMENTARY_MAX) return true;
    if (keycode >= QK_TOGGLE_LAYER && keycode <= QK_TOGGLE_LAYER_MAX) return true;
    if (keycode >= QK_TO && keycode <= QK_TO_MAX) return true;
    if (keycode >= QK_ONE_SHOT_LAYER && keycode <= QK_ONE_SHOT_LAYER_MAX) return true;
    if (keycode >= QK_DEF_LAYER && keycode <= QK_DEF_LAYER_MAX) return true;
    if (keycode >= QK_LAYER_TAP && keycode <= QK_LAYER_TAP_MAX) return true;
    if (keycode >= QK_MOD_TAP && keycode <= QK_MOD_TAP_MAX) return true;
    if (keycode >= QK_ONE_SHOT_MOD && keycode <= QK_ONE_SHOT_MOD_MAX) return true;
    return false;
}

bool snake_process_record(uint16_t keycode, keyrecord_t *record) {
    if (snake_is_passthrough(keycode)) return true;
    if (!record->event.pressed) return false;

    if (snake_game_over) {
        if (keycode == KC_ESC) {
            snake_stop();
        } else {
            snake_start();
        }
        return false;
    }

    switch (keycode) {
        case KC_UP:
            if (snake_dir != SNAKE_DIR_DOWN) snake_dir_queued = SNAKE_DIR_UP;
            break;
        case KC_DOWN:
            if (snake_dir != SNAKE_DIR_UP) snake_dir_queued = SNAKE_DIR_DOWN;
            break;
        case KC_LEFT:
            if (snake_dir != SNAKE_DIR_RIGHT) snake_dir_queued = SNAKE_DIR_LEFT;
            break;
        case KC_RIGHT:
            if (snake_dir != SNAKE_DIR_LEFT) snake_dir_queued = SNAKE_DIR_RIGHT;
            break;
        case KC_ESC:
            snake_stop();
            break;
        default:
            break;
    }
    return false;
}

bool rgb_matrix_indicators_advanced_kb(uint8_t led_min, uint8_t led_max) {

    bool continue_user_indicators = rgb_matrix_indicators_advanced_user(led_min, led_max);

    if (snake_active) {
        snake_tick();
        snake_render(led_min, led_max);
        return false;
    }

    if (continue_user_indicators) {
        if (host_keyboard_led_state().caps_lock) {
            rgb_matrix_set_color(64, 0x77, 0x77, 0x77);
        }

#    ifdef WIRELESS_ENABLE
        rgb_matrix_wls_indicator();
#    endif
    }

#    ifdef WIRELESS_ENABLE
    // Battery indicator on F-row (LED indices 1-10 = F1-F10). Painted last,
    // unconditionally, so it wins over SignalRGB frames streamed via raw_hid
    // — otherwise the bar flickers as HID packets race with the indicator pass.
    // On USB the reading stays at the default 100 (md_inquire_bat only runs
    // on wireless), so the bar effectively becomes a "held" smoke test there.
    if (show_battery) {
        uint8_t bat      = *md_getp_bat();
        uint8_t segments = (bat + 9) / 10;
        if (segments > 10) segments = 10;

        for (uint8_t i = 0; i < 10; i++) {
            uint8_t led = i + 1;
            if (led < led_min || led >= led_max) continue;

            if (i < segments) {
                if (bat > 60) {
                    rgb_matrix_set_color(led, 0x00, 0x77, 0x00);
                } else if (bat > 20) {
                    rgb_matrix_set_color(led, 0x77, 0x77, 0x00);
                } else {
                    rgb_matrix_set_color(led, 0x77, 0x00, 0x00);
                }
            } else {
                rgb_matrix_set_color(led, 0x00, 0x00, 0x00);
            }
        }
    }
#    endif

    return continue_user_indicators;
}


void md_devs_change(uint8_t devs, bool reset) {

    switch (devs) {
        case DEVS_USB: {
            md_send_devctrl(MD_SND_CMD_DEVCTRL_USB);
        } break;
        case DEVS_2G4: {
            if (reset) {
                md_send_devctrl(MD_SND_CMD_DEVCTRL_PAIR);
            } else {
                md_send_devctrl(MD_SND_CMD_DEVCTRL_2G4);
            }
        } break;
        case DEVS_BT1: {
            if (reset) {
                md_send_devctrl(MD_SND_CMD_DEVCTRL_PAIR);
            } else {
                md_send_devctrl(MD_SND_CMD_DEVCTRL_BT1);
            }
        } break;
        case DEVS_BT2: {
            if (reset) {
                md_send_devctrl(MD_SND_CMD_DEVCTRL_PAIR);
            } else {
                md_send_devctrl(MD_SND_CMD_DEVCTRL_BT2);
            }
        } break;
        case DEVS_BT3: {
            if (reset) {
                md_send_devctrl(MD_SND_CMD_DEVCTRL_PAIR);
            } else {
                md_send_devctrl(MD_SND_CMD_DEVCTRL_BT3);
            }
        } break;
        default:
            break;
    }
}

#endif

void wireless_send_nkro(report_nkro_t *report) {
    static report_keyboard_t temp_report_keyboard = {0};
    uint8_t wls_report_nkro[MD_SND_CMD_NKRO_LEN]  = {0};

#ifdef NKRO_ENABLE

    if (report != NULL) {
        report_nkro_t temp_report_nkro = *report;
        uint8_t key_count              = 0;

        temp_report_keyboard.mods = temp_report_nkro.mods;
        for (uint8_t i = 0; i < NKRO_REPORT_BITS; i++) {
            key_count += __builtin_popcount(temp_report_nkro.bits[i]);
        }

        /*
         * Use NKRO for sending when more than 6 keys are pressed
         * to solve the issue of the lack of a protocol flag in wireless mode.
         */

        for (uint8_t i = 0; i < key_count; i++) {
            uint8_t usageid;
            uint8_t idx, n = 0;

            for (n = 0; n < NKRO_REPORT_BITS && !temp_report_nkro.bits[n]; n++) {}
            usageid = (n << 3) | biton(temp_report_nkro.bits[n]);
            del_key_bit(&temp_report_nkro, usageid);

            for (idx = 0; idx < WLS_KEYBOARD_REPORT_KEYS; idx++) {
                if (temp_report_keyboard.keys[idx] == usageid) {
                    goto next;
                }
            }

            for (idx = 0; idx < WLS_KEYBOARD_REPORT_KEYS; idx++) {
                if (temp_report_keyboard.keys[idx] == 0x00) {
                    temp_report_keyboard.keys[idx] = usageid;
                    break;
                }
            }
        next:
            if (idx == WLS_KEYBOARD_REPORT_KEYS && (usageid < (MD_SND_CMD_NKRO_LEN * 8))) {
                wls_report_nkro[usageid / 8] |= 0x01 << (usageid % 8);
            }
        }

        temp_report_nkro = *report;

         // find key up and del it.
        uint8_t nkro_keys = key_count;
        for (uint8_t i = 0; i < WLS_KEYBOARD_REPORT_KEYS; i++) {
            report_nkro_t found_report_nkro;
            uint8_t usageid = 0x00;
            uint8_t n;

            found_report_nkro = temp_report_nkro;

            for (uint8_t c = 0; c < nkro_keys; c++) {
                for (n = 0; n < NKRO_REPORT_BITS && !found_report_nkro.bits[n]; n++) {}
                usageid = (n << 3) | biton(found_report_nkro.bits[n]);
                del_key_bit(&found_report_nkro, usageid);
                if (usageid == temp_report_keyboard.keys[i]) {
                    del_key_bit(&temp_report_nkro, usageid);
                    nkro_keys--;
                    break;
                }
            }

            if (usageid != temp_report_keyboard.keys[i]) {
                temp_report_keyboard.keys[i] = 0x00;
            }
        }
    } else {
        memset(&temp_report_keyboard, 0, sizeof(temp_report_keyboard));
    }
#endif
    // smsg_push serializes kb+nkro in order; the previous
    // while(smsg_is_busy()) { wireless_task(); } spin was unsafe — it
    // re-entered the UART pump from the keyboard report path and could
    // hang if the module stopped ACK'ing.
    extern host_driver_t wireless_driver;
    wireless_driver.send_keyboard(&temp_report_keyboard);
    md_send_nkro(wls_report_nkro);
}
