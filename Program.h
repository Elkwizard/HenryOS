#ifndef PROGRAM_INCLUDE
#define PROGRAM_INCLUDE

#include "Time.h"
class ProgramBase {
    public:
        virtual void init() = 0;
        virtual void update(float t = 16.666f) = 0;
        virtual void internalInit() = 0;
        virtual bool internalUpdate(float t = 16.666f) = 0;
        static void run(ProgramBase& program) {
            program.internalInit();
            float delta = 16.666f;
            while (true) {
                Time::Timer t;
                t.start();
                if (!program.internalUpdate(delta)) break;
                delta = t.end();
                Time::sleep(16);
            }
        }
};
class GeneralProgramBase : public ProgramBase {
    public:
        void internalInit() override {
            init();
        }
        bool internalUpdate(float delta) override {
            update(delta);
            return true;
        }
};

#endif