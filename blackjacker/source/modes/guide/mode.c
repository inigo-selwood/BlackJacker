#include "mode.h"

#include <stdio.h>
#include <string.h>

#include "graphics/graphics.h"
#include "strategy/strategy.h"
#include "table/table.h"
#include "terminal/terminal.h"

enum {
    DEALER_COLUMN_COUNT = 10,
    HARD_ROW_COUNT = 17,
    SOFT_ROW_COUNT = 9,
    PAIR_ROW_COUNT = 10,
    GUIDE_CELL_WIDTH = 5,
    GUIDE_ROW_LABEL_WIDTH = 10,
    GUIDE_CONTROL_COUNT = 5,
    GUIDE_SECTION_CONTROL_COUNT = 3,
    GUIDE_NAV_CONTROL_COUNT = 2,
    GUIDE_BUTTON_WIDTH = 14,
    GUIDE_BUTTON_GAP = 1,
    GUIDE_KEY_WIDTH = 72,
    GUIDE_KEY_HEIGHT = 3,
};

typedef struct {
    const char *code;
    const char *label;
} GuideKeyItem;

typedef struct {
    GuideKeyItem items[5];
    int itemCount;
} GuideKeyRow;

static const char *dealerColumns[DEALER_COLUMN_COUNT] = {
    "2",
    "3",
    "4",
    "5",
    "6",
    "7",
    "8",
    "9",
    "10",
    "A",
};

static Table_Rank dealerRanks[DEALER_COLUMN_COUNT] = {
    TABLE_RANK_TWO,
    TABLE_RANK_THREE,
    TABLE_RANK_FOUR,
    TABLE_RANK_FIVE,
    TABLE_RANK_SIX,
    TABLE_RANK_SEVEN,
    TABLE_RANK_EIGHT,
    TABLE_RANK_NINE,
    TABLE_RANK_TEN,
    TABLE_RANK_ACE,
};

static const char *pairRows[PAIR_ROW_COUNT] = {
    "2,2",
    "3,3",
    "4,4",
    "5,5",
    "6,6",
    "7,7",
    "8,8",
    "9,9",
    "10,10",
    "A,A",
};

static int pairTotals[PAIR_ROW_COUNT] = {4, 6, 8, 10, 12, 14, 16, 18, 20, 22};

static const char *sectionTitle(Runtime_GuideSection section) {
    switch(section) {
    case GUIDE_SECTION_SOFT:
        return "SOFT TOTALS";
    case GUIDE_SECTION_PAIRS:
        return "PAIRS";
    case GUIDE_SECTION_HARD:
    default:
        return "HARD TOTALS";
    }
}

static void setHard(Runtime_Game *game) {
    game->state.guideSection = GUIDE_SECTION_HARD;
}

static void setSoft(Runtime_Game *game) {
    game->state.guideSection = GUIDE_SECTION_SOFT;
}

static void setPairs(Runtime_Game *game) {
    game->state.guideSection = GUIDE_SECTION_PAIRS;
}

static void backToTrain(Runtime_Game *game) {
    Runtime_queueState(&game->state, MODE_PLAY);
}

static void openSettings(Runtime_Game *game) {
    Runtime_queueState(&game->state, MODE_SETTINGS);
}

static void
arrangeSectionControls(Graphics_Control *controls, Graphics_Box box) {
    const int count = GUIDE_SECTION_CONTROL_COUNT;
    const int totalWidth =
        count * GUIDE_BUTTON_WIDTH + (count - 1) * GUIDE_BUTTON_GAP;
    const Graphics_Box row = Graphics_positionWithin(
        box,
        (Graphics_Size){totalWidth, 1},
        GRAPHICS_ALIGN_START,
        GRAPHICS_ALIGN_CENTER
    );

    Graphics_arrangeControlsWithin(
        row,
        GRAPHICS_DIRECTION_ROW,
        GUIDE_BUTTON_GAP,
        controls,
        count
    );
}

static void arrangeNavControls(Graphics_Control *controls, Graphics_Box box) {
    Graphics_positionControlWithin(
        &controls[0],
        box,
        GRAPHICS_ALIGN_START,
        GRAPHICS_ALIGN_STRETCH
    );
    Graphics_positionControlWithin(
        &controls[1],
        box,
        GRAPHICS_ALIGN_END,
        GRAPHICS_ALIGN_STRETCH
    );
}

