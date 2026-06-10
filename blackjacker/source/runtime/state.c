#include "runtime/runtime.h"

Runtime_State Runtime_initState(void) {
    const Runtime_State state = {MODE_MENU, MODE_MENU, false, true, {0}};
    return state;
}

void Runtime_queueState(Runtime_State *state, Runtime_Mode mode) {
    state->queuedMode = mode;
    state->hasQueuedMode = true;
}

void Runtime_applyQueuedState(Runtime_State *state) {
    if(!state->hasQueuedMode) {
        return;
    }

    state->mode = state->queuedMode;
    state->hasQueuedMode = false;
}
