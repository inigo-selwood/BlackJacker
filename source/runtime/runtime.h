#ifndef BLACKJACKER_RUNTIME_H
#define BLACKJACKER_RUNTIME_H

#include <stdbool.h>

#include "modes/play/state.h"
#include "table/table.h"

/*******************************************************************************
 * Runtime_State
 ******************************************************************************/

/** Application modes registered with the runtime loop. */
typedef enum {
    MODE_NOTICE,
    MODE_MENU,
    MODE_SETTINGS,
    MODE_PLAY,
    MODE_COUNT
} Runtime_Mode;

/** Mutable runtime state for mode, deferred transitions, and focus. */
typedef struct {
    Runtime_Mode mode;
    Runtime_Mode queuedMode;
    bool hasQueuedMode;
    bool running;
    int modeFocus[MODE_COUNT];
} Runtime_State;

/** Creates the initial application state. */
Runtime_State Runtime_initState(void);

/** Queues a mode transition to be applied on the next loop cycle. */
void Runtime_queueState(Runtime_State *state, Runtime_Mode mode);

/** Applies any queued mode transition. */
void Runtime_applyQueuedState(Runtime_State *state);

/*******************************************************************************
 * Runtime_Game
 ******************************************************************************/

/** Root game model passed through mode callbacks. */
typedef struct {
    Runtime_State state;
    PlaySettings playSettings;
    Table_Shoe shoe;
    PlayRound playRound;
} Runtime_Game;

/** Creates a game with default state and play settings. */
Runtime_Game Runtime_initGame(void);

/*******************************************************************************
 * Settings
 ******************************************************************************/

/** Returns the built-in play settings used when no YAML file exists. */
PlaySettings Runtime_defaultPlaySettings(void);

/** Loads play settings from the runtime YAML file into an existing settings
 * struct. */
bool Runtime_loadSettings(PlaySettings *settings);

/** Saves play settings to the runtime YAML file. */
bool Runtime_saveSettings(const PlaySettings *settings);

/** Applies play settings to a game and persists them. */
bool Runtime_updateSettings(Runtime_Game *game, PlaySettings settings);

/*******************************************************************************
 * Runtime_Mode Registry
 ******************************************************************************/

/** Lifecycle event type delivered to a mode callback. */
typedef enum {
    MODE_EVENT_RENDER,
    MODE_EVENT_INPUT,
} Runtime_ModeEventType;

/** Per-frame event delivered to the active mode. */
typedef struct {
    Runtime_ModeEventType type;
    int input;
} Runtime_ModeEvent;

/** Callback signature for mode rendering and input handling. */
typedef void (*Runtime_ModeCallback)(
    Runtime_Game *game,
    Runtime_ModeEvent event
);

/** Clears all registered mode callbacks. */
void Runtime_clearStateRegistry(void);

/** Registers a callback for a mode. */
void Runtime_registerState(Runtime_ModeCallback callback, Runtime_Mode mode);

/** Returns the callback registered for a mode, or null. */
Runtime_ModeCallback Runtime_stateCallback(Runtime_Mode mode);

/*******************************************************************************
 * Run Loop
 ******************************************************************************/

/** Runs the main game loop until the game state requests shutdown. */
void Runtime_runGame(Runtime_Game *game);

#endif
