#include "platform/includes/thread.hpp"

// Empty implementation. We dont need threads in the wasm build.

void Thread::sleep(unsigned int duration_ms)
{
}

int Thread::create(
    TaskFunction_t pvTaskCode,
    const char * const pcName,
    Purpose purpose,
    const uint32_t usStackDepth,
    void * const pvParameters,
    int uxPriority
){
    return 0;
}

void Thread::destroy(){}