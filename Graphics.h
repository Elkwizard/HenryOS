#ifndef GRAPHICS_INCLUDE
#define GRAPHICS_INCLUDE

#ifndef SCREEN_WIDTH
#define SCREEN_WIDTH 300
#define SCREEN_HEIGHT 300
#endif

#ifndef RENDERER_TYPE
#define RENDERER_TYPE WindowsRenderer
#endif

typedef unsigned char color_channel;
class GraphicsBitmap {
    public:
        int width;
        int height;
        color_channel* imageData;
        GraphicsBitmap(int w, int h)
            : width(w), height(h) {
            imageData = new color_channel[w * h * 4];
            for (int i = 0; i < w * h * 4; i++) imageData[i] = 0;
        }
        void resize(int w, int h) {
            width = w;
            height = h;
            delete[] imageData;
            imageData = new color_channel[w * h * 4];
            for (int i = 0; i < w * h * 4; i++) imageData[i] = 0;
        }
        void set(int x, int y, color_channel r, color_channel g, color_channel b, color_channel a) {
            int inx = (x + y * width) * 4;
            imageData[inx] = r;
            imageData[inx + 1] = g;
            imageData[inx + 2] = b;
            imageData[inx + 3] = a;
        }
        color_channel* get(int x, int y) {
            int inx = (x + y * width) * 4;
            color_channel rgba[] {
                imageData[inx],
                imageData[inx + 1],
                imageData[inx + 2],
                imageData[inx + 3]
            };
            return rgba;
        }
        ~GraphicsBitmap() {
            delete[] imageData;
        }
};

//context
#include <vector>
#include <algorithm>
#include <cmath>
class GraphicsContext {
    public:
        struct Color {
            color_channel r = 0, g = 0, b = 0, a = 255;
        };
        static Color WHITE;
    private:
        struct Int2 {
            int x, y;
            static Int2& empty() {
                Int2 p { 0, 0 };
                return p;
            }
        };
        struct Int4 {
            Int2 start, end;
        };
        struct Float2 {
            float x, y;
            static Float2& empty() {
                Float2 p { 0, 0 };
                return p;
            }
        };
        struct Float4 {
            Float2 start, end;
        };
        int clipX(int x) {
            if (x < 0) return 0;
            if (x > bitmap.width - 1) return bitmap.width - 1;
            return x;
        }
        int clipY(int y) {
            if (y < 0) return 0;
            if (y > bitmap.height - 1) return bitmap.height - 1;
            return y;
        }
        Int2 clipPoint(Int2 p) {
            return { clipX(p.x), clipY(p.y) };
        }
        void pixel(int x, int y, Color color) {
            bitmap.set(x, y, color.r, color.g, color.b, color.a);
        }
        void scanline(int min, int max, int y, Color color = WHITE) {
            if (y < 0 || y > bitmap.height - 1) return;
            min = clipX(min);
            max = clipX(max);
            int range = max - min;
            for (int i = 0; i < range; i++) {
                pixel(min + i, y, color);
            }
        }
        void strokeLine(Int4 line, Color color) {
            float dx = line.end.x - line.start.x;
            float dy = line.end.y - line.start.y;
            float mag = sqrt(dx * dx + dy * dy);
            dx /= mag;
            dy /= mag;
            for (int i = 0; i < mag; i++) {
                setPixel((int)(line.start.x + dx * i), (int)(line.start.y + dy * i), color);
            }
        }
        Int2 lineStart { 0, 0 };
        Int2 transform(Int2 point) {
            Float2 p { (float)point.x, (float)point.y };
            return { (int)p.x, (int)p.y };
        }
        std::vector<Int4> path;
    public:
        GraphicsBitmap& bitmap;
        Color fillStyle;

