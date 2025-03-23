#pragma once

#include <Windows.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <winternl.h>
#pragma comment(lib, "ntdll.lib")

#include <xorstr.hpp>

typedef int BOOL;

enum class KeyboardKey : unsigned char
{
    a = 0x4,
    b = 0x5,
    c = 0x6,
    d = 0x7,
    e = 0x8,
    f = 0x9,
    g = 0xA,
    h = 0xB,
    i = 0xC,
    j = 0xD,
    k = 0xE,
    l = 0xF,
    m = 0x10,
    n = 0x11,
    o = 0x12,
    p = 0x13,
    q = 0x14,
    r = 0x15,
    s = 0x16,
    t = 0x17,
    u = 0x18,
    v = 0x19,
    w = 0x1A,
    x = 0x1B,
    y = 0x1C,
    z = 0x1D,
    n1 = 0x1E,
    n2 = 0x1F,
    n3 = 0x20,
    n4 = 0x21,
    n5 = 0x22,
    n6 = 0x23,
    n7 = 0x24,
    n8 = 0x25,
    n9 = 0x26,
    n0 = 0x27,
    enter = 0x28,
    esc = 0x29,
    back_space = 0x2A,
    tab = 0x2B,
    space = 0x2C,
    minus = 0x2D,
    equal = 0x2E,
    square_bracket_left = 0x2F,
    square_bracket_right = 0x30,
    back_slash = 0x31,
    back_slash_ = 0x32,
    column = 0x33,
    quote = 0x34,
    back_tick = 0x35,
    comma = 0x36,
    period = 0x37,
    slash = 0x38,
    cap = 0x39,
    f1 = 0x3A,
    f2 = 0x3B,
    f3 = 0x3C,
    f4 = 0x3D,
    f5 = 0x3E,
    f6 = 0x3F,
    f7 = 0x40,
    f8 = 0x41,
    f9 = 0x42,
    f10 = 0x43,
    f11 = 0x44,
    f12 = 0x45,
    snapshot = 0x46,
    scroll_lock = 0x47,
    pause = 0x48,
    insert = 0x49,
    home = 0x4A,
    page_up = 0x4B,
    del = 0x4C,
    end = 0x4D,
    page_down = 0x4E,
    right = 0x4F,
    left = 0x50,
    down = 0x51,
    up = 0x52,
    numlock = 0x53,
    numpad_div = 0x54,
    numpad_mul = 0x55,
    numpad_minus = 0x56,
    numpad_plus = 0x57,
    numpad_enter = 0x58,
    numpad_1 = 0x59,
    numpad_2 = 0x5A,
    numpad_3 = 0x5B,
    numpad_4 = 0x5C,
    numpad_5 = 0x5D,
    numpad_6 = 0x5E,
    numpad_7 = 0x5F,
    numpad_8 = 0x60,
    numpad_9 = 0x61,
    numpad_0 = 0x62,
    numpad_dec = 0x63,
    apps = 0x65,
    f13 = 0x68,
    f14 = 0x69,
    f15 = 0x6A,
    f16 = 0x6B,
    f17 = 0x6C,
    f18 = 0x6D,
    f19 = 0x6E,
    f20 = 0x6F,
    f21 = 0x70,
    f22 = 0x71,
    f23 = 0x72,
    f24 = 0x73,
    rwin = 0x8C,
    f24_ = 0x94,
    lctrl = 0xE0,
    lshift = 0xE1,
    lalt = 0xE2,
    lwin = 0xE3,
    rctrl = 0xE4,
    rshift = 0xE5,
    ralt = 0xE6,
    rwin_ = 0xE7
};

class Keyboard {
public:
    BOOL keyboard_open(void);
    void keyboard_close(void);
    BOOL keyboard_press(KeyboardKey key);
    BOOL keyboard_release(void);
    BOOL keyboard_type(const std::vector<KeyboardKey>& keys);
    BOOL keyboard_multi_press(const std::vector<KeyboardKey>& keys);
    BOOL keyboard_release_all(void);

    std::wstring findDriver();

