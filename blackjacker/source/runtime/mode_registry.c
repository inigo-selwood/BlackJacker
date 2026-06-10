#include "runtime/runtime.h"

static Runtime_ModeCallback callbacks[MODE_COUNT];

void Runtime_clearStateRegistry(void) {
    for(int mode = 0; mode < MODE_COUNT; mode += 1) {
        callbacks[mode] = 0;
    }
}

void Runtime_registerState(Runtime_ModeCallback callback, Runtime_Mode mode) {
    if(mode < 0 || mode >= MODE_COUNT) {
        return;
    }

    callbacks[mode] = callback;
}

Runtime_ModeCallback Runtime_stateCallback(Runtime_Mode mode) {
    if(mode < 0 || mode >= MODE_COUNT) {
        return 0;
    }

    return callbacks[mode];
}