static Graphics_ControlGroup guideControls(
    Runtime_Game *game,
    Graphics_Box sectionBox,
    Graphics_Box ruleBox
) {
    static Graphics_Control controls[GUIDE_CONTROL_COUNT];

    controls[0] = (Graphics_Control){
        .label = "HARD",
        .labelWidth = 0,
        .labelAlignment = GRAPHICS_ALIGN_START,
        .width = GUIDE_BUTTON_WIDTH,
        .shortcut = 'h',
        .type = GRAPHICS_CONTROL_BUTTON,
        .data.button = {setHard},
    };
    controls[1] = (Graphics_Control){
        .label = "SOFT",
        .labelWidth = 0,
        .labelAlignment = GRAPHICS_ALIGN_START,
        .width = GUIDE_BUTTON_WIDTH,
        .shortcut = 's',
        .type = GRAPHICS_CONTROL_BUTTON,
        .data.button = {setSoft},
    };
    controls[2] = (Graphics_Control){
        .label = "PAIRS",
        .labelWidth = 0,
        .labelAlignment = GRAPHICS_ALIGN_START,
        .width = GUIDE_BUTTON_WIDTH,
        .shortcut = 'p',
        .type = GRAPHICS_CONTROL_BUTTON,
        .data.button = {setPairs},
    };
    controls[3] = (Graphics_Control){
        .label = "SETTINGS",
        .labelWidth = 0,
        .labelAlignment = GRAPHICS_ALIGN_START,
        .width = GUIDE_BUTTON_WIDTH,
        .shortcut = 'o',
        .type = GRAPHICS_CONTROL_BUTTON,
        .data.button = {openSettings},
    };
    controls[4] = (Graphics_Control){
        .label = "BACK",
        .labelWidth = 0,
        .labelAlignment = GRAPHICS_ALIGN_START,
        .width = GUIDE_BUTTON_WIDTH,
        .shortcut = 'b',
        .type = GRAPHICS_CONTROL_BUTTON,
        .data.button = {backToTrain},
    };

    arrangeSectionControls(controls, sectionBox);
    arrangeNavControls(&controls[GUIDE_SECTION_CONTROL_COUNT], ruleBox);

    for(int index = 0; index < GUIDE_CONTROL_COUNT; index += 1) {
        controls[index].tabIndex = index;
    }

    const Graphics_ControlGroup group = {
        controls,
        GUIDE_CONTROL_COUNT,
        &game->state.modeFocus[MODE_GUIDE],
    };

    return group;
}

static void arrangeGuideSections(Graphics_Box screen, Graphics_Box *sections) {
    (void)screen;

    const Graphics_Box content = Graphics_minimumContentBox();
    const Graphics_Table maxTable = {
        .columns = dealerColumns,
        .columnCount = DEALER_COLUMN_COUNT,
        .rowCount = HARD_ROW_COUNT,
        .cellWidth = GUIDE_CELL_WIDTH,
        .rowLabelWidth = GUIDE_ROW_LABEL_WIDTH,
    };
    const Graphics_Size maxTableSize = Graphics_tableSize(maxTable);
    const Graphics_Box chartPanel = Graphics_positionWithin(
        content,
        (Graphics_Size){GUIDE_KEY_WIDTH, maxTableSize.height + 4},
        GRAPHICS_ALIGN_CENTER,
        GRAPHICS_ALIGN_CENTER
    );
    const Graphics_Box controlsPanel = Graphics_positionWithin(
        content,
        (Graphics_Size){content.width, 3},
        GRAPHICS_ALIGN_START,
        GRAPHICS_ALIGN_END
    );

    sections[0] = (Graphics_Box){0, 0, 0, 1};
    sections[1] = (Graphics_Box){0, 0, 0, GUIDE_KEY_HEIGHT};
    sections[2] = (Graphics_Box){0, 0, 0, maxTableSize.height};
    sections[3] = (Graphics_Box){0, 0, 0, 1};
    sections[4] = (Graphics_Box){0, 0, 0, 1};
    sections[5] = (Graphics_Box){0, 0, 0, 1};

    Graphics_arrangeWithin(
        chartPanel,
        GRAPHICS_DIRECTION_COLUMN,
        0,
        sections,
        3
    );
    Graphics_arrangeWithin(
        controlsPanel,
        GRAPHICS_DIRECTION_COLUMN,
        0,
        &sections[3],
        3
    );
}

static int guideRowCount(Runtime_GuideSection section) {
    switch(section) {
    case GUIDE_SECTION_SOFT:
        return SOFT_ROW_COUNT;
    case GUIDE_SECTION_PAIRS:
        return PAIR_ROW_COUNT;
    case GUIDE_SECTION_HARD:
    default:
        return HARD_ROW_COUNT;
    }
}

static Strategy_HandKind guideHandKind(Runtime_GuideSection section) {
    switch(section) {
    case GUIDE_SECTION_SOFT:
        return STRATEGY_HAND_KIND_SOFT;
    case GUIDE_SECTION_PAIRS:
        return STRATEGY_HAND_KIND_PAIR;
    case GUIDE_SECTION_HARD:
    default:
        return STRATEGY_HAND_KIND_HARD;
    }
}

