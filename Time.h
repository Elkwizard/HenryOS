#ifndef TIME_INCLUDE
#define TIME_INCLUDE

#include <chrono>
#include <thread>

class Time {
    public:
        static float now() {
            return std::chrono::time_point_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now()).time_since_epoch().count() / 1000.0f;
        }
        class Timer {
            public:
                float startTime;
                Timer() {}
                void start() {
                    startTime = Time::now();
                }
                float end() {
                    return Time::now() - startTime;
                }
        };
        static void sleep(int ms) {
            std::this_thread::sleep_for(std::chrono::milliseconds(ms));
        }
};

#endif