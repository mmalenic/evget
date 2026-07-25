#include "evgetwindows/vk_keysym.h"

#include <windows.h>

#include <cctype>
#include <format>
#include <string>

std::string evgetwindows::NamedKeysym(UINT vk, bool e0) {
    switch (vk) {
        case VK_LSHIFT:
            return "Shift_L";
        case VK_RSHIFT:
            return "Shift_R";
        case VK_LCONTROL:
            return "Control_L";
        case VK_RCONTROL:
            return "Control_R";
        case VK_LMENU:
            return "Alt_L";
        case VK_RMENU:
            return "Alt_R";
        case VK_LWIN:
            return "Super_L";
        case VK_RWIN:
            return "Super_R";
        case VK_SHIFT:
            return "Shift_L";
        case VK_CONTROL:
            return "Control_L";
        case VK_MENU:
            return "Alt_L";
        case VK_CAPITAL:
            return "Caps_Lock";
        case VK_NUMLOCK:
            return "Num_Lock";
        case VK_SCROLL:
            return "Scroll_Lock";
        case VK_RETURN:
            return e0 ? "KP_Enter" : "Return";
        case VK_SPACE:
            return "space";
        case VK_TAB:
            return "Tab";
        case VK_BACK:
            return "BackSpace";
        case VK_ESCAPE:
            return "Escape";
        case VK_PRIOR:
            return "Prior";
        case VK_NEXT:
            return "Next";
        case VK_HOME:
            return "Home";
        case VK_END:
            return "End";
        case VK_INSERT:
            return "Insert";
        case VK_DELETE:
            return "Delete";
        case VK_LEFT:
            return "Left";
        case VK_RIGHT:
            return "Right";
        case VK_UP:
            return "Up";
        case VK_DOWN:
            return "Down";
        case VK_SNAPSHOT:
            return "Print";
        case VK_PAUSE:
            return "Pause";
        case VK_APPS:
            return "Menu";
        case VK_CANCEL:
            return "Cancel";
        case VK_CLEAR:
            return "Clear";
        case VK_HELP:
            return "Help";
        case VK_MULTIPLY:
            return "KP_Multiply";
        case VK_ADD:
            return "KP_Add";
        case VK_SUBTRACT:
            return "KP_Subtract";
        case VK_DECIMAL:
            return "KP_Decimal";
        case VK_DIVIDE:
            return "KP_Divide";
        case VK_SEPARATOR:
            return "KP_Separator";
        case VK_OEM_1:
            return "semicolon";
        case VK_OEM_PLUS:
            return "equal";
        case VK_OEM_COMMA:
            return "comma";
        case VK_OEM_MINUS:
            return "minus";
        case VK_OEM_PERIOD:
            return "period";
        case VK_OEM_2:
            return "slash";
        case VK_OEM_3:
            return "grave";
        case VK_OEM_4:
            return "bracketleft";
        case VK_OEM_5:
            return "backslash";
        case VK_OEM_6:
            return "bracketright";
        case VK_OEM_7:
            return "apostrophe";
        case VK_OEM_102:
            return "less";
        default:
            return std::format("VK_{:#04x}", vk);
    }
}

std::string evgetwindows::VkToKeysymName(UINT vk, bool e0) {
    if (vk >= 'A' && vk <= 'Z') {
        return {static_cast<char>(std::tolower(static_cast<unsigned char>(vk)))};
    }
    if (vk >= '0' && vk <= '9') {
        return {static_cast<char>(vk)};
    }
    if (vk >= VK_F1 && vk <= VK_F24) {
        return std::format("F{}", vk - VK_F1 + 1);
    }
    if (vk >= VK_NUMPAD0 && vk <= VK_NUMPAD9) {
        return std::format("KP_{}", vk - VK_NUMPAD0);
    }
    return NamedKeysym(vk, e0);
}

USHORT evgetwindows::RightShiftScanCode() {
    return static_cast<USHORT>(MapVirtualKey(VK_RSHIFT, MAPVK_VK_TO_VSC));
}

UINT evgetwindows::ResolveVk(const RAWKEYBOARD& keyboard) {
    const bool e0_flag = (keyboard.Flags & RI_KEY_E0) != 0;
    switch (keyboard.VKey) {
        case VK_SHIFT:
            return (keyboard.MakeCode == RightShiftScanCode()) ? VK_RSHIFT : VK_LSHIFT;
        case VK_CONTROL:
            return e0_flag ? VK_RCONTROL : VK_LCONTROL;
        case VK_MENU:
            return e0_flag ? VK_RMENU : VK_LMENU;
        default:
            return keyboard.VKey;
    }
}