static int playerTotalForRow(Runtime_GuideSection section, int row) {
    switch(section) {
    case GUIDE_SECTION_SOFT:
        return row + 13;
    case GUIDE_SECTION_PAIRS:
        return pairTotals[row];
    case GUIDE_SECTION_HARD:
    default:
        return row + 5;
    }
}

static const char *rowLabelFor(
    Runtime_GuideSection section,
    int row,
    char *label,
    int labelSize
) {
    if(section == GUIDE_SECTION_PAIRS) {
        return pairRows[row];
    }

    if(section == GUIDE_SECTION_SOFT) {
        snprintf(label, (size_t)labelSize, "A,%d", row + 2);
        return label;
    }

    snprintf(label, (size_t)labelSize, "%d", row + 5);
    return label;
}

static void writeRowRangeLabel(
    Runtime_GuideSection section,
    int firstRow,
    int lastRow,
    char *label,
    int labelSize
) {
    char firstLabel[GUIDE_ROW_LABEL_WIDTH + 1];
    char lastLabel[GUIDE_ROW_LABEL_WIDTH + 1];
    const int lowerRow = firstRow < lastRow ? firstRow : lastRow;
    const int upperRow = firstRow > lastRow ? firstRow : lastRow;
    const char *lowerLabel =
        rowLabelFor(section, lowerRow, firstLabel, (int)sizeof(firstLabel));
    const char *upperLabel =
        rowLabelFor(section, upperRow, lastLabel, (int)sizeof(lastLabel));

    if(lowerRow == upperRow || strcmp(lowerLabel, upperLabel) == 0) {
        snprintf(label, (size_t)labelSize, "%s", lowerLabel);
        return;
    }

    snprintf(label, (size_t)labelSize, "%s-%s", lowerLabel, upperLabel);
}

static bool guideRowsMatch(
    const char **cells,
    const int *cellAttrs,
    int firstRow,
    int secondRow
) {
    for(int column = 0; column < DEALER_COLUMN_COUNT; column += 1) {
        const int firstIndex = firstRow * DEALER_COLUMN_COUNT + column;
        const int secondIndex = secondRow * DEALER_COLUMN_COUNT + column;

        if(strcmp(cells[firstIndex], cells[secondIndex]) != 0
            || cellAttrs[firstIndex] != cellAttrs[secondIndex]) {
            return false;
        }
    }

    return true;
}

static void copyGuideRow(
    const char **sourceCells,
    const int *sourceAttrs,
    int sourceRow,
    const char **cells,
    int *cellAttrs,
    int targetRow
) {
    for(int column = 0; column < DEALER_COLUMN_COUNT; column += 1) {
        const int sourceIndex = sourceRow * DEALER_COLUMN_COUNT + column;
        const int targetIndex = targetRow * DEALER_COLUMN_COUNT + column;

        cells[targetIndex] = sourceCells[sourceIndex];
        cellAttrs[targetIndex] = sourceAttrs[sourceIndex];
    }
}

static int buildGuideTable(
    Runtime_GuideSection section,
    const char **rows,
    const char **cells,
    int *cellAttrs
) {
    static char rowLabels[HARD_ROW_COUNT][GUIDE_ROW_LABEL_WIDTH + 1];
    static const char *sourceCells[HARD_ROW_COUNT * DEALER_COLUMN_COUNT];
    static int sourceAttrs[HARD_ROW_COUNT * DEALER_COLUMN_COUNT];
    static int sourceRows[HARD_ROW_COUNT];
    const int rowCount = guideRowCount(section);
    const Strategy_HandKind handKind = guideHandKind(section);
    int outputRow = 0;

    for(int displayRow = 0; displayRow < rowCount; displayRow += 1) {
        const int sourceRow = rowCount - displayRow - 1;
        sourceRows[displayRow] = sourceRow;

        for(int column = 0; column < DEALER_COLUMN_COUNT; column += 1) {
            const char *code;
            const int total = playerTotalForRow(section, sourceRow);

            const int cellIndex = displayRow * DEALER_COLUMN_COUNT + column;

            if(Strategy_bestActionCode(
                   handKind,
                   total,
                   dealerRanks[column],
                   &code
               )) {
                sourceCells[cellIndex] = code;
                sourceAttrs[cellIndex] =
                    (int)Terminal_actionAttrs(sourceCells[cellIndex]);
            } else {
                sourceCells[cellIndex] = "";
                sourceAttrs[cellIndex] = 0;
            }
        }
    }

    for(int groupStart = 0; groupStart < rowCount;) {
        int groupEnd = groupStart;

        while(groupEnd + 1 < rowCount
            && guideRowsMatch(
                sourceCells,
                sourceAttrs,
                groupStart,
                groupEnd + 1
            )) {
            groupEnd += 1;
        }

        writeRowRangeLabel(
            section,
            sourceRows[groupStart],
            sourceRows[groupEnd],
            rowLabels[outputRow],
            (int)sizeof(rowLabels[outputRow])
        );
        rows[outputRow] = rowLabels[outputRow];
        copyGuideRow(
            sourceCells,
            sourceAttrs,
            groupStart,
            cells,
            cellAttrs,
            outputRow
        );

        outputRow += 1;
        groupStart = groupEnd + 1;
    }

    return outputRow;
}

