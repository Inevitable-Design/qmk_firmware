export function Name() { return "Womier SK87 QMK"; }
export function Version() { return "1.0.0"; }
export function VendorId() { return 0x342D; }
export function ProductId() { return 0xE401; }
export function Publisher() { return "Womier"; }
export function DeviceType() { return "keyboard"; }
export function Size() { return [19, 7]; }
export function DefaultPosition() { return [10, 100]; }
export function DefaultScale() { return 8.0; }

export function ControllableParameters() {
    return [
        { "property": "shutdownColor", "group": "lighting", "label": "Shutdown Color", "min": "0", "max": "360", "type": "color", "default": "#009bde" },
        { "property": "LightingMode", "group": "lighting", "label": "Lighting Mode", "type": "combobox", "values": ["SignalRGB", "Hardware"], "default": "SignalRGB" },
        { "property": "forcedColor", "group": "lighting", "label": "Forced Color", "min": "0", "max": "360", "type": "color", "default": "#009bde" },
    ];
}

// LED index mapping (order LEDs appear in QMK's rgb_matrix layout)
const vKeys = [
     0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16,  // Row 0
    17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33,  // Row 1
    34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50,  // Row 2
    51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64,              // Row 3
    65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78,              // Row 4
    79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91,                  // Row 5
    92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104              // Underglow
];

const vKeyNames = [
    // Row 0 (indices 0-16)
    "Esc", "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "F11", "F12", "F13", "PrtSc", "ScrLk", "Pause",
    // Row 1 (indices 17-33, reversed in wiring)
    "PgUp", "Home", "Ins", "Backspace", "Equals", "Minus", "0", "9", "8", "7", "6", "5", "4", "3", "2", "1", "Tilde",
    // Row 2 (indices 34-50)
    "Tab", "Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P", "Left Bracket", "Right Bracket", "Backslash", "Delete", "End", "PgDn",
    // Row 3 (indices 51-64, reversed)
    "Enter", "Hash", "Apostrophe", "Semicolon", "L", "K", "J", "H", "G", "F", "D", "S", "A", "CapsLock",
    // Row 4 (indices 65-78)
    "Left Shift", "ISO Backslash", "Z", "X", "C", "V", "B", "N", "M", "Comma", "Period", "Slash", "Right Shift", "Up",
    // Row 5 (indices 79-91, reversed)
    "Right Arrow", "Down", "Left Arrow", "Right Ctrl", "App", "Right Fn", "Right Alt", "Lang1", "Space", "Lang2", "Left Alt", "Left Win", "Left Ctrl",
    // Underglow (indices 92-104)
    "Underglow 1", "Underglow 2", "Underglow 3", "Underglow 4", "Underglow 5", "Underglow 6", "Underglow 7",
    "Underglow 8", "Underglow 9", "Underglow 10", "Underglow 11", "Underglow 12", "Underglow 13"
];

const vKeyPositions = [
    // Row 0 (indices 0-16)
    [0, 0], [1, 0], [2, 0], [3, 0], [4, 0], [5, 0], [6, 0], [7, 0], [8, 0], [9, 0], [10, 0], [11, 0], [12, 0], [13, 0], [15, 0], [16, 0], [17, 0],
    // Row 1 (indices 17-33)
    [17, 2], [16, 2], [15, 2], [13, 2], [12, 2], [11, 2], [10, 2], [9, 2], [8, 2], [7, 2], [6, 2], [5, 2], [4, 2], [3, 2], [2, 2], [1, 2], [0, 2],
    // Row 2 (indices 34-50)
    [0, 3], [1, 3], [2, 3], [3, 3], [4, 3], [5, 3], [6, 3], [7, 3], [8, 3], [9, 3], [10, 3], [11, 3], [12, 3], [13, 3], [15, 3], [16, 3], [17, 3],
    // Row 3 (indices 51-64)
    [14, 4], [12, 4], [11, 4], [10, 4], [9, 4], [8, 4], [7, 4], [6, 4], [5, 4], [4, 4], [3, 4], [2, 4], [1, 4], [0, 4],
    // Row 4 (indices 65-78)
    [0, 5], [1, 5], [2, 5], [3, 5], [4, 5], [5, 5], [6, 5], [7, 5], [8, 5], [9, 5], [10, 5], [11, 5], [13, 5], [16, 5],
    // Row 5 (indices 79-91)
    [17, 6], [16, 6], [15, 6], [14, 6], [12, 6], [11, 6], [10, 6], [9, 6], [6, 6], [4, 6], [2, 6], [1, 6], [0, 6],
    // Underglow (indices 92-104)
    [0, 0], [0, 1], [2, 1], [4, 1], [5, 2], [7, 2], [8, 3], [10, 3], [12, 4], [13, 4], [15, 5], [16, 5], [18, 6]
];

