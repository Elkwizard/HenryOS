#ifndef GRAPHICS_INCLUDE
#define GRAPHICS_INCLUDE

#ifndef SCREEN_WIDTH
#define SCREEN_WIDTH 300
#define SCREEN_HEIGHT 300
#endif

#ifndef RENDERER_TYPE
#define RENDERER_TYPE WindowsRenderer
#endif


#include "Utils.h"

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
#include <string>
#include <iostream>
#define CIRCLE_PRECISION 10
class GraphicsContext {
    public:
        struct Color {
            color_channel r = 0, g = 0, b = 0, a = 255;
        };
        static Color WHITE;
        Color fillStyle { 0, 0, 0 };
        Color strokeStyle { 0, 0, 0 };
        int lineWidth = 1;
        float globalAlpha = 1;
        std::string lineCap = "flat";
    private:
        struct Int2 {
            int x, y;

            Int2(int X, int Y) {
                x = X;
                y = Y;
            }
            Int2(int v) {
                x = v;
                y = v;
            }
            static Int2& empty() {
                Int2 p { 0, 0 };
                return p;
            }
            static float dist(Int2& a, Int2& b) {
                float dx = b.x - a.x;
                float dy = b.y - a.y;
                return sqrt(dx * dx + dy * dy);
            }
            Int2 operator+(Int2 b) {
                return { x + b.x, y + b.y };
            }
            Int2 operator-(Int2 b) {
                return { x - b.x, y - b.y };
            }
            Int2 operator*(Int2 b) {
                return { x * b.x, y * b.y };
            }
            Int2 operator/(Int2 b) {
                return { x / b.x, y / b.y };
            }
        };
        struct Int4 {
            Int2 start, end;
        };
        struct Float2 {
            float x, y;
            
