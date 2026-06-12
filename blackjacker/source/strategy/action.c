#include "strategy.h"

const char *strategyActionLabel(Strategy_Action action) {
    switch(action) {
    case STRATEGY_ACTION_STAND:
        return "Stand";
    case STRATEGY_ACTION_DOUBLE:
        return "Double";
    case STRATEGY_ACTION_SPLIT:
        return "Split";
    case STRATEGY_ACTION_SURRENDER:
        return "Surrender";
    case STRATEGY_ACTION_HIT:
    default:
        return "Hit";
    }
}

const char *Strategy_actionCode(Strategy_Action action) {
    switch(action) {
    case STRATEGY_ACTION_STAND:
        return "S";
    case STRATEGY_ACTION_DOUBLE:
        return "D";
    case STRATEGY_ACTION_SPLIT:
        return "P";
    case STRATEGY_ACTION_SURRENDER:
        return "R";
    case STRATEGY_ACTION_HIT:
    default:
        return "H";
    }
}