        GraphicsContext(GraphicsBitmap& target) 
            : bitmap(target) {
            
        }
        void setPixel(int x, int y, Color color = WHITE) {
            Int2 p = clipPoint(transform({ x, y }));
            pixel(p.x, p.y, color);
        }
        void beginPath() {
            path.clear();
        }
        void moveTo(int x, int y) {
            lineStart = transform({ x, y });
        }
        void lineTo(int x, int y) {
            Int4 line { lineStart, transform({ x, y }) };
            path.push_back(line);
            lineStart = transform({ x, y });
        }
        void rect(int x, int y, int w, int h) {
            moveTo(x, y);
            lineTo(x + w, y);
            lineTo(x + w, y + h);
            lineTo(x, y + h);
            lineTo(x, y);
        }
        void fill() {
            if (path.size() <= 3) return;
            int minY = 10000;
            int maxY = -10000;
            for (Int4 line : path) {
                int y1 = line.start.y;
                int y2 = line.end.y;
                int min_ = std::min(y1, y2);
                int max_ = std::max(y1, y2);
                if (min_ < minY) minY = min_;
                if (max_ > maxY) maxY = max_;
            }
            int range = maxY - minY;
            for (int i = 0; i < range; i++) {
                int y = minY + i;
                std::vector<Int4> validLines { };

                for (Int4 line : path) {
                    int y1 = line.start.y;
                    int y2 = line.end.y;
                    int min_ = std::min(y1, y2);
                    int max_ = std::max(y1, y2);
                    if (y2 - y1 != 0) {
                        if (min_ < y && max_ > y) validLines.push_back(line);
                    }
                }
                std::vector<int> intersections { };
                for (Int4 line : validLines) {
                    int x1 = line.start.x;
                    int x2 = line.end.x;
                    int y1 = line.start.y;
                    int y2 = line.end.y;
                    int dx = x2 - x1;
                    int dy = x1 - x2;
                    if (dx == 0) {
                        intersections.push_back(x1);
                    } else {
                        float m = dy / (float)dx;
                        float b = y1 - m * x1;
                        int x = (y - b) / m;
                        intersections.push_back(x);
                    }
                }
                for (int j = 0; j < intersections.size(); j += 3) {
                    scanline(intersections[j], intersections[j + 1], y, fillStyle);
                }
            }
        }
        void clearRect(int x, int y, int w, int h) {
            Int2 min = clipPoint({ x, y });
            Int2 max = clipPoint({ x + w, y + h });
            int rangeX = max.x - min.x;
            int rangeY = max.y - min.y;
            for (int i = 0; i < rangeX; i++) for (int j = 0; j < rangeY; j++) bitmap.set(i, j, 0, 0, 0, 0);
        }
};
GraphicsContext::Color GraphicsContext::WHITE = { 255, 255, 255, 255 };

//renderers
class GraphicsRenderer {
    public:
        GraphicsBitmap& bitmap;
        GraphicsRenderer(GraphicsBitmap& target) 
            : bitmap(target) {
            
        }
        virtual void setTitle(const char* title) = 0;
        virtual bool render() = 0;
};

#include <iostream>
#include <string>
#include <stdlib.h>
#include <Windows.h>
class STDRenderer : public GraphicsRenderer {
    public:
        STDRenderer(GraphicsBitmap& target) 
            : GraphicsRenderer(target) {

        }
        void setTitle(const char* title) override {
            HWND handle = GetConsoleWindow();
            SetWindowTextA(handle, (LPCSTR)title);
        }
        bool render() override {
            system("CLS");
            std::string result = "";
            for (int i = 0; i < bitmap.height; i += 2) {
                for (int j = 0; j < bitmap.width; j++) {
                    result += (bitmap.get(j, i)[0] > 0) ? "#" : " ";
                }
                result += "\n";
            }
            std::cout << result;
            return true;
        }
};