static void drawGuideKey(Graphics_Box box) {
    static const GuideKeyRow rows[] = {
        {{{"H", "Hit"},
             {"S", "Stand"},
             {"D", "Double"},
             {"P", "Split"},
             {"R", "Surrender"}},
            5},
    };
    const int rowCount = (int)(sizeof(rows) / sizeof(rows[0]));
    const int itemX[] = {
        box.x + (box.width - 68) / 2,
        box.x + (box.width - 68) / 2 + 12,
        box.x + (box.width - 68) / 2 + 26,
        box.x + (box.width - 68) / 2 + 41,
        box.x + (box.width - 68) / 2 + 54,
    };

    for(int row = 0; row < rowCount; row += 1) {
        const int y = box.y + row + 1;

        for(int item = 0; item < rows[row].itemCount; item += 1) {
            int x = itemX[item];
            const GuideKeyItem key = rows[row].items[item];
            const attr_t attrs = Terminal_actionAttrs(key.code);

            attron(attrs);
            mvaddstr(y, x, " ");
            x += 1;
            mvaddstr(y, x, key.code);
            x += (int)strlen(key.code);
            mvaddstr(y, x, " ");
            attroff(attrs);
            x += 2;

            mvaddstr(y, x, key.label);
        }
    }
}

static void drawGuide(Runtime_Game *game) {
    static const char *rows[HARD_ROW_COUNT];
    static const char *cells[HARD_ROW_COUNT * DEALER_COLUMN_COUNT];
    static int cellAttrs[HARD_ROW_COUNT * DEALER_COLUMN_COUNT];
    const Runtime_GuideSection section = game->state.guideSection;
    const Graphics_Box screen = Graphics_screenBox();
    Graphics_Box sections[6];
    Graphics_Label title = Graphics_label(
        sectionTitle(section),
        GRAPHICS_ALIGN_CENTER,
        true,
        false
    );
    Graphics_ControlGroup controls;

    arrangeGuideSections(screen, sections);
    controls = guideControls(game, sections[3], sections[5]);

    Graphics_positionLabelWithin(
        &title,
        sections[0],
        GRAPHICS_ALIGN_STRETCH,
        GRAPHICS_ALIGN_STRETCH
    );
    Graphics_drawLabel(title);

    if(!Strategy_loadDatabase()) {
        Graphics_Label error = Graphics_label(
            "Strategy database not found.",
            GRAPHICS_ALIGN_CENTER,
            false,
            false
        );
        Graphics_positionLabelWithin(
            &error,
            sections[2],
            GRAPHICS_ALIGN_STRETCH,
            GRAPHICS_ALIGN_CENTER
        );
        Graphics_drawLabel(error);
        Graphics_drawControls(controls);
        return;
    }

    const int rowCount = buildGuideTable(section, rows, cells, cellAttrs);
    const Graphics_Table table = {
        .columns = dealerColumns,
        .columnCount = DEALER_COLUMN_COUNT,
        .rows = rows,
        .rowCount = rowCount,
        .cells = cells,
        .cellAttrs = cellAttrs,
        .cellWidth = GUIDE_CELL_WIDTH,
        .rowLabelWidth = GUIDE_ROW_LABEL_WIDTH,
    };
    const Graphics_Size tableSize = Graphics_tableSize(table);
    Graphics_Table positionedTable = table;

    drawGuideKey(sections[1]);
    Graphics_drawHorizontalDivider(sections[4]);
    positionedTable.box = Graphics_positionWithin(
        sections[2],
        tableSize,
        GRAPHICS_ALIGN_CENTER,
        GRAPHICS_ALIGN_START
    );
    Graphics_drawTable(positionedTable);
    Graphics_drawControls(controls);
}

void guideCallback(Runtime_Game *game, Runtime_ModeEvent event) {
    const Graphics_Box screen = Graphics_screenBox();
    Graphics_Box sections[6];

    arrangeGuideSections(screen, sections);
    Graphics_ControlGroup controls =
        guideControls(game, sections[3], sections[5]);

    if(event.type == MODE_EVENT_INPUT) {
        Graphics_handleControlInput(&controls, event.input, game);
        return;
    }

    drawGuide(game);
}
