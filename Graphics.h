#ifndef GRAPHICS_INCLUDE
#define GRAPHICS_INCLUDE

#ifndef SCREEN_WIDTH
#define SCREEN_WIDTH 300
#define SCREEN_HEIGHT 300
#endif



#include "Utils.h"
#include "WindowsRenderer.h"
#include "Program.h"
#include <string>
void transformGraphicsProgramRender(void* ctx);

class GraphicsProgramBase : public ProgramBase {
    public:
        float delta = 16.666f; 
        GraphicsProgramBase() {
            Window::create("Graphics Program", SCREEN_WIDTH, SCREEN_HEIGHT);
            Window::pRequestRender(&transformGraphicsProgramRender, this); 
        }
        void setTitle(std::string title) {
            Window::setTitle(title);
        }
        void internalInit() override {
            init();
        }
        void render() {
            update(delta);
        }
        bool internalUpdate(float _delta) override {
            delta = _delta;
            return Window::run();
        }
};

void transformGraphicsProgramRender(void* ctx) {
    GraphicsProgramBase* gpb = (GraphicsProgramBase*)ctx;
    gpb->update(gpb->delta);
};

#endif