class ConsoleRenderer : public GraphicsRenderer {
    public:
        HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
        ConsoleRenderer(GraphicsBitmap& target)
            : GraphicsRenderer(target) {

        }
        void setTitle(const char* title) override {
            HWND handle = GetConsoleWindow();
            SetWindowTextA(handle, (LPCSTR)title);
        }
        bool render() override {
            auto sortCols = [=](std::pair<float, short> a, std::pair<float, short> b) {
                return a.first > b.first;
            };
            auto rgb = [=](float r, float g, float b) {
                std::vector<std::pair<float, short>> cols { { r, 0x0004 }, { g, 0x0002 }, { b, 0x0001 } };
                std::sort(cols.begin(), cols.end(), sortCols);
                std::pair<float, short> pri = cols[0], sec = cols[1];
                short A = sec.second;
                short B = pri.second;
                float total = pri.first + sec.first;
                float f = pri.first / total;
                if (sec.first < 10.0f && pri.first > 30.0f) {
                    f = 0.0f;
                    A = 0x0000;
                }
                if (pri.first < 10.0f) {
                    std::pair<std::pair<short, short>, char> result { { (short)0x0000, (short)0x0000 }, (char)219 };
                    return result;
                }
                short a_ = A;
                short b_ = A | B;
                float t = 1 - 2 * (f - 0.5);
                float seg = 1.0f / 7.0f;
                short color = 0x0004 | 0x0002 | 0x0001;
                short background = 0x0004 | 0x0002 | 0x0001;
                char symbol = 219;
                if (t < 2 * seg) {
                    color = a_;
                    background = a_;
                } else if (t < 4 * seg) {
                    color = a_;
                    background = b_;
                    symbol = 178;
                } else if (t < 6 * seg) {
                    color = a_;
                    background = b_;
                    symbol = 177;
                } else {
                    color = b_;
                    background = b_;
                }
                if (pri.first > 128) {
                    color |= 0x0008;
                    background |= 0x0008;
                }
                if (f > 0.8) {
                    color = B;
                    background = B;
                }
                std::pair<std::pair<short, short>, char> finalColor { { color, background }, symbol }; 
                return finalColor;
            };
            COORD bufferSize { (short)bitmap.width, (short)bitmap.height };
            COORD bufferStart { (short)0, (short)0 };
            SMALL_RECT write { bufferStart.X, bufferStart.Y, bufferSize.X, bufferSize.Y };
            std::vector<CHAR_INFO> chars { };
            for (int i = 0; i < bufferSize.X * bufferSize.Y; i++) chars.push_back({ });
            for (int j = 0; j < bitmap.width; j++) for (int i = 0; i < bitmap.height; i++) {
                color_channel* pixel = bitmap.get(i, j * 2);
                if (j < bitmap.width - 1) {
                    color_channel* pixel_below = bitmap.get(i, j * 2 + 1);
                    int bright = (pixel[0] + pixel[1] + pixel[2] + pixel[3]) / 4;
                    int bright_below = (pixel_below[0] + pixel_below[1] + pixel_below[2] + pixel_below[3]) / 4;
                    if (bright_below > bright) {
                        pixel[0] = pixel_below[0];
                        pixel[1] = pixel_below[1];
                        pixel[2] = pixel_below[2];
                        pixel[3] = pixel_below[3];
                    }
                }
                CHAR_INFO ch { };

                std::pair<std::pair<short, short>, char> color = rgb((float)pixel[0], (float)pixel[1], (float)pixel[2]);
                
                ch.Char.AsciiChar = color.second;
                ch.Char.UnicodeChar = color.second;
                ch.Attributes = color.first.first | color.first.second;
                int inx = i + (j * bufferSize.X);
                chars[inx] = ch;
            }
            WriteConsoleOutput(consoleHandle, &chars[0], bufferSize, bufferStart, &write);
            return true;
        }
};

#pragma comment(lib, "User32.lib")
#pragma comment(lib, "Gdi32.lib")

