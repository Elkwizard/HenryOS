#ifndef WINDOWS_RENDERER_INCLUDED
#define WINDOWS_RENDERER_INCLUDED

#include <windows.h>
#include <gdiplus.h>
#include <vector>
#pragma comment(lib, "User32.lib")
#pragma comment(lib, "Gdi32.lib")
#pragma comment(lib, "Gdiplus.lib")

namespace Window {

    class Renderer {
        public:
            struct Point {
                int x, y;
            };
            struct Color {
                int r = 0, g = 0, b = 0, a = 255;
            };
            struct Image {
                const wchar_t* src = L"";
                Gdiplus::Bitmap bmp;
                Image(const wchar_t* _src)
                    : src(_src), bmp({ _src }) { }
            };
            Gdiplus::Graphics* g;
            Gdiplus::Pen pen = { Gdiplus::Color(0, 0, 0) };
            Gdiplus::SolidBrush brush = { Gdiplus::Color(0, 0, 0) };
            Renderer(Gdiplus::Graphics* _g)
            : g(_g) { }  
            Renderer() { }
            Renderer(const Renderer& r) {
                g = r.g;
            }
            void drawImage(Image& img, int x, int y, int w = 0, int h = 0) {
                if (w == 0) w = img.bmp.GetWidth();
                if (h == 0) h = img.bmp.GetHeight();
                Gdiplus::Rect r { x, y, w, h };
                g->DrawImage(&img.bmp, r);
            }
            void fillStyle(Color r) {
                brush.SetColor(Gdiplus::Color(r.a, r.r, r.g, r.b));
            }
            void strokeStyle(Color r) {
                pen.SetColor(Gdiplus::Color(r.a, r.r, r.g, r.b));
            }
            void lineWidth(int width) {
                pen.SetWidth(width);
            }
            void fillRect(int x, int y, int w, int h) {
                Gdiplus::Rect r { x, y, w, h };
                g->FillRectangle(&brush, r);
            }
            void strokeRect(int x, int y, int w, int h) {
                Gdiplus::Rect r { x, y, w, h };
                g->DrawRectangle(&pen, r);
            }
            float deg(float radians) {
                return 360.0f * radians / 3.14159f;
            }
            void strokeArc(int x, int y, int r, float s, float e) {
                g->DrawArc(&pen, x - r, y - r, r * 2, r * 2, deg(s), deg(e));
            }
            void fillArc(int x, int y, int r, float s, float e) {
                g->FillPie(&brush, x - r, y - r, r * 2, r * 2, deg(s), deg(e));
            }
            void strokeEllipse(int x, int y, int rx, int ry) {
                g->DrawEllipse(&pen, x - rx, y - ry, rx * 2, ry * 2);
            }
            void fillEllipse(int x, int y, int rx, int ry) {
                g->FillEllipse(&brush, x - rx, y - ry, rx * 2, ry * 2);
            }
            void strokeLine(int x, int y, int x1, int y1) {
                g->DrawLine(&pen, x, y, x1, y1);
            }
            void strokePolygon(std::vector<Point> verts) {
                Gdiplus::Point* points = new Gdiplus::Point[verts.size()];
                for (int i = 0; i < verts.size(); i++) {
                    points[i].X = verts[i].x;
                    points[i].Y = verts[i].y;
                }
                g->DrawPolygon(&pen, points, verts.size());
                delete[] points;
            }
            void fillPolygon(std::vector<Point> verts) {
                Gdiplus::Point* points = new Gdiplus::Point[verts.size()];
                for (int i = 0; i < verts.size(); i++) {
                    points[i].X = verts[i].x;
                    points[i].Y = verts[i].y;
                }
                g->FillPolygon(&brush, points, verts.size());
                delete[] points;
            }
    };

    bool closed = false;
    HWND handle;
    int windowFrame = 0;
    Gdiplus::GdiplusStartupInput gdiIn;
    ULONG_PTR gdiToken;
    int width, height;
    Renderer renderer;

    typedef void (*RenderRequest) ();

    std::vector<RenderRequest> requests;

    void requestRender(RenderRequest r) {
        requests.push_back(r);
    }
    bool render() {
        for (RenderRequest r : requests) {
            r();
        }
        if (requests.size() == 0) return false;
        requests.clear();
        return true;
    }

    LRESULT CALLBACK windowProcedure(HWND handle, UINT message, WPARAM wParam, LPARAM lParam) {
        switch (message) {
            case WM_ERASEBKGND:
                return true;
            case WM_TIMER:
                InvalidateRect(handle, NULL, false);
                break;
            case WM_PAINT: {
                    PAINTSTRUCT ctx;
                    HDC device = BeginPaint(handle, &ctx);
                    HDC frameDC = CreateCompatibleDC(device);
                    HBITMAP frame = CreateCompatibleBitmap(device, width, height);
                    SelectObject(frameDC, frame);
                    
                    Gdiplus::Graphics g (frameDC);
                    renderer.g = &g;
                    bool rendered = render();

                    if (rendered) BitBlt(device, 0, 0, width, height, frameDC, 0, 0, SRCCOPY);
                    EndPaint(handle, &ctx);
                    
                    DeleteObject(frame);
                    ReleaseDC(handle, device);
                    ReleaseDC(handle, frameDC);
                }
                break;
            case WM_DESTROY:
                PostQuitMessage(0);
                closed = true;
                break;
        }
        return DefWindowProc(handle, message, wParam, lParam);
    };


    HWND create(LPCSTR name, int w, int h) {
        HINSTANCE instanceHandle = GetModuleHandle(NULL);
        //create window
        WNDCLASS windowClass { };
        windowClass.lpfnWndProc = windowProcedure;
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
        handle = myWindow;

        Gdiplus::GdiplusStartup(&gdiToken, &gdiIn, NULL);

        
        ShowWindow(myWindow, 1);
        MoveWindow(GetConsoleWindow(), 0, 0, 0, 0, false);

        HWND desktopHandle = GetDesktopWindow();
        RECT desktop;
        GetWindowRect(desktopHandle, &desktop);
        int dw = desktop.right;
        int dh = desktop.bottom;
        int x = (dw - w) / 2;
        int y = (dh - h) / 2;
        MoveWindow(handle, x, y, w, h, true);
        width = w;
        height = h;
        //keep updating
        SetTimer(handle, NULL, 1000 / 60, NULL);
        return handle;
    }
    bool run() {
        windowFrame++;

        MSG msg { };
        if (GetMessage(&msg, NULL, 0, 0)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        if (closed) Gdiplus::GdiplusShutdown(gdiToken);
        return !closed;
    }
}

#endif