export function LedNames() { return vKeyNames; }
export function LedPositions() { return vKeyPositions; }

export function Validate(endpoint) {
    return endpoint.interface === 1;
}

function requestFirmwareType() {
    device.write([0x00, 0x28], 33);
    device.pause(30);
}

function requestQMKVersion() {
    device.write([0x00, 0x21], 33);
    device.pause(30);
}

function requestSignalRGBProtocolVersion() {
    device.write([0x00, 0x22], 33);
    device.pause(30);
}

function requestUniqueIdentifier() {
    device.write([0x00, 0x23], 33);
    device.pause(30);
}

function requestTotalLeds() {
    device.write([0x00, 0x27], 33);
    device.pause(30);
}

function effectEnable() {
    device.write([0x00, 0x25], 33);
    device.pause(30);
}

function effectDisable() {
    device.write([0x00, 0x26], 33);
    device.pause(30);
}

function StreamLightingData(StartLedIdx, RGBData) {
    const packet = [0x00, 0x24, StartLedIdx, Math.floor(RGBData.length / 3)].concat(RGBData);
    device.write(packet, 33);
}

function grabColors(overrideColor) {
    const RGBData = [];

    for (let iIdx = 0; iIdx < vKeys.length; iIdx++) {
        let color;

        if (overrideColor) {
            color = hexToRgb(overrideColor);
        } else if (LightingMode === "Hardware") {
            color = hexToRgb(forcedColor);
        } else {
            color = device.color(vKeyPositions[iIdx][0], vKeyPositions[iIdx][1]);
        }

        RGBData[iIdx * 3] = color[0];
        RGBData[iIdx * 3 + 1] = color[1];
        RGBData[iIdx * 3 + 2] = color[2];
    }

    return RGBData;
}

function sendColors(overrideColor) {
    const RGBData = grabColors(overrideColor);
    const ledsPerPacket = 9;

    for (let iIdx = 0; iIdx < vKeys.length; iIdx += ledsPerPacket) {
        const count = Math.min(ledsPerPacket, vKeys.length - iIdx);
        const chunk = RGBData.slice(iIdx * 3, (iIdx + count) * 3);

        StreamLightingData(iIdx, chunk);
    }
}

function hexToRgb(hex) {
    const result = /^#?([a-f\d]{2})([a-f\d]{2})([a-f\d]{2})$/i.exec(hex);
    return result ? [parseInt(result[1], 16), parseInt(result[2], 16), parseInt(result[3], 16)] : [0, 0, 0];
}

export function Initialize() {
    requestFirmwareType();
    requestQMKVersion();
    requestSignalRGBProtocolVersion();
    requestUniqueIdentifier();
    requestTotalLeds();
    effectEnable();
}

export function Render() {
    sendColors();
}

export function Shutdown(SystemSuspending) {
    if (SystemSuspending) {
        sendColors("#000000");
    } else {
        if (LightingMode === "SignalRGB") {
            sendColors(shutdownColor);
        } else {
            effectDisable();
        }
    }
}