namespace WindowData {
    RECT screen { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };
    bool windowClosed = false;
    LRESULT CALLBACK windowProcedure(HWND handle, UINT message, WPARAM wParam, LPARAM lParam) {
        switch (message) {
            case WM_DESTROY:
                PostQuitMessage(0);
                windowClosed = true;
                break;
        }
        return DefWindowProc(handle, message, wParam, lParam);
    };
    HWND createWindow(LPCSTR name) {
        HINSTANCE instanceHandle = GetModuleHandle(NULL);
        //create window
        WNDCLASS windowClass { };
        windowClass.lpfnWndProc = WindowData::windowProcedure;
        windowClass.hInstance = instanceHandle;
        windowClass.lpszClassName = (LPCSTR)"GraphicsWindow";

        RegisterClass(&windowClass);

        HWND myWindow = CreateWindowEx(
            0,
            windowClass.lpszClassName,
            name,
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
            NULL,
            NULL,
            instanceHandle,
            NULL
        );
        
        ShowWindow(myWindow, 1);
        
        return myWindow;
    }
    bool runWindow(HWND windowHandle) {
        MSG msg { };
        if (GetMessage(&msg, NULL, 0, 0)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        return !windowClosed;
    }
}
class WindowsRenderer : public GraphicsRenderer {
    private:
        HWND windowHandle;
        // int frame = 0;
    public:
        WindowsRenderer(GraphicsBitmap& target)
            : GraphicsRenderer(target) {
            windowHandle = WindowData::createWindow("Graphical Window");
            int windowX = 0;
            int windowY = 0;
            int windowWidth = WindowData::screen.right + 10;
            int windowHeight = WindowData::screen.bottom + 38;
            HWND fullScreen = GetDesktopWindow();
            RECT rect { };
            GetWindowRect(fullScreen, &rect);
            windowX = (rect.right - windowWidth) / 2;
            windowY = (rect.bottom - windowHeight) / 2;
            MoveWindow(windowHandle, windowX, windowY, windowWidth, windowHeight, true);
            SetTimer(windowHandle, NULL, 10, NULL);
        }
        void setTitle(const char* title) override {
            SetWindowTextA(windowHandle, (LPCSTR)title);
        }
        bool render() override {

            RedrawWindow(windowHandle, NULL, NULL, RDW_INTERNALPAINT | RDW_INVALIDATE | RDW_ERASE);
            
            PAINTSTRUCT painter;
            HDC deviceContext = BeginPaint(windowHandle, &painter);

            // frame++;
            // RECT rect { frame, 0, frame + 100, 100 };
            // FillRect(deviceContext, &rect, CreateSolidBrush(RGB(255, 0, frame)));

            //create image data array
            int len = bitmap.width * bitmap.height * 4;
            BYTE* bmprgb = new BYTE[len];
            for (int i = 0; i < bitmap.width; i++) for (int j = 0; j < bitmap.height; j++) {
                int inx = (i + j * bitmap.width) * 4;
                color_channel* rgb = bitmap.get(i, j);
                bmprgb[inx] = rgb[2];
                bmprgb[inx + 1] = rgb[1];
                bmprgb[inx + 2] = rgb[0];
                bmprgb[inx + 3] = rgb[3];
            }

            //make bitmap
            HBITMAP bmp = CreateBitmap(bitmap.width, bitmap.height, 1, 4*8, (VOID*)bmprgb);
            HDC src = CreateCompatibleDC(deviceContext);
            SelectObject(src, bmp);

            //move to screen
            BitBlt(deviceContext, 0, 0, bitmap.width, bitmap.height, src, 0, 0, SRCCOPY);

            //deallocate
            DeleteObject(bmp);
            DeleteDC(src);
            delete[] bmprgb;

            EndPaint(windowHandle, &painter);

            return WindowData::runWindow(windowHandle);
        }
};
//program types
#include "Program.h"
class GraphicsProgramBase : public ProgramBase {
    public:
        GraphicsBitmap& screen = GraphicsBitmap(SCREEN_WIDTH, SCREEN_HEIGHT);
        RENDERER_TYPE& renderer = RENDERER_TYPE(screen);
        GraphicsProgramBase() {
            
        }
        void internalInit() override {
            init();
        }
        bool internalUpdate(float delta) override {
            update(delta);
            return renderer.render();
        }
};

#endif