    static KeyboardKey VKToKeyboardKey(int vkCode) {
        switch (vkCode) {
            // Letters
        case 'A': return KeyboardKey::a;
        case 'B': return KeyboardKey::b;
        case 'C': return KeyboardKey::c;
        case 'D': return KeyboardKey::d;
        case 'E': return KeyboardKey::e;
        case 'F': return KeyboardKey::f;
        case 'G': return KeyboardKey::g;
        case 'H': return KeyboardKey::h;
        case 'I': return KeyboardKey::i;
        case 'J': return KeyboardKey::j;
        case 'K': return KeyboardKey::k;
        case 'L': return KeyboardKey::l;
        case 'M': return KeyboardKey::m;
        case 'N': return KeyboardKey::n;
        case 'O': return KeyboardKey::o;
        case 'P': return KeyboardKey::p;
        case 'Q': return KeyboardKey::q;
        case 'R': return KeyboardKey::r;
        case 'S': return KeyboardKey::s;
        case 'T': return KeyboardKey::t;
        case 'U': return KeyboardKey::u;
        case 'V': return KeyboardKey::v;
        case 'W': return KeyboardKey::w;
        case 'X': return KeyboardKey::x;
        case 'Y': return KeyboardKey::y;
        case 'Z': return KeyboardKey::z;

            // Numbers
        case '1': return KeyboardKey::n1;
        case '2': return KeyboardKey::n2;
        case '3': return KeyboardKey::n3;
        case '4': return KeyboardKey::n4;
        case '5': return KeyboardKey::n5;
        case '6': return KeyboardKey::n6;
        case '7': return KeyboardKey::n7;
        case '8': return KeyboardKey::n8;
        case '9': return KeyboardKey::n9;
        case '0': return KeyboardKey::n0;

            // Function keys
        case VK_F1: return KeyboardKey::f1;
        case VK_F2: return KeyboardKey::f2;
        case VK_F3: return KeyboardKey::f3;
        case VK_F4: return KeyboardKey::f4;
        case VK_F5: return KeyboardKey::f5;
        case VK_F6: return KeyboardKey::f6;
        case VK_F7: return KeyboardKey::f7;
        case VK_F8: return KeyboardKey::f8;
        case VK_F9: return KeyboardKey::f9;
        case VK_F10: return KeyboardKey::f10;
        case VK_F11: return KeyboardKey::f11;
        case VK_F12: return KeyboardKey::f12;

            // Special keys
        case VK_RETURN: return KeyboardKey::enter;
        case VK_ESCAPE: return KeyboardKey::esc;
        case VK_BACK: return KeyboardKey::back_space;
        case VK_TAB: return KeyboardKey::tab;
        case VK_SPACE: return KeyboardKey::space;
        case VK_SNAPSHOT: return KeyboardKey::snapshot;
        case VK_INSERT: return KeyboardKey::insert;
        case VK_DELETE: return KeyboardKey::del;
        case VK_HOME: return KeyboardKey::home;
        case VK_END: return KeyboardKey::end;
        case VK_PRIOR: return KeyboardKey::page_up;
        case VK_NEXT: return KeyboardKey::page_down;
        case VK_LEFT: return KeyboardKey::left;
        case VK_RIGHT: return KeyboardKey::right;
        case VK_UP: return KeyboardKey::up;
        case VK_DOWN: return KeyboardKey::down;

            // Modifier keys
        case VK_LCONTROL: return KeyboardKey::lctrl;
        case VK_RCONTROL: return KeyboardKey::rctrl;
        case VK_CONTROL: return KeyboardKey::lctrl;
        case VK_LSHIFT: return KeyboardKey::lshift;
        case VK_RSHIFT: return KeyboardKey::rshift;
        case VK_LMENU: return KeyboardKey::lalt;
        case VK_RMENU: return KeyboardKey::ralt;
        case VK_LWIN: return KeyboardKey::lwin;
        case VK_RWIN: return KeyboardKey::rwin;

        default: return KeyboardKey{}; // Return empty/default key if not found
        }
    }

private:
    struct KEYBOARD_IO
    {
        unsigned char unknown0;
        unsigned char unknown1;
        KeyboardKey button0;
        KeyboardKey button1;
        KeyboardKey button2;
        KeyboardKey button3;
        KeyboardKey button4;
        KeyboardKey button5;
    };

    static BOOL callKeyboard(KEYBOARD_IO* buffer);
    BOOL sendKeyboardInput(KeyboardKey b0 = {}, KeyboardKey b1 = {}, KeyboardKey b2 = {}, KeyboardKey b3 = {}, KeyboardKey b4 = {}, KeyboardKey b5 = {});
};