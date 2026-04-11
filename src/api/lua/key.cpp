#include "api/lua/key.h"

#include <windows.h>

Key::Key(QObject *parent)
    : QObject(parent),
      m_vkHash{
          {"BACKSPACE", VK_BACK},
          {"TAB", VK_TAB},
          {"ENTER", VK_RETURN},
          {"SHIFT", VK_SHIFT},
          {"CTRL", VK_CONTROL},
          {"ALT", VK_MENU},
          {"PAUSE", VK_PAUSE},
          {"CAPSLOCK", VK_CAPITAL},
          {"ESC", VK_ESCAPE},
          {"SPACE", VK_SPACE},
          {"PAGEUP", VK_PRIOR},
          {"PAGEDOWN", VK_NEXT},
          {"END", VK_END},
          {"HOME", VK_HOME},
          {"LEFT", VK_LEFT},
          {"UP", VK_UP},
          {"RIGHT", VK_RIGHT},
          {"DOWN", VK_DOWN},
          {"PRINTSCREEN", VK_SNAPSHOT},
          {"INSERT", VK_INSERT},
          {"DELETE", VK_DELETE},
          {"0", 48},
          {"1", 49},
          {"2", 50},
          {"3", 51},
          {"4", 52},
          {"5", 53},
          {"6", 54},
          {"7", 55},
          {"8", 56},
          {"9", 57},
          {"A", 65},
          {"B", 66},
          {"C", 67},
          {"D", 68},
          {"E", 69},
          {"F", 70},
          {"G", 71},
          {"H", 72},
          {"I", 73},
          {"J", 74},
          {"K", 75},
          {"L", 76},
          {"M", 77},
          {"N", 78},
          {"O", 79},
          {"P", 80},
          {"Q", 81},
          {"R", 82},
          {"S", 83},
          {"T", 84},
          {"U", 85},
          {"V", 86},
          {"W", 87},
          {"X", 88},
          {"Y", 89},
          {"Z", 90}
      } {
}

void Key::tap(const std::string &key) {
    INPUT inputs[2] = {};
    inputs[0].type = INPUT_KEYBOARD;
    inputs[0].ki.wVk = m_vkHash[QString::fromStdString(key)];
    inputs[1] = inputs[0];
    inputs[1].ki.dwFlags |= KEYEVENTF_KEYUP;
    SendInput(2, inputs, sizeof(INPUT));
}

void Key::type(const std::string &text) {
    for (const wchar_t ch : QString::fromStdString(text).toStdWString()) {
        INPUT inputs[2] = {};
        inputs[0].type = INPUT_KEYBOARD;
        inputs[0].ki.wScan = ch;
        inputs[0].ki.dwFlags = KEYEVENTF_UNICODE;
        inputs[1] = inputs[0];
        inputs[1].ki.dwFlags |= KEYEVENTF_KEYUP;
        SendInput(2, inputs, sizeof(INPUT));
    }
}
