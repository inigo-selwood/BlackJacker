#include "controls.h"

#include "runtime/runtime.h"

static void backToMenu(Runtime_Game *game) {
    (void)Runtime_updateSettings(game, game->playSettings);
    Runtime_queueState(&game->state, MODE_MENU);
}

Graphics_ControlGroup settingsControls(
    Runtime_Game *game,
    Graphics_Box optionsBox,
    Graphics_Box backBox
) {
    static const char *dealerSoft17Choices[] = {
        "Hit Soft 17",
        "Stand Soft 17",
    };
    static const char *blackjackPayoutChoices[] = {
        "3:2",
        "6:5",
    };
    static const char *doubleRuleChoices[] = {
        "Any Two",
        "9/10/11",
        "10/11",
    };
    PlaySettings *settings = &game->playSettings;
    static Graphics_Control controls[15];

    controls[0] = (Graphics_Control){
        .label = "Decks",
        .info = "Number of decks in the shoe; more decks make counts move "
                "more slowly.",
        .labelWidth = 20,
        .type = GRAPHICS_CONTROL_INT_INPUT,
        .data.integer = {&settings->deckCount, 1, 8, 1},
    };
    controls[1] = (Graphics_Control){
        .label = "Cut Card",
        .info = "Percentage of the shoe dealt before reshuffling.",
        .labelWidth = 20,
        .type = GRAPHICS_CONTROL_INT_INPUT,
        .data.integer = {&settings->cutPercent, 50, 95, 5},
    };
    controls[2] = (Graphics_Control){
        .label = "Double Down",
        .info = "Allows doubling the wager after the initial two-card hand.",
        .labelWidth = 20,
        .type = GRAPHICS_CONTROL_BOOL_INPUT,
        .data.boolean = {&settings->allowDoubleDown},
    };
    controls[3] = (Graphics_Control){
        .label = "Double After Split",
        .info =
            "Allows doubling after splitting a pair; player-friendly rule.",
        .labelWidth = 20,
        .type = GRAPHICS_CONTROL_BOOL_INPUT,
        .data.boolean = {&settings->allowDoubleAfterSplit},
    };
    controls[4] = (Graphics_Control){
        .label = "Double Rule",
        .info = "Restricts which initial two-card totals may double.",
        .labelWidth = 20,
        .type = GRAPHICS_CONTROL_ENUM_INPUT,
        .data.enumeration =
            {
                (int *)&settings->doubleRule,
                doubleRuleChoices,
                3,
            },
    };
    controls[5] = (Graphics_Control){
        .label = "Split",
        .info = "Allows splitting pairs into separate hands.",
        .labelWidth = 20,
        .type = GRAPHICS_CONTROL_BOOL_INPUT,
        .data.boolean = {&settings->allowSplit},
    };
    controls[6] = (Graphics_Control){
        .label = "Resplit",
        .info = "Allows splitting a new pair again, up to four total hands.",
        .labelWidth = 20,
        .type = GRAPHICS_CONTROL_BOOL_INPUT,
        .data.boolean = {&settings->allowResplit},
    };
    controls[7] = (Graphics_Control){
        .label = "Hit Split Aces",
        .info = "Allows hitting hands created by splitting aces.",
        .labelWidth = 20,
        .type = GRAPHICS_CONTROL_BOOL_INPUT,
        .data.boolean = {&settings->allowHitSplitAces},
    };
    controls[8] = (Graphics_Control){
        .label = "Resplit Aces",
        .info =
            "Allows splitting aces again when a split ace draws another ace.",
        .labelWidth = 20,
        .type = GRAPHICS_CONTROL_BOOL_INPUT,
        .data.boolean = {&settings->allowResplitAces},
    };
    controls[9] = (Graphics_Control){
        .label = "Surrender",
        .info = "Allows forfeiting half the bet before playing a weak hand.",
        .labelWidth = 20,
        .type = GRAPHICS_CONTROL_BOOL_INPUT,
        .data.boolean = {&settings->allowSurrender},
    };
    controls[10] = (Graphics_Control){
        .label = "No Hole Card",
        .info = "Dealer receives no hidden second card until player hands "
                "finish.",
        .labelWidth = 20,
        .type = GRAPHICS_CONTROL_BOOL_INPUT,
        .data.boolean = {&settings->useNoHoleCardRule},
    };
    controls[11] = (Graphics_Control){
        .label = "Dealer Soft 17",
        .info = "Whether the dealer hits or stands on soft 17.",
        .labelWidth = 20,
        .type = GRAPHICS_CONTROL_ENUM_INPUT,
        .data.enumeration =
            {
                (int *)&settings->dealerSoft17Rule,
                dealerSoft17Choices,
                2,
            },
    };
    controls[12] = (Graphics_Control){
        .label = "Blackjack Pays",
        .info =
            "Payout for a natural blackjack; 3:2 is better for the player.",
        .labelWidth = 20,
        .type = GRAPHICS_CONTROL_ENUM_INPUT,
        .data.enumeration =
            {
                (int *)&settings->blackjackPayout,
                blackjackPayoutChoices,
                2,
            },
    };
    controls[13] = (Graphics_Control){
        .label = "Show True Count",
        .info = "Shows running count adjusted by decks remaining.",
        .labelWidth = 20,
        .type = GRAPHICS_CONTROL_BOOL_INPUT,
        .data.boolean = {&settings->showTrueCount},
    };
    controls[14] = (Graphics_Control){
        .label = "BACK",
        .info = "Return to the main menu.",
        .labelWidth = 0,
        .labelAlignment = GRAPHICS_ALIGN_CENTER,
        .type = GRAPHICS_CONTROL_BUTTON,
        .data.button = {backToMenu},
    };

    Graphics_arrangeControlsWithin(
        optionsBox,
        GRAPHICS_DIRECTION_COLUMN,
        0,
        controls,
        14
    );

    controls[14].tabIndex = 14;
    Graphics_positionControlWithin(
        &controls[14],
        backBox,
        GRAPHICS_ALIGN_STRETCH,
        GRAPHICS_ALIGN_STRETCH
    );

    const Graphics_ControlGroup group = {
        controls,
        15,
        &game->state.modeFocus[MODE_SETTINGS],
    };

    return group;
}
