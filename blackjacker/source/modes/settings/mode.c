#include "mode.h"

#include "graphics/graphics.h"

typedef struct {
    Graphics_Box title;
    Graphics_Box help;
    Graphics_Box divider;
    Graphics_Box options;
    Graphics_Box back;
} SettingsLayout;

static void backToMenu(Runtime_Game *game) {
    (void)Runtime_updateSettings(game, game->playSettings);
    Runtime_queueState(&game->state, MODE_MENU);
}

static SettingsLayout settingsLayout(void) {
    const Graphics_Box screen = Graphics_screenBox();
    const Graphics_Box panel = Graphics_positionWithin(
        screen,
        (Graphics_Size){44, 22},
        GRAPHICS_ALIGN_CENTER,
        GRAPHICS_ALIGN_CENTER
    );
    Graphics_Box sections[] = {
        {0, 0, 0, 1},
        {0, 0, 0, 1},
        {0, 0, 0, 3},
        {0, 0, 0, 1},
        {0, 0, 0, 14},
        {0, 0, 0, 1},
        {0, 0, 0, 1},
    };

    Graphics_arrangeWithin(panel, GRAPHICS_DIRECTION_COLUMN, 0, sections, 7);

    const SettingsLayout settings = {
        .title = sections[0],
        .help = sections[2],
        .divider = sections[3],
        .options = sections[4],
        .back = sections[6],
    };

    return settings;
}

static Graphics_ControlGroup settingsControls(
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

static void drawSettings(Runtime_Game *game) {
    const SettingsLayout layout = settingsLayout();
    Graphics_ControlGroup controls =
        settingsControls(game, layout.options, layout.back);
    Graphics_Control *focused = Graphics_focusedControl(&controls);
    Graphics_Label title =
        Graphics_label("PLAY SETTINGS", GRAPHICS_ALIGN_CENTER, true, false);
    Graphics_Label help = Graphics_label(
        focused && focused->info ? focused->info : "",
        GRAPHICS_ALIGN_START,
        false,
        false
    );

    Graphics_positionLabelWithin(
        &title,
        layout.title,
        GRAPHICS_ALIGN_STRETCH,
        GRAPHICS_ALIGN_STRETCH
    );
    Graphics_positionLabelWithin(
        &help,
        layout.help,
        GRAPHICS_ALIGN_STRETCH,
        GRAPHICS_ALIGN_STRETCH
    );

    Graphics_drawLabel(title);
    Graphics_drawWrappedLabel(help);
    Graphics_drawHorizontalDivider(layout.divider);
    Graphics_drawControls(controls);
}

void settingsCallback(Runtime_Game *game, Runtime_ModeEvent event) {
    const SettingsLayout layout = settingsLayout();
    Graphics_ControlGroup controls =
        settingsControls(game, layout.options, layout.back);

    if(event.type == MODE_EVENT_INPUT) {
        Graphics_handleControlInput(&controls, event.input, game);
        return;
    }

    drawSettings(game);
}