            Float2(float X, float Y) {
                x = X;
                y = Y;
            }
            Float2(float v) {
                x = v;
                y = v;
            }
            static Float2& empty() {
                Float2 p { 0.0f, 0.0f };
                return p;
            }
            static float dist(Float2& a, Float2& b) {
                float dx = b.x - a.x;
                float dy = b.y - a.y;
                return sqrt(dx * dx + dy * dy);
            }
            float length() {
                return sqrt(x * x + y * y);
            }
            Float2 operator+(Float2 b) {
                return { x + b.x, y + b.y };
            }
            Float2 operator-(Float2 b) {
                return { x - b.x, y - b.y };
            }
            Float2 operator*(Float2 b) {
                return { x * b.x, y * b.y };
            }
            Float2 operator/(Float2 b) {
                return { x / b.x, y / b.y };
            }
            operator Int2() {
                return { (int)x, (int)y };
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
        bool invalid(int x, int y) {
            return x < 0 || y < 0 || x > bitmap.width - 1 || y > bitmap.height - 1;
        }
        void directPixel(int x, int y, Color color) {
            bitmap.set(x, y, color.r, color.g, color.b, color.a);
        }
        void pixel(int x, int y, Color color) {
            color_channel* current = bitmap.get(x, y);
            if (current[3] < 10) directPixel(x, y, color);
            float a = globalAlpha * color.a / 255.0f;
            float a1 = 1.0f - a;
            // std::cout << (int)color.r << "," << (int)color.g << "," << (int)color.b << "," << (int)color.a << std::endl;
            bitmap.set(x, y, color.r * a + current[0] * a1, color.g * a + current[1] * a1, color.b * a + current[2] * a1, color.a * a + current[3] * a1);
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
        void strokeLine(Int4 line, Color color, int lineWidth, std::string lineCap) {
            float dx = line.end.x - line.start.x;
            float dy = line.end.y - line.start.y;
            float mag = sqrt(dx * dx + dy * dy);
            dx /= mag;
            dy /= mag;
            float n_x = -dy;
            float n_y = dx;
            for (int l = 0; l < lineWidth * 2; l++) for (int i = 0; i < mag; i++) {
                int L = l - lineWidth;
                float ox = n_x * L / 2.0f;
                float oy = n_y * L / 2.0f;
                Int2 p = { (int)(line.start.x + dx * i + ox), (int)(line.start.y + dy * i + oy) };
                if (invalid(p.x, p.y)) continue;
                pixel(p.x, p.y, color);
            }
            if (lineCap == "flat") return;
            if (lineCap == "round") {
                dx *= mag;
                dy *= mag;
                int radius = lineWidth / 2;
                for (int i = 0; i < 2; i++) for (int x = 0; x < lineWidth; x++) for (int y = 0; y < lineWidth; y++) {
                    int ax = line.start.x + dx * i;
                    int ay = line.start.y + dy * i;
                    int X = x - lineWidth / 2;
                    int Y = y - lineWidth / 2;
                    if (X * X + Y * Y < radius * radius) {
                        Int2 p = { ax + X, ay + Y };
                        if (invalid(p.x, p.y)) continue;
                        pixel(p.x, p.y, color);
                    }
                }
            }
        }
        Int2 lineStart { 0, 0 };
        std::vector<Int4> path;
        enum TransformationType { ROTATION, TRANSLATION, SCALE };
        struct Transformation {
            TransformationType type;
            float x, y;
        };
        std::vector<std::vector<Transformation>> transformationSaveStack;
        std::vector<Transformation> transformationStack;
        Int2 transform(Int2 point) {
            Float2 p { (float)point.x, (float)point.y };
            for (int i = transformationStack.size() - 1; i >= 0; i--) {
                Transformation transformation = transformationStack[i];
                float t_x = p.x, t_y = p.y;
                float arg_x = transformation.x, arg_y = transformation.y;

                switch (transformation.type) {
                    case ROTATION:
                        t_x = p.x * arg_x - p.y * arg_y;
                        t_y = p.x * arg_y + p.y * arg_x;
                        break;
                    case TRANSLATION:
                        t_x = p.x + arg_x;
                        t_y = p.y + arg_y;
                        break;
                    case SCALE:
                        t_x = p.x * arg_x;
                        t_y = p.y * arg_y;
                        break;
                }

                p.x = t_x;
                p.y = t_y;
            }
            return { (int)p.x, (int)p.y };
        }
        float lineWidthFactor = 1;
    public:
        GraphicsBitmap& bitmap;

        GraphicsContext(GraphicsBitmap& target) 
            : bitmap(target) {
            
        }
        void save() {
            transformationSaveStack.push_back(transformationStack);
        }
        void restore() {
            if (transformationSaveStack.size() > 0) {
                std::vector<Transformation> nStack = transformationSaveStack[transformationSaveStack.size() - 1];
                transformationSaveStack.pop_back();
                transformationStack = nStack;
            }
        }
        void translate(int x, int y) {
            transformationStack.push_back({ TRANSLATION, (float)x, (float)y });
        }
        void scale(int x, int y) {
            lineWidthFactor *= x;
            transformationStack.push_back({ SCALE, (float)x, (float)y });
        }
        void rotate(float angle) {
            transformationStack.push_back({ ROTATION, cos(angle), sin(angle) });
        }
        void setPixel(int x, int y, Color color = WHITE) {
            Int2 p = transform({ x, y });
            if (invalid(p.x, p.y)) return;
            directPixel(p.x, p.y, color);
        }
        void drawImage(GraphicsBitmap& image, int x, int y, int w, int h) {
            Int2 start = transform({ x, y });
            Int2 start2 = transform({ x + w, y });
            Int2 end = transform({ x, y + h });
            Int2 end2 = transform({ x + w, y + h });
            Float2 X_AXIS = { (float)(start2.x - start.x), (float)(start2.y - start.y) };
            Float2 Y_AXIS = { (float)(end.x - start.x), (float)(end.y - start.y) };
            float XLN = X_AXIS.length();
            float YLN = end.y - start.y;//Y_AXIS.length();
            X_AXIS = X_AXIS / -XLN;
            Y_AXIS = Y_AXIS / YLN;
            float X_START = X_AXIS.x * start.x + X_AXIS.y * start.y;
            float Y_START = Y_AXIS.x * start.x + Y_AXIS.y * start.y;
            //scanline fill
            beginPath();
            rect(x, y, w, h);
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
                        if (min_ <= y && max_ > y) validLines.push_back(line);
                    }
                }
                std::vector<int> intersections { };
                for (Int4 line : validLines) {
                    int x1 = line.start.x;
                    int x2 = line.end.x;
                    int y1 = line.start.y;
                    int y2 = line.end.y;
                    int dx = x2 - x1;
                    int dy = y2 - y1;
                    if (dx == 0) {
                        intersections.push_back(x1);
                    } else {
                        float m = dy / (float)dx;
                        int b = y1 - m * x1;
                        int x = (y - b) / m;
                        if (intersections.size() == 0 || intersections[intersections.size() - 1] != x) intersections.push_back(x);
                    }
                }
                if (intersections.size() >= 2) {
                    std::sort(intersections.begin(), intersections.end(), [](int a, int b) {
                        return a < b;
                    });
                    for (int j = 0; j < intersections.size(); j += 3) {
                        if (y < 0 || y > bitmap.height - 1) continue;
                        int minX = clipX(intersections[j]);
                        int maxX = clipX(intersections[j + 1]);
                        int range = maxX - minX;
                        for (int X = 0; X < range; X++) {
                            Int2 p { minX + X, y };
                            float x_dot = (p.x * X_AXIS.x + p.y * X_AXIS.y - X_START) / XLN;//(p.x * X_AXIS.x + p.y * X_AXIS.y) / XLN;
                            float y_dot = (p.x * Y_AXIS.x + p.y * Y_AXIS.y - Y_START) / YLN;//(p.x * Y_AXIS.x + p.y * Y_AXIS.y) / YLN;
                            color_channel* ch = image.get(x_dot * image.width, std::min((int)(image.height - 1), (int)(y_dot * image.height + 1)));
                            Color col { ch[0], ch[1], ch[2], ch[3] };
                            pixel(p.x, p.y, col);
                        }
                    }
                }
            }
            closePath();
        }
        Color getPixel(int x, int y) {
            color_channel* col = bitmap.get(x, y);
            return { col[0], col[1], col[2], col[3] };
        }
        void beginPath() {
            path.clear();
        }
        void closePath() {
            beginPath();
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
        void arc(int x, int y, int radius, float startAngle, float endAngle) {
            if (abs(endAngle - startAngle) < 0.001) return;
            std::vector<Int2> points { };
            int dif = radius * (endAngle - startAngle) / CIRCLE_PRECISION;
            for (int i = 0; i < dif; i++) {
                float angle = CIRCLE_PRECISION * i / (float)radius + startAngle;
                points.push_back({ (int)(cos(angle) * radius) + x, (int)(sin(angle) * radius) + y });
            }
            Int2 start = points[0];
            moveTo(start.x, start.y);
            for (int i = 0; i < points.size(); i++) {
                Int2 point = points[i];
                lineTo(point.x, point.y);
            }
            lineTo(start.x, start.y);
        }
        void fill() {
            if (path.size() < 3) return;
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
                        if (min_ <= y && max_ > y) validLines.push_back(line);
                    }
                }
                std::vector<int> intersections { };
                for (Int4 line : validLines) {
                    int x1 = line.start.x;
                    int x2 = line.end.x;
                    int y1 = line.start.y;
                    int y2 = line.end.y;
                    int dx = x2 - x1;
                    int dy = y2 - y1;
                    if (dx == 0) {
                        intersections.push_back(x1);
                    } else {
                        float m = dy / (float)dx;
                        int b = y1 - m * x1;
                        int x = (y - b) / m;
                        if (intersections.size() == 0 || intersections[intersections.size() - 1] != x) intersections.push_back(x);
                    }
                }
                if (intersections.size() >= 2) {
                    std::sort(intersections.begin(), intersections.end(), [](int a, int b) {
                        return a < b;
                    });
                    for (int j = 0; j < intersections.size(); j += 3) {
                        scanline(intersections[j], intersections[j + 1], y, fillStyle);
                    }
                }
            }
            closePath();
        }
        void stroke() {
            if (path.size() == 0) return;
            for (Int4 line : path) {
                strokeLine(line, strokeStyle, lineWidth * lineWidthFactor, lineCap);
            }
            closePath();
        }
        void strokeRect(int x, int y, int w, int h) {
            beginPath();
            rect(x, y, w, h);
            stroke();
        }
        void fillRect(int x, int y, int w, int h) {
            beginPath();
            rect(x, y, w, h);
            fill();
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
#include <Windows.h>
class GraphicsRenderer {
    public:
        GraphicsBitmap& bitmap;
        GraphicsRenderer(GraphicsBitmap& target) 
            : bitmap(target) {
            
        }
        virtual HWND getWindowHandle() = 0;
        virtual bool render() = 0;
};

#include <iostream>
#include <stdlib.h>
class STDRenderer : public GraphicsRenderer {
    public:
        STDRenderer(GraphicsBitmap& target) 
            : GraphicsRenderer(target) {

        }
        HWND getWindowHandle() override {
            return GetConsoleWindow();
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
        HWND getWindowHandle() override {
            return GetConsoleWindow();
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
            case WM_SIZING:
                return 0;
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
            HWND consoleWindow = GetConsoleWindow();
            // MoveWindow(consoleWindow, 0, 0, 0, 0, true);
        }
        HWND getWindowHandle() {
            return windowHandle;
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
#include <string>
class GraphicsProgramBase : public ProgramBase {
    public:
        GraphicsBitmap& screen = GraphicsBitmap(SCREEN_WIDTH, SCREEN_HEIGHT);
        RENDERER_TYPE& renderer = RENDERER_TYPE(screen);
        GraphicsProgramBase() {
            
        }
        int mouseX() {
            POINT mouse;
            GetCursorPos(&mouse);
            ScreenToClient(renderer.getWindowHandle(), &mouse);
            return mouse.x;
        }
        int mouseY() {
            POINT mouse;
            GetCursorPos(&mouse);
            ScreenToClient(renderer.getWindowHandle(), &mouse);
            return mouse.y;
        }
        void setTitle(std::string title) {
            SetWindowTextA(renderer.getWindowHandle(), (LPCSTR)title.c_str());
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