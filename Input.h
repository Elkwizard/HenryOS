#ifndef INPUT_DEFINED
#define INPUT_DEFINED

#include <map>
#include <string>
#include "ScreenSetup.h"
#include "WindowsImport.h"
class InputHandler {
    private:
        std::map<std::string, short> keymap {
            { "Left", VK_LBUTTON },
            { "Middle", VK_MBUTTON },
            { "Right", VK_RBUTTON },
            { "Tab", VK_TAB }, { "Backspace", VK_BACK }, { "Enter", VK_RETURN }, { "Shift", VK_SHIFT }, 
            { "Control", VK_CONTROL }, { "Alt", VK_MENU }, { "Escape", VK_ESCAPE }, { " ", VK_SPACE },
            { "ArrowUp", VK_UP }, { "ArrowDown", VK_DOWN }, { "ArrowLeft", VK_LEFT }, { "ArrowRight", VK_RIGHT },
            { "0", 0x0030 }, { "1", 0x0031 }, { "2", 0x0032 }, { "3", 0x0033 }, { "4", 0x0034 }, { "5", 0x0035 },
            { "6", 0x0036 }, { "7", 0x0037 }, { "8", 0x0038 }, { "1", 0x0039 },
            { "a", 0x0041 }, { "b", 0x0042 }, { "c", 0x0043 }, { "d", 0x0044 }, { "e", 0x0045 }, { "f", 0x0046 },
            { "g", 0x0047 }, { "h", 0x0048 }, { "i", 0x0049 }, { "j", 0x004A }, { "k", 0x004B }, { "l", 0x004C },
            { "m", 0x004D }, { "n", 0x004E }, { "o", 0x004F }, { "p", 0x0050 }, { "q", 0x0051 }, { "r", 0x0052 },
            { "s", 0x0053 }, { "t", 0x0054 }, { "u", 0x0055 }, { "v", 0x0056 }, { "w", 0x0057 }, { "x", 0x0058 },
            { "y", 0x0059 }, { "z", 0x005A }
        };
        std::map<std::string, int> keyDownCounts { };
    public:
        void initKeyCounts() {
            for (auto entry : keymap) {
                keyDownCounts.insert_or_assign(entry.first, 0);
            }
        }
        bool getKey(std::string key) {
            return GetAsyncKeyState(keymap.at(key));
        }
        bool pressed(std::string key) {
            return getKey(key);
        }
        bool justPressed(std::string key) {
            return keyDownCounts.at(key) == 1;
        }
        void update() {
            for (auto entry : keymap) {
                if (pressed(entry.first)) keyDownCounts.at(entry.first)++;
                else keyDownCounts.at(entry.first) = 0;
            }
        }
};
class MouseHandler : public InputHandler {
    private:
    public:
        HWND WINDOW_HANDLE = GetActiveWindow();
        void init() {
            WINDOW_HANDLE = GetActiveWindow();
        }
        int x() {
            POINT mouse;
            GetCursorPos(&mouse);
            ScreenToClient(WINDOW_HANDLE, &mouse);
            return mouse.x / PIXEL_SIZE;
        }
        int y() {
            POINT mouse;
            GetCursorPos(&mouse);
            ScreenToClient(WINDOW_HANDLE, &mouse);
            return mouse.y / PIXEL_SIZE;
        }
};
class KeyboardHandler : public InputHandler {
    public:
};
KeyboardHandler Keyboard;
MouseHandler Mouse;

#endif