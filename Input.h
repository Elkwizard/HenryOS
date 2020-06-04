#ifndef INPUT_DEFINED
#define INPUT_DEFINED

#include <map>
#include <string>
#include "ScreenSetup.h"
#include "WindowsImport.h"
class InputHandler {
    private:
        std::map<std::string, short> keymap;
        std::map<std::string, int> keyDownCounts;
    public:
        bool getKey(std::string key) {
            return GetAsyncKeyState(keymap.at(key));
        }
        bool pressed(std::string key) {
            // return getKey(key);
            return true;
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
        std::map<std::string, short> keymap {
            { "Left", VK_LBUTTON },
            { "Middle", VK_MBUTTON },
            { "Right", VK_RBUTTON }
        };
        std::map<std::string, int> keyDownCounts {
            { "Left", 0 },
            { "Middle", 0 },
            { "Right", 0 }
        };
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